#include <yk_file_reader.h>

/*
fopen_s() is only defined in windows
*/
#ifdef __unix
#define fopen_s(pFile, filepath, mode) ((*(pFile)) = fopen((filepath), (mode))) == NULL
#endif

char* yk_read_text_file(const char* filepath)
{
    FILE* file;
    fopen_s(&file, filepath, "r");

    if (file == NULL)
    {
        fprintf(stderr, "Error: Unable to open the file %s\n", filepath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (length == 0)
    {
        fprintf(stderr, "Error: File is empty\n");
        fclose(file);
        return NULL;
    }

    char* string = malloc(sizeof(char) * (length + 1));
    if (string == NULL)
    {
        fprintf(stderr, "Error: File reader malloc failed for file %s\n", filepath);
        fclose(file);
        return NULL;
    }

    char c;
    int i = 0;

    while ((c = fgetc(file)) != EOF)
    {
        string[i] = c;
        i++;
    }
    string[i] = '\0';

    fclose(file);

    return string;
}

char* yk_read_binary_file(const char* filename, size_t* fileSize) 
{
    FILE* file;
    fopen_s(&file, filename, "rb");

    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(*fileSize);
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, 1, *fileSize, file) != *fileSize) 
    {
        perror("Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);

    return buffer;
}
