#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_SIZE 1024


void downloadFile(int clientSocket, const char *fileName)
{ 
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        perror("Error sending file name to server");
        return;
    }
 
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

    // Ensure the server is ready to send the file by checking "$READY$"
    /*if (strcmp(response, "$READY$") == 0)
    {
        printf("Server is ready to send the file.\n");
    }
    else
    {
        printf("Unexpected server response: %s\n", response);
        return;
    }*/

    int fileDescriptor = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fileDescriptor < 0)
    {
        perror("Error creating file");
        return;
    }

    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (write(fileDescriptor, buffer, bytesRead) != bytesRead)
        {
            perror("Error writing to file");
            close(fileDescriptor);
            return;
        }
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
    int fileDescriptor = open(filePath, O_RDONLY);
    if (fileDescriptor < 0)
    {
        perror("Error opening file");
        return;
    }
 
    const char *fileName = strrchr(filePath, '/');
    fileName = fileName ? fileName + 1 : filePath;

    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        perror("Error sending file name to server");
        close(fileDescriptor);
        return;
    }

    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        perror("Error receiving acknowledgment from server");
        close(fileDescriptor);
        return;
    }
    response[receivedBytes] = '\0';

    /*if (strcmp(response, "$READY$") != 0)
    {
        printf("Server is not ready to receive the file.\n");
        close(fileDescriptor);
        return;
    }*/
 
    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0)
    {
        send(clientSocket, buffer, bytesRead, 0);
    }

    printf("File uploaded successfully.\n");
    close(fileDescriptor);
}

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8081);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Error connecting to server");
        return 1;
    }

    printf("Select an option:\n1. Register\n2. Authenticate\n");
    int option;
    scanf("%d", &option);
    getchar();
    send(clientSocket, &option, sizeof(option), 0);

    if (option == 1)
    {
        // Register new user
        char userName[MAX_SIZE];
        printf("Enter username: ");
        scanf("%s", userName);
        send(clientSocket, userName, strlen(userName), 0);

        char password[MAX_SIZE];
        printf("Enter password: ");
        scanf("%s", password);
        send(clientSocket, password, strlen(password), 0);


        printf("\n$ USER IS REGISTERED $\n");
    }
    else if (option == 2)
    {
        // Authenticate user
        char userName[MAX_SIZE];
        printf("Enter username: ");
        scanf("%s", userName);
        send(clientSocket, userName, strlen(userName), 0);

        char password[MAX_SIZE];
        printf("Enter password: ");
        scanf("%s", password);
        send(clientSocket, password, strlen(password), 0);

        char response[MAX_SIZE];
        recv(clientSocket, response, sizeof(response) - 1, 0);
        response[sizeof(response) - 1] = '\0';
        printf("Server: %s\n", response);

        if (strcmp(response, "User found") == 0)
        {
            printf("Select an option:\n1. Upload file\n2. Download file\n");
            int fileOption;
            scanf("%d", &fileOption);
            getchar();
            send(clientSocket, &fileOption, sizeof(fileOption), 0);

            if (fileOption == 1)
            {
                char fileName[MAX_SIZE];
                printf("Enter file name: ");
                scanf("%s", fileName);
                send(clientSocket, fileName, strlen(fileName), 0);

                // file name path , baad ke liyee
                int fileDescriptor = open(fileName, O_RDONLY);
                if (fileDescriptor < 0)
                {
                    perror("File Not found | Error opening file :/");
                    close(clientSocket);
                    return 0;
                }

                long fileSize;
                printf("Enter file size: ");
                scanf("%ld", &fileSize);
                send(clientSocket, &fileSize, sizeof(fileSize), 0);

                printf("File name and size sent.\n");

                char response[MAX_SIZE];
                int bytesRead = recv(clientSocket, response, sizeof(response) - 1, 0);
                if (bytesRead > 0)
                {
                    response[bytesRead] = '\0';
                    printf("Server response: %s\n", response);

                    if (strcmp(response, "Out of space.") == 0)
                    {
                        printf("Server indicates out of space.\n");
                    }
                    else if (strcmp(response, "Error parsing config file.") == 0 ||
                             strcmp(response, "Error updating config file.") == 0 ||
                             strcmp(response, "Error writing to config file.") == 0)
                    {
                        printf("Server error: %s\n", response);
                    }
                    else
                    {
                        printf("Server message: %s\n", response);
                        uploadFile(clientSocket, fileName);
                    }
                }
                else
                {
                    printf("Failed to receive server response.\n");
                }
            }
            else if (fileOption == 2)
            {
                // Prompt user for file name to download
                char fileName[512];
                printf("Enter the file name to download: ");
                scanf("%s", fileName);

                // Send the file name to the server
                send(clientSocket, fileName, strlen(fileName), 0);

                // Receive response from the server
                char response[MAX_SIZE];
                int bytesReceived = recv(clientSocket, response, sizeof(response) - 1, 0);
                if (bytesReceived > 0)
                {
                    response[bytesReceived] = '\0'; // Null-terminate the response
                    printf("Server response: %s\n", response);

                    if (strcmp(response, "File found.") == 0)
                    {
                        // Proceed with file download logic if file is found
                        printf("File '%s' is available for download.\n", fileName);
                        downloadFile(clientSocket, fileName);
                        // Additional download logic would go here (e.g., file transfer)
                    }
                    else if (strcmp(response, "File not found.") == 0)
                    {
                        printf("The requested file '%s' does not exist on the server.\n", fileName);
                    }
                    else
                    {
                        printf("Unexpected server response: %s\n", response);
                    }
                }
                else
                {
                    printf("Failed to receive response from the server.\n");
                }
            }
        }
    }

    close(clientSocket);
    return 0;
}
