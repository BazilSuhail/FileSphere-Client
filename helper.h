// my_functions.h
#ifndef MY_FUNCTIONS_H
#define MY_FUNCTIONS_H

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

char* getFileSize(const char* fileName) ;
int getDecodedFileSize(const char *fileName);
char *rle_encode(const char *filename, int *encoded_leng);
char *rle_decode(const char *encoded_data, int encoded_length, const char *output_filename);

void view_files(int clientSocket);
void downloadFile(int clientSocket, const char *fileName);
void uploadFile(int clientSocket, const char *filePath);
void deleteFileRequest(int clientSocket, const char *fileName);
void update_file(int clientSocket, const char *filePath);
void replace_file(int clientSocket, const char *filePath);
void handleFileExistsResponse(int serverSocket, const char *fileName);

#endif