// Adapted from the provided comm.h

#ifndef	_comm_h_
#define _comm_h_

/*
 * Input Modes
 */
#define I_FILE  1
#define I_PIPE  2

/*
 * Output Modes
 */
#define O_WRITE        1
#define O_APPEND       2
#define O_PIPE         3 

/*
 * Run Next Command Modes
 */
#define NEXT_ON_ANY        1
#define NEXT_ON_SUCCESS    2
#define NEXT_ON_FAIL       3 


/*
 * Parse State 
 */
#define ERROR_STATE -1
#define NEED_ANY_TOKEN 0
#define NEED_NEW_COMMAND 1
#define NEED_IN_PATH 2
#define NEED_OUT_PATH 3
#define NEED_END 4

extern int cerror;


/* A C sytle linked list to parse and build the argv structure */
typedef struct arg_t {
        char *arg;
        struct arg_t *next;
} arg_t;


/* A C sytle linked list to parse the input line */
typedef struct command_t {
        int parse_state;

        struct arg_t *arg_list;
        struct arg_t *last_arg;

        char *input_file;
        int input_mode;
        int input_fd;

        char *output_file;
        int output_mode;
        int output_fd;

        int next_command_exec_on;
        pid_t pid;
        struct command_t *next;
} command_t;

#endif