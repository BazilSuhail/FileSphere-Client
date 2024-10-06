# Client Program in C

This folder contains a C program source file named `client.c`. This code implements a client-side application that allows users to manage files uploaded to a server and monitor the storage space used. It also includes a basic authentication mechanism.

## Files

- **`client.c`**: The source code for the client-side application.
- **`Makefile`**: Used to build, clean, and run the project.

## How to Build and Execute the Code

### Build and Execution Commands

The following commands are used to compile, clean, and run the executable of this C program:

- **`make`**: Compiles and builds the program, generating the `client.exe` executable.
- **`make clean`**: Removes the existing executable and any intermediate compiled files.
- **`make run`**: Compiles the program (if necessary) and runs the executable. If the source files are unchanged and the executable exists, it directly runs the program.
- **`make clean run`**: Cleans any previous builds, compiles the program, and then runs the executable.

### Tools Required

- **`gcc`**: A popular C compiler, part of the GNU Compiler Collection.
- **`make`**: A build tool for automating the process of compiling and managing dependencies.
- **`libc6-dev`**: Development libraries and headers for the GNU C Library, essential for compiling C programs.

### Setup Instructions

Before building the program, ensure that the necessary build tools (compiler, make, libc6-dev) are installed on your system. On a Debian-based system (e.g., Ubuntu), you can install them using the following commands:

```bash
sudo apt-get update
sudo apt-get install build-essential
```

Once the tools are installed, navigate to the directory containing the `Makefile` and run the desired command (e.g., `make` or `make run`).

---
