#ifndef YK_FILE_READER_H
#define YK_FILE_READER_H

#include <stdio.h>
#include <stdlib.h>

/*
    ToDo (facts 12/22 2357): Use transient memory instead of allocating it 
*/

/*
    Reads a file at filepath and retuns data as a char *
    The returned char* needs to be freed by the caller.
*/
char* yk_read_text_file(const char* filepath);

/*
    Reads a binary file (no next transformations applied) at filepath and retuns data as a char *
    The returned char* needs to be freed by the caller.
*/
char* yk_read_binary_file(const char* filename, size_t* fileSize);
#endif