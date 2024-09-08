#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#define MAX_SIZE 1024

void Sleeping()
{
    for(int i=0;i<100000;i++)
    {
        for(int j=0;j<10000;j++)
        {
            for(int k=0;k<100;k++);
        }
    }
}

void downloadFile(int clientSocket, const char *fileName)
{
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    Sleeping();
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
    Sleeping();
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
        ssize_t bytesSent = send(clientSocket, buffer, bytesRead, 0);Sleeping();
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
int Authentication(const char *input, char *status, char *username, char *password)
{
    int len = strlen(input);
    if (len < 5 || input[0] != '$' || input[len - 1] != '$')
    {
        printf("Invalid command format. Must start and end with '$'.\n");
        return 0; 
    }
    char *inputCopy = strdup(input);
    inputCopy[len - 1] = '\0'; // Remove trailing $
    inputCopy++;               // Skip the leading $
    char *token = strtok(inputCopy, "$");
    if (token == NULL)
    {
        printf("Invalid command format. Missing status.\n");
        return 0;
    }
    strcpy(status, token); // First token is the status
    if (strcmp(status, "LOGIN") != 0 && strcmp(status, "SIGNUP") != 0)
    {
        printf("Invalid status. Please specify 'LOGIN' or 'SIGNUP'.\n");
        return 0;
    }
    // Get the username
    token = strtok(NULL, "$");
    if (token == NULL)
    {
        printf("Invalid command format. Missing username.\n");
        return 0;
    }
    strcpy(username, token); // Second token is the username

    // Get the password
    token = strtok(NULL, "$");
    if (token == NULL)
    {
        printf("Invalid command format. Missing password.\n");
        return 0;
    }
    strcpy(password, token); // Third token is the password

    // Ensure no extra data after password
    token = strtok(NULL, "$");
    if (token != NULL)
    {
        printf("Invalid command format. Too many fields.\n");
        return 0;
    }

    return 1; // Return 1 for success
}
int userValidation(int clientSocket,const char *status, const char *username, const char *password)
{
    char buffer[1024] = {0};
    if (strncmp(status, "LOGIN",5) == 0)
    {
        if(send(clientSocket, "1", 1, 0)<0){
            Sleeping();
            printf("Error while Sending status to Server");
            return 0;
        }  
        printf("Status sent");
        if(send(clientSocket, username, strlen(username), 0)<0){Sleeping();
            printf("Error while Sending Username to Server");
            return 0;
        }
        printf("Username sent");
        
        // Send the password
        if(send(clientSocket, password, strlen(password), 0)<0){Sleeping();
            printf("Error while Sending Password to Server");
            return 0;
        }
        if(recv(clientSocket, buffer, sizeof(buffer), 0)<0){
            printf("Error while Receiving Authentication from server");
            return 0;
        }  // Wait for server's response
        printf("Password sent");

        // Check server's response
        if (strcmp(buffer, "1") == 0)
        {
            printf("Login successful! You can perform further operations.\n");
        }
        else if (strcmp(buffer, "3") == 0)
        {
            printf("Login failed! Username or password incorrect.\n");
        }
    }
    // If status is SIGNUP, send "2" to server
    else if (strncmp(status, "SIGNUP",6) == 0)
    {
        if(send(clientSocket, "2", 1, 0)<=0){Sleeping();
            printf("Error while Sending status to Server");
            return 0;
        }  
        printf("Status sent");
        if(send(clientSocket, username, strlen(username), 0)<0){Sleeping();
            printf("Error while Sending Username to Server");
            return 0;
        }
        printf("Username sent");
        if(send(clientSocket, password, strlen(password), 0)<0){Sleeping();
            printf("Error while Sending Password to Server");
            return 0;
        }
        printf("Password sent");

        if(recv(clientSocket, buffer, sizeof(buffer), 0)<0){
            printf("Error while Receiving Authentication from server");
            return 0;
        }  

        // Check server's response
        if (strcmp(buffer, "2") == 0)
        {
            printf("Signup successful! You can perform further operations.\n");
        }
    }
    return 1;
}
void viewFiles(int clientSocket)
{ 
    const char *operation = "3";
    ssize_t sentBytes = send(clientSocket, operation, strlen(operation), 0);Sleeping();
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
    char status[20], username[50], password[50];
    int auth=1;
    do{
        char input[256];
        printf("Enter your command: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = '\0';
        
        auth = Authentication(input, status, username, password);
    }while(auth==0);

    
    auth = userValidation(clientSocket,status,username,password);
    if(auth==0){return 1;}
    char command[512];
    printf("-> ");
    scanf("%s", command);

    if (strncmp(command, "$UPLOAD$", 8) == 0)
    { 
        const char *filePath = command + 8;
        send(clientSocket, "2", 1, 0);
        Sleeping();
        uploadFile(clientSocket, filePath);
    }
    else if (strncmp(command, "$DOWNLOAD$", 10) == 0)
    { 
        const char *fileName = command + 10;
        send(clientSocket, "1", 1, 0);Sleeping();
        downloadFile(clientSocket, fileName);
    }
    else if (strncmp(command, "$VIEW$", 10) == 0)
    {
        send(clientSocket, "3", 1, 0);Sleeping();
        viewFiles(clientSocket);
    }
    else
    {
        printf("Invalid command.\n");
    }

    close(clientSocket);
    return 0;
}