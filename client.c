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

int getFileSize(const char *filename, long *fileSize) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error: Unable to open file %s\n", filename);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fclose(file);

    return 0;
}


char *rle_encode(const char *filename, int *encoded_leng)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("Error opening input file");
        return NULL;
    }

    char *data = NULL;
    size_t buffer_size = 0;
    size_t nread;

    char *encoded = (char *)malloc(sizeof(char) * 1024);
    int encoded_capacity = 1024;
    int encoded_length = 0;
    int len = 0;

    while ((nread = getline(&data, &buffer_size, fp)) != -1)
    {
        data[strcspn(data, "\n")] = '\0';

        for (int i = 0; i < strlen(data); i++)
        {
            int count = 1;
            while (i + 1 < strlen(data) && data[i + 1] == data[i])
            {
                count++;
                i++;
            }
            len += count + 3;
            if (encoded_length + 4 >= encoded_capacity)
            {
                encoded_capacity *= 2;
                encoded = (char *)realloc(encoded, encoded_capacity);
            }

            if (count < 9)
            {
                encoded[encoded_length++] = '*';
                encoded[encoded_length++] = '0' + count;
            }
            else if (count < 100)
            {
                encoded[encoded_length++] = '#';
                encoded[encoded_length++] = '0' + (count / 10);
                encoded[encoded_length++] = '0' + (count % 10);
            }
            else if (count < 1000)
            {
                encoded[encoded_length++] = '@';
                encoded[encoded_length++] = '0' + (count / 100);
                encoded[encoded_length++] = '0' + ((count % 100) / 10);
                encoded[encoded_length++] = '0' + (count % 10);
            }
            else
            {
                encoded[encoded_length++] = '$';
                encoded[encoded_length++] = '0' + (count / 1000);
                encoded[encoded_length++] = '0' + ((count % 1000) / 100);
                encoded[encoded_length++] = '0' + ((count % 100) / 10);
                encoded[encoded_length++] = '0' + (count % 10);
            }

            encoded[encoded_length++] = data[i];
        }

        encoded[encoded_length++] = '*';
        encoded[encoded_length++] = '1';
        encoded[encoded_length++] = '\n';
    }

    fclose(fp);

    FILE *encoded_file = fopen("encoded_data.txt", "w");
    if (encoded_file == NULL)
    {
        perror("Error opening encoded data file");
        free(encoded);
        return NULL;
    }
    fwrite(encoded, sizeof(char), encoded_length, encoded_file);
    fclose(encoded_file);

    *encoded_leng = len;
    return encoded;
}

char *rle_decode(const char *encoded_data, int encoded_length, const char *output_filename)
{
    int estimated_size = encoded_length * 900;
    char *decoded = (char *)malloc(sizeof(char) * estimated_size);
    if (decoded == NULL)
    {
        perror("Error allocating memory for decoded data");
        return NULL;
    }

    int i = 0, j = 0;
    while (i < encoded_length)
    {
        int count = 0;
        switch (encoded_data[i])
        {
        case '*':
            count = encoded_data[i + 1] - '0';
            i += 2;
            break;
        case '#':
            count = (encoded_data[i + 1] - '0') * 10 + (encoded_data[i + 2] - '0');
            i += 3;
            break;
        case '@':
            count = (encoded_data[i + 1] - '0') * 100 + (encoded_data[i + 2] - '0') * 10 + (encoded_data[i + 3] - '0');
            i += 4;
            break;
        default:
            count = (encoded_data[i + 1] - '0') * 1000 + (encoded_data[i + 2] - '0') * 100 +
                    (encoded_data[i + 3] - '0') * 10 + (encoded_data[i + 4] - '0');
            i += 5;
            break;
        }

        if (j + count >= estimated_size)
        {
            estimated_size *= 2;
            decoded = (char *)realloc(decoded, sizeof(char) * estimated_size);
            if (decoded == NULL)
            {
                perror("Error reallocating memory for decoded data");
                return NULL;
            }
        }

        char ch = encoded_data[i++];
        for (int k = 0; k < count; k++)
        {
            decoded[j++] = ch;
        }
    }

    FILE *fp = fopen(output_filename, "w");
    if (fp == NULL)
    {
        perror("Error opening output file");
        free(decoded);
        return NULL;
    }
    fwrite(decoded, sizeof(char), j, fp);
    fclose(fp);

    return decoded;
}


