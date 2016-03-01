#include <stdbool.h>
#include <string.h>

#include "operators.h"

int is_file_operator(char* str) 
{
    const char* const fileop[3] = { OP_OUTPUT, OP_INPUT, OP_APPEND };     
    for (int i = 0; i < 3; i++) 
    {
        if (strcmp(str, fileop[i]) == 0) return true;
    }
    return false;
}

int is_terminating_operator(char* str) 
{
    const char* const terminating[4] = { OP_PIPE, OP_ON_SUCCESS, OP_ON_FAIL, OP_ON_ANY };
    for (int i = 0; i < 4; i++) 
    {
        if (strcmp(str, terminating[i]) == 0) return true;
    }
    return false;
}
