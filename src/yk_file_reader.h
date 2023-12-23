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
char* yk_file_reader(const char* filepath);



#endif