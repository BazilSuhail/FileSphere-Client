#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#define MAX_SIZE 1024
void downloadFile(int clientSocket, const char *fileName)
{
    // Send the file name to the server
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        perror("Error sending file name to server");
        return;
    }

    // Receive acknowledgment from the server
    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        perror("Error receiving acknowledgment from server");
        return;
    }
    response[receivedBytes] = '\0';

    if (strcmp(response, "No file found") == 0)
    {
        printf("Server response: No file found\n");
        return;
    }

    if (strcmp(response, "$READY$") != 0)
    {
        printf("Server is not ready to send the file.\n");
        return;
    }

    // Create the file to save the received data
    int fileDescriptor = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor < 0)
    {
        perror("Error creating file");
        return;
    }

    // Receive file data and write to the file
    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        if(strncmp(buffer,"END",3)==0) break;
        write(fileDescriptor, buffer, bytesRead);
    }

    if (bytesRead < 0)
    {
        perror("Error receiving file data");
    }
    else
    {
        printf("File downloaded and saved successfully.\n");
    }

    close(fileDescriptor);
}


void uploadFile(int clientSocket, const char *filePath)
{
    // Open the file
    int fileDescriptor = open(filePath, O_RDONLY);
    if (fileDescriptor < 0)
    {
        perror("Error opening file");
        return;
    }

    // Extract file name from file path
    const char *fileName = strrchr(filePath, '/');
    fileName = fileName ? fileName + 1 : filePath;

    // Send the file name to the server
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        perror("Error sending file name to server");
        close(fileDescriptor);
        return;
    }

    // Receive acknowledgment from the server
    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        perror("Error receiving acknowledgment from server");
        close(fileDescriptor);
        return;
    }
    response[receivedBytes] = '\0';

    if (strcmp(response, "$READY$") != 0)
    {
        printf("Server is not ready to receive the file.\n");
        close(fileDescriptor);
        return;
    }

    // Send the file data
    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
    {
        send(clientSocket, buffer, bytesRead, 0);
    }
    if((send(clientSocket,"END",3,0))<0)printf("ERROR in Sending END\n");
    recv(clientSocket,"buffer",sizeof(buffer),0);
    printf("File uploaded successfully.\n");
    close(fileDescriptor);
}

void viewFiles(int clientSocket)
{
    // Request the file list from the server
    const char *operation = "3";
    ssize_t sentBytes = send(clientSocket, operation, strlen(operation), 0);
    if (sentBytes < 0)
    {
        perror("Error sending operation type to server");
        return;
    }

    // Receive and display file list from the server
    char response[MAX_SIZE];
    ssize_t receivedBytes;
    while ((receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0)) > 0)
    {
        response[receivedBytes] = '\0';
        if (strcmp(response, "$END$") == 0)
        {
            break;
        }
        printf("%s", response);
    }
}

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Failed to connect to the server");
        close(clientSocket);
        return 1;
    }

    char command[512];

    while(1)
    {
        printf("-> ");
        scanf("%s", command);

        if (strncmp(command, "$UPLOAD$", 8) == 0)
        {
            // Extract the file path from the command
            const char *filePath = command + 8;
            send(clientSocket, "2", 1, 0);
            uploadFile(clientSocket, filePath);
        }
        else if (strncmp(command, "$DOWNLOAD$", 10) == 0)
        {
            // Extract the file name from the command
            const char *fileName = command + 10;
            send(clientSocket, "1", 1, 0);
            downloadFile(clientSocket, fileName);
        }
        else if (strncmp(command, "$VIEW$", 10) == 0)
        {
            send(clientSocket, "3", 1, 0);
            viewFiles(clientSocket);
        }
        else if (strcmp(command, "exit") == 0)
        {
            send(clientSocket,"5",1,0);
            printf("Exiting program.\n");
            break;
        }
        else
        {
            printf("Invalid command.\n");
        }

    }

    close(clientSocket);
    return 0;
}

