#include "helper.h"

char *getFileSize(const char *fileName)
{
    FILE *file = fopen(fileName, "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize == -1)
    {
        perror("Error getting file size");
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Allocate memory for the size string
    char *sizeStr = (char *)malloc(MAX_SIZE * sizeof(char));
    if (sizeStr == NULL)
    {
        perror("Error allocating memory");
        return NULL;
    }

    sprintf(sizeStr, "%ld", fileSize);
    return sizeStr;
}

int getDecodedFileSize(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return -1;
    }

    int total_size = 0;
    char ch;
    while ((ch = fgetc(file)) != EOF)
    {
        int count = 0;
        switch (ch)
        {
        case '*':                      // Single digit count
            count = fgetc(file) - '0'; // Convert char to int
            total_size += count;
            fgetc(file); // Skip the character after the count (e.g., 'a' or '\n')
            break;
        case '#':                                                   // Two digit count
            count = (fgetc(file) - '0') * 10 + (fgetc(file) - '0'); // Convert two chars to int
            total_size += count;
            fgetc(file); // Skip the character after the count (e.g., 'b' or '\n')
            break;
        case '@': // Three digit count
            count = (fgetc(file) - '0') * 100 + (fgetc(file) - '0') * 10 + (fgetc(file) - '0');
            total_size += count;
            fgetc(file); // Skip the character after the count (e.g., 'c' or '\n')
            break;
        default:
            if (ch == '\n')
            {
                total_size += 1; // Count newlines as part of the decoded data
            }
            else
            {
                // Handle four-digit counts
                count = (ch - '0') * 1000 + (fgetc(file) - '0') * 100 +
                        (fgetc(file) - '0') * 10 + (fgetc(file) - '0');
                total_size += count;
                fgetc(file); // Skip the character after the count
            }
            break;
        }
    }

    fclose(file);

    printf("\n file decoded :%d\n",total_size);
    
    return total_size;
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
    int estimated_size = encoded_length;
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
