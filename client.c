#include "helper.h"

#define PORT 8000

void handle_Upload(int clientSocket, char *upload_download_command)
{
    // int option = 1;
    // send(clientSocket, &option, sizeof(option), 0);
    send(clientSocket, &(int){1}, sizeof(int), 0);

    const char *fileName = upload_download_command + 8;
    send(clientSocket, fileName, strlen(fileName), 0);

    // Delimiter of newline is send, to separate file name size
    send(clientSocket, "\n", 1, 0);

    int fileDescriptor = open(fileName, O_RDONLY);
    if (fileDescriptor < 0)
    {
        perror("File Not found | Error opening file :/");
        close(clientSocket);
        return;
    }

    char *sizeStr = getFileSize(fileName);
    printf("File size: %s bytes\n", sizeStr);

    // Send the file size as a string
    send(clientSocket, sizeStr, strlen(sizeStr), 0);

    printf("File Sent to the Server.\n");

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
        else if (strcmp(response, "Error parsing config file.") == 0 || strcmp(response, "Error updating config file.") == 0 || strcmp(response, "Error writing to config file.") == 0)
        {
            printf("Server error: %s\n", response);
        }
        else if (strcmp(response, "File already exists.") == 0)
        {
            handleFileExistsResponse(clientSocket, fileName);
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

    close(fileDescriptor);
}

void handle_Downlaod(int clientSocket, char *upload_download_command)
{
    // option = 2;
    // send(clientSocket, &option, sizeof(option), 0);

    send(clientSocket, &(int){2}, sizeof(int), 0);

    const char *fileName = upload_download_command + 10;

    send(clientSocket, fileName, strlen(fileName), 0);

    char response[MAX_SIZE];
    int bytesReceived = recv(clientSocket, response, sizeof(response) - 1, 0);

    if (bytesReceived > 0)
    {
        response[bytesReceived] = '\0';
        printf("Server response: %s\n", response);

        if (strcmp(response, "File found.") == 0)
        {
            printf("File '%s' is available for download.\n", fileName);
            downloadFile(clientSocket, fileName);
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

void handle_ViewFiles(int clientSocket, char *upload_download_command)
{
    send(clientSocket, &(int){3}, sizeof(int), 0);
    view_files(clientSocket);
}

void handle_Delete_File(int clientSocket, char *upload_download_command)
{
    // option = 4;
    // send(clientSocket, &option, sizeof(option), 0);
    send(clientSocket, &(int){4}, sizeof(int), 0);
    const char *fileName = upload_download_command + 8;
    deleteFileRequest(clientSocket, fileName);
}

void handle_Update_File(int clientSocket, char *upload_download_command)
{
    send(clientSocket, &(int){5}, sizeof(int), 0);
    const char *fileName = upload_download_command + 8;
    update_file(clientSocket, fileName);
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
    serverAddr.sin_port = htons(PORT);
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

        char userName[MAX_SIZE];
        printf("\nEnter username: ");
        scanf("%s", userName);
        send(clientSocket, userName, strlen(userName), 0);

        char password[MAX_SIZE];
        printf("Enter password: ");
        scanf("%s", password);

        send(clientSocket, password, strlen(password), 0);

        int userExists;
        recv(clientSocket, &userExists, sizeof(userExists), 0);
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
            printf("\nAvailable Commands:\n\n$UPLOAD$<file-name> for Uploading Data/File\n$DOWNLOAD$<file-name> for Downloading an Uploaded Data/File\n$View$ for View Uploaded Files and there sizes\n$UPDATE$<file-name> for Updating and Existing/Uploaded File\n$DELETE$<file-name> for Deleting a Uploading File\n\n");

            char upload_download_command[512];

            printf("-> ");
            scanf("%s", upload_download_command);

            int option;
            if (strncmp(upload_download_command, "$UPLOAD$", 8) == 0)
            {
                handle_Upload(clientSocket, upload_download_command);
            }

            else if (strncmp(upload_download_command, "$DOWNLOAD$", 10) == 0)
            {
                handle_Downlaod(clientSocket, upload_download_command);
            }

            else if (strncmp(upload_download_command, "$VIEW$", 6) == 0)
            {
                handle_ViewFiles(clientSocket, upload_download_command);
            }

            else if (strncmp(upload_download_command, "$DELETE$", 8) == 0)
            {
                handle_Delete_File(clientSocket, upload_download_command);
            }

            else if (strncmp(upload_download_command, "$UPDATE$", 8) == 0)
            {
                handle_Update_File(clientSocket, upload_download_command);
            }
        }
    }

    close(clientSocket);
    return 0;
}
