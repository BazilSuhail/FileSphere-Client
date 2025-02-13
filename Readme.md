# Client Program in C
A client implementation with for a server handling Queue-Based Reader-Writer Locking mechanism I created as a semester project in my 5th semster under supervision of [Dr. Khawaja Umar Suleman](https://www.linkedin.com/in/umar-suleman/) along with my fellows [Muhammad Rehman](https://github.com/MuhammdRehman) and [Abdullah Masood](https://github.com/Abdullah-Masood-05). 

[![Open Source Love svg1](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)](#)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat&label=Contributions&colorA=red&colorB=black	)](#)



### Project Description
This project has a C program source code for a client implementation in C such that a client looks after his files uploaded to server and check for the storage he has used ,covering basic implementation of authentication in C. The user can create his account using name and password in the server. After that the user can type in various commands and perform Upload, Download And View queries after logging in to his account.

- For this C code use make file to build it and then the C program will be compiled into main.exe.

### 🤖 Tech Stack 
<a href="#"> 
<img alt="C Language" src="https://img.shields.io/badge/C%20Language-%2300599C.svg?&style=for-the-badge&logo=C&logoColor=white"/>
<img alt="Server" src="https://img.shields.io/badge/Server-%23FF6F00.svg?&style=for-the-badge&logo=Server&logoColor=white"/>
<img alt="Reader Writer Problem" src="https://img.shields.io/badge/Reader%20Writer%20Problem-%236C63FF.svg?&style=for-the-badge&logo=ReadMe&logoColor=white"/>
<img alt="Mutex" src="https://img.shields.io/badge/Mutex-%23FF4081.svg?&style=for-the-badge&logo=Lock&logoColor=white"/>
<img alt="Semaphores" src="https://img.shields.io/badge/Semaphores-%23009688.svg?&style=for-the-badge&logo=Traffic%20Light&logoColor=white"/>

 </a>

---
- Find the Server Repository of this Project Here [FilesSphere-Server](https://github.com/BazilSuhail/FileSphere-Server). 
---

### Files and Folder Structure

- **`client.c`**: The source code for the client-side application.
- **`Makefile`**: Used to build, clean, and run the project.
- **`helper_utilities.c`**: Contains Queries funciton's utility modules.
- **`helper_queries.c`**: Contains Queries code/ funcitons the user executes.
- **`helper.h`**: Initialization of globally accessible functions.
- **`Makefile`**: Used to build, clean, and run the project.

---



## Features

#### 1. Server Setup and Connection Management
- **Socket Initialization**: Creates a TCP server that listens on a specified port (`PORT`).
- **Client Connections**: Handles up to a maximum number of clients (`MAX_CONNECTIONS`) concurrently.
- **Threaded Client Handling**: Spawns a new thread for each client connection to handle client-specific tasks independently.

#### 2. User Management
- **User Information Storage**: Stores and manages user-specific data (e.g., username, read/write counts, and request queues).
- **User Initialization**: Creates a new `UserInfo` entry for a client if the user is not already connected.
- **Global User List**: Maintains a list of active users, limited by `MAX_CONNECTIONS`.

#### 3. Queue-Based Reader-Writer Synchronization
- **Request Queue**: Manages a queue of requests (`READ` or `WRITE`) for each user.
- **Read Operations**: Allows multiple clients to read simultaneously, provided no write is in progress.
- **Write Operations**: Allows only one client to write at a time, blocking reads and other writes until the write completes.
- **Queue Processing**: Processes requests in a first-come, first-served order for fairness.

#### 4. Client Task Management
- **Task Options**: Supports a range of operations based on client requests:
  - **Upload/Delete/Update** (Write operations): Requires exclusive access to user data.
  - **Download/View** (Read operations): Allows shared access unless a write is ongoing.
- **Read-Write Execution**: Manages read and write operations with functions `startRead`, `finishRead`, `startWrite`, and `finishWrite` to ensure safe concurrent access.

#### 5. Authentication/Registration
- **Registration Logic**: Registration of the user using his name and password(e.g., storing of credentials like name and passwords).
- **Authentication Logic**: Authentication of user using his name and password with which he registered his account(e.g., name and password checks).
- **Username Retrieval**: Retrieves the username for each client upon connection.

#### 6. Synchronization and Thread Safety
- **Mutex Locks and Condition Variables**: Uses mutexes and condition variables to ensure safe access to shared data.
- **Global Connection Control**: Limits the maximum number of client connections using a global mutex and condition variable.

#### 7. Functionalites Offered to User
- **Upload**: Upload files to the server. Files are saved in the User's directory on the server, the server alos asks user to replace if the file exists otherwise will create a copy automatically.
- **Download** Download files from the server. The requested file is sent from the server to the client.
- **View** View file details, such as file name, size, and creation date, if available.
- **Delete** Delete files from the server. The specified file is removed from the server’s storage.
- **Update** Update or replace existing files on the server with a new version.

---

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