void receiveFileData(int clientSocket)
{
    char buffer[MAX_SIZE];
    ssize_t bytesRead;

    char fileNames[MAX_FILES][MAX_FILENAME_SIZE];
    int fileSizes[MAX_FILES];
    int fileCount = 0;
    int totalSize = 0;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesRead] = '\0';

        char *line = strtok(buffer, "\n");
        while (line != NULL)
        {
            char fileName[MAX_FILENAME_SIZE];
            int fileSize;
            if (sscanf(line, "%s - %d", fileName, &fileSize) == 2)
            {
                strncpy(fileNames[fileCount], fileName, MAX_FILENAME_SIZE);
                fileSizes[fileCount] = fileSize;
                totalSize += fileSize;
                fileCount++;
            }

            line = strtok(NULL, "\n");
        }
    }

    if (bytesRead < 0)
    {
        perror("Error receiving data");
    }
    else
    {
        printf("\nFile data received successfully\n\n");

        for (int i = 0; i < fileCount; i++)
        {
            printf("File Name: %s - FileSize: %d\n", fileNames[i], fileSizes[i]);
        }
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

    printf("Receiving file data for: %s\n", fileName);

    char *encoded_data = (char *)malloc(MAX_SIZE);
    if (encoded_data == NULL)
    {
        perror("Memory allocation failed");
        return;
    }

    ssize_t total_bytes_received = 0;
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, encoded_data + total_bytes_received, MAX_SIZE - total_bytes_received, 0)) > 0)
    {
        total_bytes_received += bytesRead;
        fwrite(encoded_data + total_bytes_received - bytesRead, 1, bytesRead, stdout);
    }

    if (bytesRead < 0)
    {
        perror("Error receiving file data");
        free(encoded_data);
        return;
    }

    printf("\nDecoding and writing file: %s\n", fileName);
    char *decoded_data = rle_decode(encoded_data, total_bytes_received, fileName);
    if (decoded_data == NULL)
    {
        printf("Error decoding received data.\n");
    }
    else
    {
        printf("File downloaded and saved successfully.\n");
    }

    free(encoded_data);
    free(decoded_data);
}

void uploadFile(int clientSocket, const char *filePath)
{
    int encoded_length;
    char *encoded = rle_encode(filePath, &encoded_length);
    if (encoded == NULL)
    {
        perror("Error encoding file");
        return;
    }

    const char *fileName = strrchr(filePath, '/');
    fileName = fileName ? fileName + 1 : filePath;
    ssize_t sentBytes = send(clientSocket, fileName, strlen(fileName), 0);
    if (sentBytes < 0)
    {
        perror("Error sending file name to server");
        free(encoded);
        return;
    }

    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        perror("Error receiving acknowledgment from server");
        free(encoded);
        return;
    }
    response[receivedBytes] = '\0';

    FILE *encoded_file = fopen("encoded_data.txt", "r");
    if (encoded_file == NULL)
    {
        perror("Error opening encoded data file");
        free(encoded);
        return;
    }

    char buffer[1024];
    while ((receivedBytes = fread(buffer, sizeof(char), sizeof(buffer), encoded_file)) > 0)
    {
        sentBytes = send(clientSocket, buffer, receivedBytes, 0);
        if (sentBytes < 0)
        {
            perror("Error sending encoded file data");
            break;
        }
    }
    fclose(encoded_file);

    if (sentBytes >= 0)
    {
        printf("Encoded file data uploaded successfully.\n");
    }
    if (remove("encoded_data.txt") != 0)
    {
        perror("Error deleting encoded_data.txt file");
    }

    free(encoded);
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
                //getFileSize(fileName,&fileSize);
                printf("File size: %ld bytes\n", fileSize);

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

            else if (strncmp(upload_download_command, "$DOWNLOAD$", 10) == 0)
            {
                option = 2;
                send(clientSocket, &option, sizeof(option), 0);
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
