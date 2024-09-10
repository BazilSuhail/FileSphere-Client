#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_SIZE 1024

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
                    }
                }
                else
                {
                    printf("Failed to receive server response.\n");
                }
            }
            else if (fileOption == 2)
            {
                printf("File download functionality is not yet implemented.\n");
            }
        }
    }

    close(clientSocket);
    return 0;
}
