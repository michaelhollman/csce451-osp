CSCE 451
Programming Assignment 2
Problem 1

Michael Hollman
cse:  mhollman
nuid: 86987337

--------------------------------------------------------------------------------

--- BUILDING ---

The solution to this problem includes a makefile. Simply execute `make` to
compile pro_con.c into an executable file called "pro_con".



--- RUNNING & TESTING ---

To run the program, simply run the pro_con executable.

For every character inserted into the buffer by a producer and every character
read from the buffer by a consumer, a message will be printed to stdout.
The message will follow the format:
    "[Producer|Consumer] (ID): 'CHAR' [->|<-] [IDX]"
        - Producer|Consumer - if the action was from a producer or a consumer
        - ID                  - a pseudo-id of the producer or consumer thread
        - CHAR                - the character inserted or removed
        - ->|<-               - the direction of dataflow (in or out of buffer)
        - IDX                 - the index in the buffer

The program will continue running until it is forcibly interrupted, e.g. SIGINT
(ctrl+c) or SIGTSTP (ctrl+z) is sent to the process.

There are 2 arguments that can be provided to pro_con to assist in understanding
what is happening in the code:

    - First, an integer may be given to limit the total number of insertions
      into the buffer. The program will immediately terminate once that limit is
      reached.
        Ex: "./pro_con 1000" will only let 1000 insertions occur.
    - Second: a "-v" flag can be provided to put the program into verbose mode
      which will print messages giving a *very* detailed description of the
      program flow
        Ex: "./pro_con -v"
    - These two can be combined, but the integer must be provided first.
        Ex: "./pro_con 1000 -v"
   
        
        
--- NOTES, THOUGHTS, & LEARNINGS ---

- I spent most of my development efforts for this assignment in an OS X 
  environment, which does not have full support for POSIX semaphores. This is
  why I used sem_open instead of sem_init and had to account for SEM_VALUE_MAX
  being much smaller than our buffer size of 10 million.
- The original assignment didn't give much specification on how the buffer
  should behave. I opted to use a cyclical, array-based buffer, which allows for
  the program to run infinitely.