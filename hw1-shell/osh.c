#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "command.h"
#include "operators.h"

#define COLOR_VERBOSE "\x1b[31m"
#define COLOR_RESET "\033[0m"
#define verbose(...) if(IS_VERBOSE) { fprintf(stderr, COLOR_VERBOSE __VA_ARGS__); fprintf(stderr, COLOR_RESET); fflush(stderr); }
static bool IS_VERBOSE = false;

const int LOOP_PROC_LIMIT = 1000;
static int LOOP_COUNT = 0;
static int PROC_COUNT = 0;

const int READ = 0;
const int WRITE = 1;

// convenience for checking sanity checks on main program loops
bool looping_okay()
{
    return LOOP_COUNT < LOOP_PROC_LIMIT && PROC_COUNT < LOOP_PROC_LIMIT;
}

// read a line of input from stdin
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
        verbose("Error reading string\n");
        exit(-1);
        return NULL;
    }
    return str;
}

// equivalent to a string.split from higher languages
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

// parse all the tokens into a chain of commands
command_t *create_command_chain(arg_t *tokenChain)
{
        // build commands up by simply dumping ALL args in them
        // also determine next command conditions
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
                lastToken = NULL;
                currentToken = currentToken->next;
                command->parse_state = NEED_ANY_TOKEN;
                continue;
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
                if (strcmp(currentToken->arg, OP_ON_SUCCESS) == 0)
                {
                    command->next_command_exec_on = NEXT_ON_SUCCESS;
                }
                else if (strcmp(currentToken->arg, OP_ON_FAIL) == 0)
                {
                    command->next_command_exec_on = NEXT_ON_FAIL;   
                }
                else if (strcmp(currentToken->arg, OP_ON_ANY) == 0)
                {
                    command->next_command_exec_on = NEXT_ON_ANY;                       
                }
                else if (strcmp(currentToken->arg, OP_PIPE) == 0)
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
                
                if (strcmp(currentToken->arg, OP_OUTPUT) == 0)
                {
                    command->output_mode = O_WRITE;
                    command->parse_state = NEED_OUT_PATH;
                }
                else if (strcmp(currentToken->arg, OP_APPEND) == 0)
                {
                    command->output_mode = O_APPND;
                    command->parse_state = NEED_OUT_PATH;
                }
                else if (strcmp(currentToken->arg, OP_INPUT) == 0)
                {
                    command->input_mode = I_FILE;
                    command->parse_state = NEED_IN_PATH;
                }
                
                // break off chain
                if (command->last_arg == NULL && lastToken != NULL)
                {
                    command->last_arg = lastToken;
                    command->last_arg->next = NULL;
                    lastToken = NULL; 
                }
                
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
        verbose("Finished constructing command chain\n");
        
        // at this point, all the args have been dumped into commands and the 
        // critical relationships between commands have been sorted out.
        return firstCommand;
}

// adopted from assignment, converts list of args into an array of args
char **generate_argv(arg_t *args)
{
    char **argv;
    int argc = 0;
    arg_t *arg = args;
    while (arg != NULL) 
    {
        argc++;
        arg = arg->next;
    }

    argv = (char **)(malloc(sizeof(char *) * argc + 1 ));

    arg = args;
    for (int i = 0; i < argc && arg != NULL; i++)
    {
        argv[i] = arg->arg;
        arg = arg->next;
    }

    argv[argc] = NULL;
    return argv;
}

