#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "command.h"
#include "operators.h"

#define verbose(...) if(ISVERBOSE) printf(__VA_ARGS__)
static bool ISVERBOSE = false;

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
        verbose("Error reading string\n");
        exit(-1);
        return NULL;
    }
    
    verbose("Finished read line  [%s]\n", str);
    
    return str;
}

arg_t* parse_tokens(char *input)
{
    verbose("Starting token parsing\n");
    
    arg_t *result = NULL;
    arg_t *last = result;
    char* arg = strtok(input, " ");
    if (arg)
    {
        result = (arg_t *)malloc(sizeof(arg_t));
        if (result == NULL) exit(-1);
        last = result;
    }
    else
    {
        return NULL;
    }
    while (arg)
    {
        verbose("  Tokenizing arg: [%s]\n", arg);
        last->arg = arg;
        arg = strtok(NULL, " ");
        if (arg == NULL) break;
        
        last->next = (arg_t *)malloc(sizeof(arg_t));
        if (last->next == NULL) exit(-1);
        last = last->next;
    }
    
    last->next = NULL;
    
    verbose("Finished token parsing\n");
    
    return result;
}

int main(int argc, char **argv)
{
    if (argc > 1) 
    {
        ISVERBOSE = strcmp(argv[1], "-v") == 0;
    }
    verbose("\n\nVERBOSE MODE! GET EXCITED!\n\n");
    
    while (true) 
    {
        printf("osh> ");
        
        char *line = readline();     
        arg_t *tokenChain = parse_tokens(line);
        
        // build commands up by simply dumping ALL args in them
        // also determine next command conditions
        verbose("Starting token analyzing");
        verbose("Starting command chain construction\n");
        verbose("  Creating first command\n");
        command_t *firstCommand = (command_t *)malloc(sizeof(command_t));
        command_t *command = firstCommand;
        arg_t *currentToken = tokenChain;
        arg_t *lastToken = NULL;
        while (currentToken != NULL) 
        {
            if (is_terminating_operator(currentToken->arg))
            {
                verbose("  Found terminating token [%s]\n", currentToken->arg);
                
                // break off chain
                command->last_arg = lastToken;
                command->last_arg->next = NULL;
                lastToken = NULL;
                
                // determine how it ended
                bool endedWithPipe = false;
                if (strcmp(currentToken->arg, ON_SUCCESS))
                {
                    command->next_command_exec_on = NEXT_ON_SUCCESS;
                }
                else if (strcmp(currentToken->arg, ON_FAIL))
                {
                    command->next_command_exec_on = NEXT_ON_FAIL;   
                }
                else if (strcmp(currentToken->arg, ON_ANY))
                {
                    command->next_command_exec_on = NEXT_ON_ANY;                       
                }
                else if (strcmp(currentToken->arg, PIPE))
                {
                    command->output_mode = O_PIPE;
                    endedWithPipe = true;
                }  

                verbose("  Creating new command\n");
                command = (command_t *)malloc(sizeof(command_t));
                if (endedWithPipe)
                {
                    command->input_mode = I_PIPE;
                }

                currentToken = currentToken->next;
                continue;
            }
            
            
            if (command->arg_list == NULL)
            {
                verbose("  Starting arg list with token [%s]\n", currentToken->arg);
                command->arg_list = currentToken;
            }
            else
            {
                verbose("  Uninteresting token passed over [%s]\n", currentToken->arg);
            }
            
            lastToken = currentToken;
            currentToken = currentToken->next;   
        }
        verbose("Finished analyzing tokens\n");
        verbose("Finished constructing command chain\n");
    }
}