/*
int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("Failed to create socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server address
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Connection failed");
        close(clientSocket);
        return 1;
    }

    printf("Connected to server.\n");

    int choice;
    char fileName[256];

    while (1)
    {
        printf("Select operation:\n1. Download file\n2. Upload file\n3. View files\n4. Exit\n");
        scanf("%d", &choice);
        getchar(); // Consume newline character

        switch (choice)
        {
            case 1:
                printf("Enter file name to download: ");
                fgets(fileName, sizeof(fileName), stdin);
                fileName[strcspn(fileName, "\n")] = '\0'; // Remove newline
                downloadFile(clientSocket, fileName);
                break;
            case 2:
                printf("Enter file path to upload: ");
                fgets(fileName, sizeof(fileName), stdin);
                fileName[strcspn(fileName, "\n")] = '\0'; // Remove newline
                uploadFile(clientSocket, fileName);
                break;
            case 3:
                viewFiles(clientSocket);
                break;
            case 4:
                close(clientSocket);
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}
*/













// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <fcntl.h>

// #define MAX_SIZE 1024
// #define STORAGE_LIMIT 10240 

// void uploadFile(int clientSocket, const char *filePath)
// {
//     int fileDescriptor = open(filePath, O_RDONLY);
//     if (fileDescriptor < 0)
//     {
//         perror("Error opening file");
//         return;
//     }

//     char command[256];
//     snprintf(command, sizeof(command), "$UPLOAD$%s$", filePath);
//     send(clientSocket, command, strlen(command), 0);
//     char response[256];
//     recv(clientSocket, response, sizeof(response) - 1, 0);
//     response[sizeof(response) - 1] = '\0';
//     if (strcmp(response, "$SUCCESS$") == 0)
//     {
//         printf("Server response: %s\n", response);
//         close(fileDescriptor);
//         return;
//     }
//     char buffer[MAX_SIZE];
//     ssize_t bytesRead;
//     while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
//     {
//         send(clientSocket, buffer, bytesRead, 0);
//     }
//     if(send(clientSocket,"END",3,0)<0)
//     {
//         printf("Error in Sending Data!!!!\n");
//     }
//     close(fileDescriptor);
//     printf("File data sent successfully.\n");

//     recv(clientSocket, response, sizeof(response) - 1, 0);
//     response[sizeof(response) - 1] = '\0';
//     printf("Server response: %s\n", response);
// }
// void DownloadFile(int clientSocket,const char* filename)
// {
//     char buffer[MAX_SIZE];
//     char command[256];
//     snprintf(command, sizeof(command), "$DOWNLOAD$%s$", filename);

//     if(send(clientSocket, command, sizeof(command) - 1, 0)<0){printf("Error Sending FileName!!!!\n");}
//     int val= recv(clientSocket,buffer,MAX_SIZE,0);

//     if(val<0){printf("Error in Receiving Data after Name sending!!!\n");}
//     if(!strncmp(buffer,"$READY$",6)){printf("SErver is ready to send data!!!\n");}
//     else{printf("Server is unable to find file\nPlease check your file name");return;}
//     int fileDescriptor = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
//     if (fileDescriptor < 0)
//     {
//         perror("Error creating file");
//         close(clientSocket);
//         return;
//     }

//     ssize_t bytesRead;
//     while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
//     {
//         if(strncmp(buffer,"END",3))
//             write(fileDescriptor, buffer, bytesRead);
//         else break;
//     }

//     if (bytesRead < 0)
//     {
//         perror("Error receiving file data");
//     }
   
//     send(clientSocket, "$SUCCESS$", 9, 0);
//     printf("Succesfully Received and saved: %s",filename);
//     close(fileDescriptor);
    
// }
// int main()
// {
//     int clientSocket;
//     struct sockaddr_in serverAddr;

//     clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//     if (clientSocket < 0)
//     {
//         perror("Failed to create socket");
//         return 1;
//     }

//     serverAddr.sin_family = AF_INET;
//     inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));
//     serverAddr.sin_port = htons(12345);

//     if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
//     {
//         perror("Failed to connect to the server");
//         close(clientSocket);
//         return 1;
//     }
//     int n;
//     printf("1.Upload\n2.Download : ");
//     scanf("%d",&n);
//     int val = send(clientSocket,(char *)&n,1,0);
//     if(val<0)
//     {
//         printf("There is an error in this please try again");
//         return 1;
//     }
//     if(n==1){

//         char filePath[256];
//         printf("Enter file path to upload: ");
//         scanf("%s", filePath);
//         uploadFile(clientSocket, filePath);
//     }
//     else{
//         char filename[30];
//         printf("Enter file path to Download: ");
//         scanf("%s", filename);
//         DownloadFile(clientSocket,filename);
//     }
//     close(clientSocket);
//     return 0;
// }
