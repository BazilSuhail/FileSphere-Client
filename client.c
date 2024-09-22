#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h> 

#define MAX_FILES 100
#define MAX_FILENAME_SIZE 256
#define MAX_SIZE 1024


void receiveFileData(int clientSocket)
{
    char buffer[MAX_SIZE];
    ssize_t bytesRead;

    char fileNames[MAX_FILES][MAX_FILENAME_SIZE]; // Array to store file names
    int fileSizes[MAX_FILES];                     // Array to store file sizes
    int fileCount = 0;
    int totalSize = 0;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesRead] = '\0'; // Null terminate the received data

        char *line = strtok(buffer, "\n"); // Split the buffer by new lines
        while (line != NULL)
        {
            // Parse file name and size
            char fileName[MAX_FILENAME_SIZE];
            int fileSize;

            // Scan the line and extract file name and size
            if (sscanf(line, "%s - %d", fileName, &fileSize) == 2)
            {
                // Store file name and size in arrays
                strncpy(fileNames[fileCount], fileName, MAX_FILENAME_SIZE);
                fileSizes[fileCount] = fileSize;

                // Sum the file size
                totalSize += fileSize;
                fileCount++;
            }

            line = strtok(NULL, "\n"); // Continue with the next line
        }
    }

    if (bytesRead < 0)
    {
        perror("Error receiving data");
    }
    else
    {
        printf("\nFile data received successfully\n\n");

        // Display the file names and sizes
        for (int i = 0; i < fileCount; i++)
        {
            printf("File Name: %s - FileSize: %d\n", fileNames[i], fileSizes[i]);
        }

        // Display the total file size
        printf("\nTotal File Size: %d\n\n", totalSize);
    }
}

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

    printf("\n\n%s\n", response);
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

    printf("Type:\n$REGISTER$ for Signing Up\n$LOGIN$ for Signing In\n");

    char command[512];

    printf("-> ");
    scanf("%s", command);

    int auth_code;

    if (strncmp(command, "$REGISTER$", 10) != 0 && strncmp(command, "$LOGIN$", 7) != 0)
    {
        printf("Invalid command.\n");
    }
 
    if (strncmp(command, "$REGISTER$", 10) == 0)
    {
        auth_code = 1;
        send(clientSocket, &auth_code, sizeof(auth_code), 0);

        // Register new user
        char userName[MAX_SIZE];
        printf("\nEnter username: ");
        scanf("%s", userName);

        // Send the username to the server
        send(clientSocket, userName, strlen(userName), 0);

        char password[MAX_SIZE];
        printf("Enter password: ");
        scanf("%s", password);

        // Send the password to the server
        send(clientSocket, password, strlen(password), 0);

        // Receive the signal from the server (0 for already exists, 1 for newly created)
        int userExists;
        recv(clientSocket, &userExists, sizeof(userExists), 0);

        // Display appropriate message based on the signal
        if (userExists == 1)
        {
            printf("$ \"%s\"'s ACCOUNT HAS BEEN REGISTERED$\n", userName);
        }
        else if (userExists == 0)
        {
            printf("$ ACCOUNT FOR \"%s\" ALREADY EXISTS$\n", userName);
        }
        else
        {
            printf("$ ERROR: Invalid response from server $\n");
        }
    }

    else if (strncmp(command, "$LOGIN$", 7) == 0)
    {
        auth_code = 2;
        send(clientSocket, &auth_code, sizeof(auth_code), 0);

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
            printf("\nAvailable Commands:\n\n$UPLOAD$<file-name> for Uploading Data/File\n$DOWNLOAD$<file-name> for Downloading an Uploaded Data/File\n\n");

            char upload_download_command[512];

            printf("-> ");
            scanf("%s", upload_download_command);

            int option;
            if (strncmp(upload_download_command, "$UPLOAD$", 8) == 0)
            {
                option = 1;
                send(clientSocket, &option, sizeof(option), 0);
 
                const char *fileName = upload_download_command + 8;

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
                printf("-> Enter file size: ");
                scanf("%ld", &fileSize);
                send(clientSocket, &fileSize, sizeof(fileSize), 0);

                printf("File name and size sent.\n");

                char response[MAX_SIZE];
                int bytesRead = recv(clientSocket, response, sizeof(response) - 1, 0);
                if (bytesRead > 0)
                {
                    response[bytesRead] = '\0';
                    printf("\nServer response: %s\n", response);

                    if (strcmp(response, "Out of space.") == 0)
                    {
                        printf("Server indicates out of space.\n");
                    }
                    else if (strcmp(response, "Error parsing config file.") == 0 ||
                             strcmp(response, "Error updating config file.") == 0 ||
                             strcmp(response, "File already exists.") == 0 ||
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

            // Prompt user for file name to download
            else if (strncmp(upload_download_command, "$DOWNLOAD$", 10) == 0)
            {
                option = 2;
                send(clientSocket, &option, sizeof(option), 0);
                const char *fileName = upload_download_command + 10;

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

            else if (strncmp(upload_download_command, "$VIEW$", 6) == 0)
            {
                option = 3;
                send(clientSocket, &option, sizeof(option), 0);

                receiveFileData(clientSocket);
            }
        }
    }

    close(clientSocket);
    return 0;
}