// execute the commands and manage child processes for it
void run_commands(command_t *commandChain)
{
    command_t *command = commandChain;
    
    int pipes[2] = {-1, -1};
    
    // while there are still commands to run
    while (command != NULL && looping_okay()) 
    {
        arg_t *args = command->arg_list;
        // handle an empty arg command
        if (args == NULL)
        {
            verbose("Skipping empty command\n");
            command = command->next;
            continue;
        }
        
        verbose("Starting command execution [%s...]\n", args->arg); 
        
        // handle the special "exit" command
        if (strcmp(args->arg, "exit") == 0)
        {
            printf("\ngoodbye!\n");
            exit(0);
        }
        
        if (command->output_mode == O_PIPE)
        {
            // open the pipes
            verbose("Opening pipes\n");
            pipe(pipes);
            command->output_fd = pipes[WRITE];
            command->next->input_fd = pipes[READ];
        }
        
        verbose("Forking! (you should see this once)\n");
        PROC_COUNT++;
        pid_t childPid = fork();
        verbose("Forked! (you should see this twice)\n");
        
        if (childPid < 0)
        {
            fprintf(stderr, "Fork Failed \n");
            exit(1);
        }
        else if (childPid == 0 )
        {
            // Child process
                    
            // open files if needed, duplicate to stdin/out
            if (command->input_mode == I_FILE)
            {
                verbose("Opening input file [%s]\n", command->input_file);
                command->input_fd = open(command->input_file, O_RDONLY);
                if (command->input_fd < 0)
                {
                    fprintf(stderr, "Error opening input file [%s]\n", command->input_file);
                    exit(1);
                }
                
                dup2(command->input_fd, 0);
                close(command->input_fd);
            }
           
            if (command->output_mode == O_WRITE)
            {
                verbose("Opening output file (re-write) [%s]\n", command->input_file);
                command->output_fd = open(command->output_file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            }
            else if (command->output_mode == O_APPND)
            {
                verbose("Opening output file (append) [%s]\n", command->input_file);
                command->output_fd = open(command->output_file, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
            }
            if (command->output_mode == O_WRITE || command->output_mode == O_APPND)
            {
                if (command->output_fd < 0)
                {
                    fprintf(stderr, "Error opening output file [%s]\n", command->output_file);
                    exit(1);
                }
                
                dup2(command->output_fd, 1);
                close(command->output_fd);
            }
            
            // set up pipes if needed
            if (command->output_mode == O_PIPE)
            {
                verbose("Dup-ing output pipe\n");
                dup2(command->output_fd, 1);    
            }
            if (command->input_mode == I_PIPE)
            {
                verbose("Dup-ing input pipe\n");
                dup2(command->input_fd, 0);
            }
            if (command->output_mode == O_PIPE || command->input_mode == I_PIPE)
            {
                verbose("Closing pipes\n");
                close(pipes[WRITE]);
                close(pipes[READ]);
                pipes[WRITE] = -1;
                pipes[READ] = -1;
            }
 
            // set up argvs    
            char **argv = generate_argv(args);
            
            // exec!
            execvp(*argv, argv);
            
            fprintf(stderr, "Exec Failed \n");
            exit(1);
        }
        else 
        {
            // Parent process
            
            if (pipes[WRITE] > 0)
            {
                verbose("Closing stray open WRITE pipe in parent process\n");
                close(pipes[WRITE]);
                pipes[WRITE] = -1;
            }
            
            int status;
            if (command->output_mode != O_PIPE)
            {
                while (PROC_COUNT > 0)
                {
                    wait(&status);
                    PROC_COUNT--;
                    verbose("Child process complete\n");
                }
                
                int exitStatus = WEXITSTATUS(status);
                
                if ((command->next_command_exec_on == NEXT_ON_SUCCESS && exitStatus != 0) ||
                    (command->next_command_exec_on == NEXT_ON_FAIL && exitStatus == 0))
                {
                    command = NULL;
                    verbose("Next command exec condition failed\n");
                    break;
                }
            }
        }
        
        command = command->next;
        LOOP_COUNT++;
    }
    
    verbose("Finished execution of all commands\n");
}

int main(int argc, char **argv)
{
    if (argc > 1) 
    {
        IS_VERBOSE = strcmp(argv[1], "-v") == 0;
    }
    verbose("\n\nVERBOSE MODE! GET EXCITED!\n\n");
    
    // This is a safeguard for errant looping or process creation.
    while (looping_okay()) 
    {
        printf("osh> ");
        
        char *line = readline();     
        arg_t *tokenChain = parse_tokens(line);
        command_t *command = create_command_chain(tokenChain);
        run_commands(command);
        

    }
    
    // This is if we reach the loop or thread limits. sad time.
    fprintf(stderr, "Thread or loop limit reached. Congrats.\n");
    return 1;
}



