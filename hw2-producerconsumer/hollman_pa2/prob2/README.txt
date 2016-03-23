CSCE 451
Programming Assignment 2
Problem 2

Michael Hollman
cse:  mhollman
nuid: 86987337

--------------------------------------------------------------------------------

--- BUILDING ---

The solution to this problem includes a makefile. Simply execute `make` to
compile pro_con.c and monitor.c/.h into an executable file called "pro_con".



--- RUNNING & TESTING ---

To run the program, simply run the pro_con executable.

The monitor will output insertions and removals into the buffer. To validate that
it is working correctly, you can compare the values between their insertion and
removal.

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

- I used a signal & wait approach to my monitor implemenation.