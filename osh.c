#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "comm.h"

static const char VERBOSE[] = "-v";
static bool ISVERBOSE = false;

void verbose(char* str, ...) 
{
    if (!ISVERBOSE) return;
    
    va_list args;
    va_start(args, str);
    
    char buf[256];
    snprintf(buf, sizeof buf, "[VERBOSE] %s\n", str);
    
    printf(buf, args);
    va_end(args);    
}

char* readline() 
{
        int bytes_read;
        size_t nbytes = 100;
        char *str = (char *) malloc(nbytes + 1);
        if (str == NULL) exit (-1);
        bytes_read = getline(&str, &nbytes, stdin);
        str[strcspn(str, "\n")] = 0;
        if (bytes_read == -1)
        {
            free(str);
            verbose("Error reading string. Expect everything to go south.");
            return NULL;
        }
        return str;
}

char** split_tokens(char* input)
{
    char** result = NULL;
    int spaces = 0, i;
    char* p = strtok (input, " ");
    while (p)
    {
        result = realloc (result, sizeof (char*) * ++spaces);
        if (result == NULL) exit (-1);
        result[spaces-1] = p;
        p = strtok (NULL, " ");
    }
    result = realloc (result, sizeof (char*) * (spaces+1));
    result[spaces] = 0;
    
    return result;
}

int main(int argc, char **argv)
{
    if (argc > 1) 
    {
        ISVERBOSE = strcmp(argv[1], VERBOSE) == 0;
    }
    verbose("VERBOSE MODE! GET EXCITED!\n");
    
    while (true) {
        printf("osh> ");
        
        char* line = readline();
        char** tokens = split_tokens(line);
        
        verbose("Counted %lu tokens", sizeof(tokens)/sizeof(char*));
        
        
        int i;
        for(i = 0; *(tokens + i); i++) 
        {
            verbose(*(tokens + i));    
        }

        // handle the simple case straight away.
        if (strcmp(line, "exit") == 0 || (ISVERBOSE && strcmp(line, "q") == 0))
        {
            return 0;
        }

    }
}



