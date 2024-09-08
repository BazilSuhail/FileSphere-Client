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
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        printf("Error sending file name to server");
        return;
    }

    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        printf("Error receiving acknowledgment from server");
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

    int fileDescriptor = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor < 0)
    {
        printf("Error creating file");
        return;
    }

    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (write(fileDescriptor, buffer, bytesRead) != bytesRead)
        {
            printf("Error writing to file");
            close(fileDescriptor);
            return;
        }
    }

    if (bytesRead < 0)
    {
        printf("Error receiving file data");
    }
    else
    {
        printf("File downloaded and saved successfully.\n");
    }

    close(fileDescriptor);
}

void uploadFile(int clientSocket, const char *filePath)
{ 
    int fileDescriptor = open(filePath, O_RDONLY);
    if (fileDescriptor < 0)
    {
        printf("Error opening file");
        return;
    }
 
    const char *fileName = strrchr(filePath, '/');
    fileName = fileName ? fileName + 1 : filePath;
 
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        printf("Error sending file name to server");
        close(fileDescriptor);
        return;
    }
 
    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        printf("Error receiving acknowledgment from server");
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
 
    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
    {
        ssize_t bytesSent = send(clientSocket, buffer, bytesRead, 0);
        if (bytesSent < 0)
        {
            printf("Error sending file data");
            close(fileDescriptor);
            return;
        }
    }
 
    send(clientSocket, "$DONE$", 6, 0);
 
     receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        printf("Error receiving final acknowledgment from server");
    }
    else
    {
        response[receivedBytes] = '\0';
        if (strcmp(response, "$SUCCESS$") == 0)
        {
            printf("File uploaded and saved successfully on the server.\n");
        }
        else
        {
            printf("File upload failed on the server.\n");
        }
    }
     close(fileDescriptor);
}

void viewFiles(int clientSocket)
{ 
    const char *operation = "3";
    ssize_t sentBytes = send(clientSocket, operation, strlen(operation), 0);
    if (sentBytes < 0)
    {
        printf("Error sending operation type to server");
        return;
    }
 
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
        printf("Failed to create socket");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(serverAddr.sin_addr));
    serverAddr.sin_port = htons(12345);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Failed to connect to the server");
        close(clientSocket);
        return 1;
    }

    char command[512];

    printf("-> ");
    scanf("%s", command);

    if (strncmp(command, "$UPLOAD$", 8) == 0)
    { 
        const char *filePath = command + 8;
        send(clientSocket, "2", 1, 0);
        uploadFile(clientSocket, filePath);
    }
    else if (strncmp(command, "$DOWNLOAD$", 10) == 0)
    { 
        const char *fileName = command + 10;
        send(clientSocket, "1", 1, 0);
        downloadFile(clientSocket, fileName);
    }
    else if (strncmp(command, "$VIEW$", 10) == 0)
    {
        send(clientSocket, "3", 1, 0);
        viewFiles(clientSocket);
    }
    else
    {
        printf("Invalid command.\n");
    }

    close(clientSocket);
    return 0;
}