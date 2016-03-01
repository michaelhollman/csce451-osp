#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "command.h"
#include "operators.h"

#define verbose(...) if(ISVERBOSE) printf(__VA_ARGS__); fflush(stdout)
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

command_t *create_command_chain(arg_t *tokenChain)
{
        // build commands up by simply dumping ALL args in them
        // also determine next command conditions
        verbose("Starting token analyzing\n");
        verbose("Starting command chain construction\n");
        verbose("  Creating first command\n");
        command_t *firstCommand = (command_t *)malloc(sizeof(command_t));
        firstCommand->parse_state = 0;
        command_t *command = firstCommand;
        arg_t *currentToken = tokenChain;
        arg_t *lastToken = NULL;
        while (currentToken != NULL) 
        {   
            if (command->parse_state == ERROR_STATE)
            {
                verbose("    error state\n");
                firstCommand = command;
                firstCommand->parse_state = ERROR_STATE;
                return firstCommand;   
            }
            
            if (command->parse_state == NEED_IN_PATH)
            {
                verbose("    Setting token as input file [%s]\n", currentToken->arg);
                command->input_file = currentToken->arg;
            }
            else if (command->parse_state == NEED_OUT_PATH)
            {
                verbose("    Setting token as output file [%s]\n", currentToken->arg);
                command->output_file = currentToken->arg;

            }
            if ((command->parse_state == NEED_IN_PATH) || (command->parse_state == NEED_OUT_PATH))
            {
                command->parse_state = NEED_END;
                lastToken = NULL;
                currentToken = currentToken->next;
                continue;
            }
            
            if (command->parse_state == NEED_END && !is_terminating_operator(currentToken->arg))
            {
                verbose("AAACK! An extra token. I have no idea what to do! I'm probably going to die now.\n");
            }
             
            if (is_terminating_operator(currentToken->arg))
            {
                verbose("    Found terminating operator [%s]\n", currentToken->arg);
                
                // break off chain if it hasn't been already
                if (command->last_arg == NULL && lastToken != NULL)
                {
                    command->last_arg = lastToken;
                    command->last_arg->next = NULL;
                    lastToken = NULL;
                }
                
                // determine how it ended
                bool endedWithPipe = false;
                if (strcmp(currentToken->arg, ON_SUCCESS) == 0)
                {
                    command->next_command_exec_on = NEXT_ON_SUCCESS;
                }
                else if (strcmp(currentToken->arg, ON_FAIL) == 0)
                {
                    command->next_command_exec_on = NEXT_ON_FAIL;   
                }
                else if (strcmp(currentToken->arg, ON_ANY) == 0)
                {
                    command->next_command_exec_on = NEXT_ON_ANY;                       
                }
                else if (strcmp(currentToken->arg, PIPE) == 0)
                {
                    command->output_mode = O_PIPE;
                    endedWithPipe = true;
                }

                verbose("  Creating new command\n");
                command_t * newCommand = (command_t *)malloc(sizeof(command_t));
                if (endedWithPipe)
                {
                    command->output_mode = O_PIPE;
                    newCommand->input_mode = I_PIPE;
                }
                command->next = newCommand;
                command->parse_state = NEED_ANY_TOKEN;
                command = newCommand;

                currentToken = currentToken->next;
                continue;
            }
            else if (is_file_operator(currentToken->arg))
            {
                verbose("    Found file operator [%s]\n", currentToken->arg);
                
                if (strcmp(currentToken->arg, OUTPUT) == 0)
                {
                    command->output_mode = O_WRITE;
                    command->parse_state = NEED_OUT_PATH;
                }
                else if (strcmp(currentToken->arg, APPEND) == 0)
                {
                    command->output_mode = O_APPEND;
                    command->parse_state = NEED_OUT_PATH;
                }
                else if (strcmp(currentToken->arg, INPUT) == 0)
                {
                    command->output_mode = I_FILE;
                    command->parse_state = NEED_IN_PATH;
                }
                
                // break off chain
                command->last_arg = lastToken;
                command->last_arg->next = NULL;
                lastToken = NULL;
                
                currentToken = currentToken->next;
                continue;
            }
            
            if (command->arg_list == NULL)
            {
                verbose("    Starting arg list with token [%s]\n", currentToken->arg);
                command->arg_list = currentToken;
            }
            else
            {
                verbose("    Uninteresting token passed over [%s]\n", currentToken->arg);
            }
            
            lastToken = currentToken;
            currentToken = currentToken->next;   
        }
        verbose("Finished analyzing tokens\n");
        verbose("Finished constructing command chain\n");
        
        // at this point, all the args have been dumped into commands and the 
        // critical relationships between commands has been sorted out.
        return firstCommand;
}

int main(int argc, char **argv)
{
    if (argc > 1) 
    {
        ISVERBOSE = strcmp(argv[1], "-v") == 0;
    }
    verbose("\n\nVERBOSE MODE! GET EXCITED!\n\n");
    
    const limit = 1000;
    int loopCount = 0;
    int threadCount = 0;
    
    // This is a safeguard for errant looping or threads.
    while (loopCount < limit && threadCount < 1000) 
    {
        printf("osh> ");
        
        char *line = readline();     
        arg_t *tokenChain = parse_tokens(line);
        command_t *commandChain = create_command_chain(tokenChain);
    }
    
    // This is if we reach the loop or thread limits. sad time.
    printf("Thread or loop limit reached. Congrats.\n");
    return 1;
}



