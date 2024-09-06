#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_SIZE 1024
#define STORAGE_LIMIT 10240 

void uploadFile(int clientSocket, const char *filePath)
{
    int fileDescriptor = open(filePath, O_RDONLY);
    if (fileDescriptor < 0)
    {
        perror("Error opening file");
        return;
    }

    char command[256];
    snprintf(command, sizeof(command), "$UPLOAD$%s$", filePath);
    send(clientSocket, command, strlen(command), 0);

    char response[256];
    recv(clientSocket, response, sizeof(response) - 1, 0);
    response[sizeof(response) - 1] = '\0';
    if (strcmp(response, "$SUCCESS$") == 0)
    {
        printf("Server response: %s\n", response);
        close(fileDescriptor);
        return;
    }

    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
    {
        send(clientSocket, buffer, bytesRead, 0);
    }

    close(fileDescriptor);
    printf("File data sent successfully.\n");

    recv(clientSocket, response, sizeof(response) - 1, 0);
    response[sizeof(response) - 1] = '\0';
    printf("Server response: %s\n", response);
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

    char filePath[256];
    printf("Enter file path to upload: ");
    scanf("%s", filePath);

    uploadFile(clientSocket, filePath);

    close(clientSocket);
    return 0;
}