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
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Error connecting to server");
        return 1;
    }

    printf("Select an option:\n1. Register\n2. Authenticate\n");
    int option;
    scanf("%d", &option);
    getchar(); // To consume the newline character left by scanf
    send(clientSocket, &option, sizeof(option), 0);

    if (option == 1)
    {
        // Register new user
        char userName[MAX_SIZE];
        printf("Enter username: ");
        scanf("%s", userName);
        send(clientSocket, userName, strlen(userName), 0);

        char response[MAX_SIZE];
        recv(clientSocket, response, sizeof(response) - 1, 0);
        response[strlen(response)] = '\0'; // Ensure null termination
        printf("%s\n", response);

        if (strcmp(response, "User created") == 0)
        {
            char password[MAX_SIZE];
            printf("Enter password: ");
            scanf("%s", password);
            send(clientSocket, password, strlen(password), 0);
        }
    }
    else if (option == 2)
    {
        // Authenticate user
        char userName[MAX_SIZE];
        printf("Enter username: ");
        scanf("%s", userName);
        send(clientSocket, userName, strlen(userName), 0);

        char response[MAX_SIZE];
        recv(clientSocket, response, sizeof(response) - 1, 0);
        response[strlen(response)] = '\0'; // Ensure null termination
        printf("%s\n", response);

        if (strcmp(response, "User found") == 0)
        {
            char password[MAX_SIZE];
            printf("Enter password: ");
            scanf("%s", password);
            send(clientSocket, password, strlen(password), 0);

            recv(clientSocket, response, sizeof(response) - 1, 0);
            response[strlen(response)] = '\0'; // Ensure null termination
            printf("%s\n", response);
        }
    }

    close(clientSocket);
    return 0;
}
