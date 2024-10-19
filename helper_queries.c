#include "helper.h" 

void view_files(int clientSocket)
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

    printf("Receiving file data for: %s\n", fileName);

    char *encoded_data = (char *)malloc(MAX_SIZE);
    if (encoded_data == NULL)
    {
        perror("Memory allocation failed");
        return;
    }

    ssize_t total_bytes_received = 0;
    ssize_t bytesRead;

    FILE *tempFile = fopen("tempfile", "w"); // Writing to a temporary file
    if (tempFile == NULL)
    {
        perror("Error opening temp file for writing");
        free(encoded_data);
        return;
    }

    // Receive the file in chunks
    while ((bytesRead = recv(clientSocket, encoded_data + total_bytes_received, MAX_SIZE - total_bytes_received, 0)) > 0)
    {
        fwrite(encoded_data + total_bytes_received, 1, bytesRead, tempFile);
        total_bytes_received += bytesRead;
    }

    fclose(tempFile);

    if (bytesRead < 0)
    {
        perror("Error receiving file data");
        free(encoded_data);
        return;
    }

    printf("\nDecoding and writing file: %s\n", fileName);
    int received_file_size = getDecodedFileSize("tempfile");
    printf("\n%d\n",received_file_size);
    
    char *decoded_data = rle_decode(encoded_data, received_file_size, fileName); // Decoding data and writing to final file
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

void deleteFileRequest(int clientSocket, const char *fileName)
{
    char serverResponse[1024];
    ssize_t bytesRead;

    if (send(clientSocket, fileName, strlen(fileName), 0) < 0)
    {
        perror("Error sending filename to server");
        return;
    }

    printf("Filename '%s' sent to server.\n", fileName);

    while ((bytesRead = recv(clientSocket, serverResponse, sizeof(serverResponse) - 1, 0)) > 0)
    {
        serverResponse[bytesRead] = '\0';
        printf("Server response: %s\n", serverResponse);

        if (strstr(serverResponse, "File is Found !!") || strstr(serverResponse, "File not Found") || strstr(serverResponse, "Error"))
        {
            break;
        }
    }

    if (bytesRead < 0)
    {
        perror("Error receiving response from server");
    }
}

void update_file(int clientSocket, const char *filePath)
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

void replace_file(int clientSocket, const char *filePath)
{ 
    int encoded_length;
    char *encoded = rle_encode(filePath, &encoded_length);
    if (encoded == NULL)
    {
        perror("Error encoding file");
        return;
    }

    // Notify the server that we're ready to send the file content
    char response[256];
    ssize_t receivedBytes = recv(clientSocket, response, sizeof(response) - 1, 0);
    if (receivedBytes < 0)
    {
        perror("Error receiving acknowledgment from server");
        free(encoded);
        return;
    }
    response[receivedBytes] = '\0';

    // Open the encoded file (assumed encoded data is saved in "encoded_data.txt")
    FILE *encoded_file = fopen("encoded_data.txt", "r");
    if (encoded_file == NULL)
    {
        perror("Error opening encoded data file");
        free(encoded);
        return;
    }

    // Send the encoded file content in chunks to the server
    char buffer[1024];
    ssize_t sentBytes;
    while ((receivedBytes = fread(buffer, sizeof(char), sizeof(buffer), encoded_file)) > 0)
    {
        sentBytes = send(clientSocket, buffer, receivedBytes, 0);
        if (sentBytes < 0)
        {
            perror("Error sending encoded file data");
            break;
        }
    }

    // Close the file after sending its content
    fclose(encoded_file);

    // Check if the file was successfully sent
    if (sentBytes >= 0)
    {
        printf("Encoded file data Replaced successfully.\n");
    }

    // Remove the encoded file after successful transmission
    if (remove("encoded_data.txt") != 0)
    {
        perror("Error deleting encoded_data.txt file");
    }

    // Free the allocated memory for the encoded data
    free(encoded);
}

void handleFileExistsResponse(int serverSocket, const char *fileName)
{
    // Prompt user for input (1 for Yes, 0 for No)
    char userInput[2];
    printf("\nFile already exists, would you like to replace the file in the destination? (Enter 1 for Yes, 0 for No): ");
    scanf("%1s", userInput);

    // Send the user's input to the server
    send(serverSocket, userInput, 1, 0);
    if (userInput[0] == '1')
    {
        update_file(serverSocket, fileName); // Call function to update the file
    }
    else
    {
        //printf("File replacement canceled.\n"); // Action when the user enters '0'

        replace_file(serverSocket,fileName);
    }
}
