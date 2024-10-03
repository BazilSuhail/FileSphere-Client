#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>

#define MAX_SIZE 1024
#define PORT 8081

void receiveFileData(int clientSocket)
{
    char buffer[MAX_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    if (bytesRead < 0)
    {
        perror("Error receiving data");
    }
    else
    {
        printf("\nFile data received successfully\n");
    }
}


char rle_decode(const char *encoded_data, int encoded_length, FILE* fp) {
    char *decoded = (char *)malloc(sizeof(char) * encoded_length*2);
    int i, j = 0;

    for (i = 0; i < encoded_length; ) {
        int count=0;
        switch (encoded_data[i])
        {
            case '*':{
                count=encoded_data[i+1]-'0';
                i++;
                break;
            }
            case '#':
            {
                count=encoded_data[i+1]-'0';
                count =count*10+ (encoded_data[i + 2]-'0');
                i += 2;
                break;
            }
            case '@':{
                count=encoded_data[i+1]-'0';
                count =count*10+ (encoded_data[i + 2]-'0');
                count =count*10+ (encoded_data[i + 3]-'0');
                i += 3;
                break;
            }
            
            default:
            {
                count=encoded_data[i+1]-'0';
                count =count*10+ (encoded_data[i + 2]-'0');
                count =count*10+ (encoded_data[i + 3]-'0');
                count =count*10+ (encoded_data[i + 4]-'0');
                i += 4;
                break;
            }
        }
        i++;
        for(int k=0;k<count;k++){
            decoded[j++]=encoded_data[i];
        }
        i++;
    }

    
    char * file_write=decoded;
    if(j>1024)
    {
       int quo=j/1024;
       j=j%1024;
       int Buffer =1024;
       for (int i = 0; i < quo; i++)
       {
          fwrite(file_write, sizeof(char), Buffer, fp);
       }
       file_write+=Buffer;
    }
    fwrite(file_write, sizeof(char), j, fp);
    free(decoded);
    return 'y';
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

    /*if (strcmp(response, "$READY$") != 0)
    {
        printf("Server is not ready to send the file.\n");
        return;
    }*/

   

    char buffer[MAX_SIZE];
    ssize_t bytesRead;
    FILE *fp = fopen(fileName, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        return NULL;
    }
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        if (rle_decode(buffer, bytesRead, fp) != 'y')
        {
            perror("Error writing to file");
            close(fp);
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

    close(fp);
}


char *rle_encode(const char *filename, int *encoded_leng) {

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error opening input file");
        return NULL;
    }
    char *data = NULL;
    size_t buffer_size = 0;
    size_t nread;

    char *encoded = (char *)malloc(sizeof(char) * 1024);
    int encoded_capacity = 1024;
    int encoded_length = 0;
    int len=0;
    while ((nread = getline(&data, &buffer_size, fp)) != -1) {
        // Process the line of data (e.g., remove newline)
        data[strcspn(data, "\n")] = '\0';

        for (int i = 0; i < strlen(data); i++) {
            int count = 1;
            while (i + 1 < strlen(data) && data[i + 1] == data[i]) {
                count++;
                i++;
            }
            len+=count+3;
            if (encoded_length + 9 >= encoded_capacity) {
                // Reallocate buffer if needed
                encoded_capacity *= 2;
                encoded = (char *)realloc(encoded, encoded_capacity);
            }

            if (count < 9) {
                encoded[encoded_length++] = '*';
                encoded[encoded_length++] = '0' + count;
            } else if (count < 100) {
                encoded[encoded_length++] = '#';
                encoded[encoded_length++] = '0' + (count / 10);
                encoded[encoded_length++] = '0' + (count % 10);
            } else if (count < 1000) {
                encoded[encoded_length++] = '@';
                encoded[encoded_length++] = '0' + (count / 100);
                encoded[encoded_length++] = '0' + ((count % 100) / 10);
                encoded[encoded_length++] = '0' + (count % 10);
            } else {
                encoded[encoded_length++] = '$';
                encoded[encoded_length++] = '0' + (count / 1000);
                encoded[encoded_length++] = '0' + ((count % 1000) / 100);
                encoded[encoded_length++] = '0' + ((count % 100) / 10);
                encoded[encoded_length++] = '0' + (count % 10);
            }

            
            encoded[encoded_length++] = data[i];

        }

        // Add newline character
        encoded[encoded_length++] = '*';
        encoded[encoded_length++] = '1';
        encoded[encoded_length++] = '\n';
    }

    fclose(fp);

    *encoded_leng = encoded_length;
    return encoded;
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
    int Encoded_size;
    char* encoded = rle_encode(fileName,&Encoded_size);
    if(send(clientSocket,&Encoded_size,sizeof(Encoded_size),0)<0){
        printf("Error in Sending File size to server!!!!");
    }
    if(Encoded_size>MAX_SIZE){
        int Quo=Encoded_size/MAX_SIZE;
        int remain=Encoded_size%MAX_SIZE;
        while(Quo>0){
        if(send(clientSocket,encoded,MAX_SIZE,0)<0){
            printf("Error in sending Data to server");
            Quo--;
            encoded+=MAX_SIZE;
            }
        }
        if(send(clientSocket,encoded,remain,0)<0)
            printf("Error in sendsing Data to server");
    }
    else{
         if(send(clientSocket,encoded,Encoded_size,0)<0)
            printf("Error in sendsing Data to server");
        }
    

    printf("File uploaded successfully.\n");
    close(fileDescriptor);
}

long getFileSize(const char *filename) {
    FILE *fp = fopen(filename, "rb"); 
    if (fp == NULL) {
        perror("Error opening file");
        return -1; 
    }

    fseek(fp, 0, SEEK_END); 
    long size = ftell(fp);
    rewind(fp);

    fclose(fp);
    return size;
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

    // int option;
    // scanf("%d", &option);
    // getchar();
    // send(clientSocket, &option, sizeof(option), 0);

    if (strncmp(command, "$REGISTER$", 10) == 0)
    {
        auth_code = 1;
        send(clientSocket, &auth_code, sizeof(auth_code), 0);

        // Register new user
        char userName[MAX_SIZE];
        printf("\nEnter username: ");
        scanf("%s", userName);
        send(clientSocket, userName, strlen(userName), 0);

        char password[MAX_SIZE];
        printf("Enter password: ");
        scanf("%s", password);
        send(clientSocket, password, strlen(password), 0);

        printf("$ \"%s\"'s ACCOUNT HAS BEEN REGISTERED$\n", userName);
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
            // printf("Select an option:\n1. Upload file\n2. Download file\n");
            printf("Type:\n$UPLOAD$<file-name> for Uploading Data/File | $DOWNLOAD$<file-name> for Downloading an Uploaded Data/File\n");

            char upload_download_command[512];

            printf("-> ");
            scanf("%s", upload_download_command);

            int option;

            /*
            // signal to server
            if (strncmp(upload_download_command, "$UPLOAD$", 8) == 0)
            {
                option = 1;
                send(clientSocket, &option, sizeof(option), 0);
            }
            else if (strncmp(upload_download_command, "$DOWNLOAD$", 10) == 0)
            {
                option = 2;
                send(clientSocket, &option, sizeof(option), 0);
            }
            else if (strncmp(upload_download_command, "$VIEW$", 6) == 0)
            {
                option = 3;
                send(clientSocket, &option, sizeof(option), 0);
            }
            else
            {
                printf("Invalid command.\n");
            }*/

            /*int fileOption;
            scanf("%d", &fileOption);
            getchar();
            send(clientSocket, &fileOption, sizeof(fileOption), 0);*/

            if (strncmp(upload_download_command, "$UPLOAD$", 8) == 0)
            {
                option = 1;
                send(clientSocket, &option, sizeof(option), 0);

                // char fileName[MAX_SIZE];
                // printf("Enter file name: ");
                // scanf("%s", fileName);

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
                long val;
                scanf("%ld",&val);

                long fileSize=getFileSize(fileName);
                
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
                // char fileName[512];
                // printf("Enter the file name to download: ");
                // scanf("%s", fileName);

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
