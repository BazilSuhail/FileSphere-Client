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
    
    int val = send(clientSocket,"1",1,0);
    if(val<0)
    {
        printf("There is an error in this please try again");
        return;
    }
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
void DownloadFile(clientSocket,filename)
{
    int val = send(clientSocket,"2",1,0);
    if(val<0)
    {
        printf("There is an error in Download please try again");
        return;
    }
    char buffer[MAX_SIZE];
    char command[256];
    int recvSize = recv(clientSocket, command, sizeof(command) - 1, 0);
    if (recvSize <= 0)
    {
        close(clientSocket);
        return;
    }
    command[recvSize] = '\0';

    if (strncmp(command, "$UPLOAD$", 8) != 0)
    {
        printf("Invalid command.\n");
        close(clientSocket);
        return;
    }

    char filePath[256];
    strcpy(filePath, "test.txt");

    int availableSpace = checkStorageSpace();
    if (availableSpace <= 0)
    {
        send(clientSocket, "$FAILURE$LOW_SPACE$", 20, 0);
        close(clientSocket);
        return;
    }

    send(clientSocket, "$SUCCESS$", 9, 0);


    int fileDescriptor = open(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor < 0)
    {
        perror("Error creating file");
        close(clientSocket);
        return;
    }

    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        write(fileDescriptor, buffer, bytesRead);
    }

    if (bytesRead < 0)
    {
        perror("Error receiving file data");
    }
    else
    {
        send(clientSocket, "$SUCCESS$", 9, 0);
    }

    close(fileDescriptor);
    close(clientSocket);
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
    int n;
    printf("1.Upload\n2.Download : ");
    scanf("%d",n);
    if(n==1){

        char filePath[256];
        printf("Enter file path to upload: ");
        scanf("%s", filePath);

        uploadFile(clientSocket, filePath);
    }
    else{
        char filename[30];
        printf("Enter file path to Download: ");
        scanf("%s", filename);
        DownloadFile(clientSocket,filename);
    }
    close(clientSocket);
    return 0;
}
