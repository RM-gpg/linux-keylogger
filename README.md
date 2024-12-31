# linux-keylogger
A keylogger written in C. It is designed to act as a replacement for the user’s shell (/bin/bash), where it processes input from the user (stdin), logs it, and sends it to a bash child process running in a pseudo terminal. The output from the bash process is piped into stdout.
