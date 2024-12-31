#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/wait.h>
#include <pty.h>
#include <sys/select.h>

#define BUFSIZE 1024
#define LOG_FILE "/var/out/log.txt"

pid_t pid; //child process PID
int pgid; //process group ID of foreground process on terminal

//signal handler to forward signals to bash process group
void handler(int signal) {
    if (pid > 0) {
        kill(-pgid,signal);
    }
}

int main() {
    int master_fd; //master file descriptor of PTY
    char buffer[BUFSIZE]; //buffer to store input/output
    struct termios orig_termios, raw_termios; //termios structures to store terminal parameters

    signal(SIGINT, handler); //specifies handler for SIGINT

    //open log file
    FILE *log_file = fopen(LOG_FILE,"a");

    //get current terminal attributes
    tcgetattr(STDIN_FILENO, &orig_termios);

    //disable echoing in terminal
    raw_termios = orig_termios;
    raw_termios.c_lflag &= ~(ECHO | ICANON); //disable echo, canonical mode
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios);

    //create new PTY for child process (bash)
    pid = forkpty(&master_fd, NULL, NULL, NULL);

    if (pid == 0) { //child
        //run interactive login bash shell
        execlp("bash", "bash", "-l", NULL);
    }

    else { //parent
        while (1) {
            pgid = tcgetpgrp(master_fd);

            //select() used to wait for input from stdin or output from the PTY
            fd_set fds;
            FD_ZERO(&fds); //clears the fd_set
            FD_SET(STDIN_FILENO, &fds); //wait on stdin for user input
            FD_SET(master_fd, &fds); //wait on PTY master for bash output

            int max_fd;
            if (STDIN_FILENO > master_fd) {
                max_fd = STDIN_FILENO;
            }
            else {
                max_fd = master_fd;
            }
            max_fd++; //select() function tests file descriptors in the range of 0 to nfds-1, so we increment this

            int ready = select(max_fd, &fds, NULL, NULL, NULL);

            if (ready == -1) {
                //printf("blah\n");
                continue;
            }

            //check if bash process has exited
            pid_t result = waitpid(pid, NULL, WNOHANG);
            if (result == pid) {
                //break out of loop
                break;
            }

            //check for input data to read from stdin (user input)
            if (FD_ISSET(STDIN_FILENO, &fds)) {
                ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
                if (n > 0) {
                    //log user input
                    fwrite(buffer, 1, n, log_file);
                    fflush(log_file);  //ensure log is written immediately

                    //write user input to PTY master (bash stdin)
                    write(master_fd, buffer, n);
                }
            }

            //check for output data from PTY master (bash output)
            if (FD_ISSET(master_fd, &fds)) {
                ssize_t n = read(master_fd, buffer, sizeof(buffer) - 1);
                if (n > 0) {
                    buffer[n] = '\0'; //null-terminate buffer for printing
                    //write bash output to stdout
                    printf("%s", buffer);
                    fflush(stdout);
                }
            }
        }
    }

    //restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

    //close log file
    fclose(log_file);

    return 0;
}
