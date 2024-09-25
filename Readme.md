This folder has a C program source code named `client.c`, This code is for a client implementation in C for a client to look after his files uploaded to server and check for the storage he has used ,covering basic implementation of authentication in C. For this C code use make file to build it and then the C program will be compiled into `main.exe`. 

## Files Made
- `server.c`: This is the source file containing the C source code of actual server.
- `Makefile`: This file is used for building, cleaning and running the project.

## How to build and Execute the code

### About Execution and build
Commands which are used to run, build and clean the `.exe` of this C program are following:

`make` Execute this command in the terminal to compile/build the program.
`make clean`  Use this command to remove the existing executable.
`make run`  This command compiles and immediately runs the program.
`make clean run` This sequence will first remove the old executable, then compile, and finally run the program.

### Tools Required
- `gcc`: A widely used C compiler, part of the GNU Compiler Collection.
- `make`: A build tool used to manage and automate the process of compiling code.

### Setup Instructions
Before building the program, ensure that gcc and make are installed on your system. On a Debian-based system (e.g., Ubuntu), you can install them using the following commands:

```sh
sudo apt-get update
sudo apt-get install gcc make
```
