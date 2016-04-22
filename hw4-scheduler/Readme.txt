CSCE 451
Programming Assignment 4

Michael Hollman
cse:  mhollman
nuid: 86987337

--------------------------------------------------------------------------------

--- COMPILING ---

The assignment included a provied makefile. It has not been modified. Simply
run make to build the program.

--- RUNNING ---

The parameter and procedures for running the program have not been modified
from the original assignment.

--- ISSUES ---

While developing this solution locally, I ran into very few issues. Development
was extremely smooth and straghtforward. However, when I got to the point of
running and testing my code on the CSE server, I immediately encountered
a plethora of issues that stemmed from what seem like bugs in the provided code.

I encountered many errors from memory being double-freed or from the user block
quota limit being reached. I was unable to adequately research why these issues
were occurring, which was incredibly discouraging. As I mentioned, local testing
worked very well, and while there were some minor discrepencies between the
output I was getting locally and the output from the test program, the behaviors
still seemed to be correct.

So, ultimately, I cannot guarantee that this will work on the CSE server. I
believe my implemenation is correct, except for some system-specific quirks that
seem to manifest themselves in the provided code.

--- LEARNINGS ---

I found this assignment initially challenging. Due to the issues I ran into that
I discussed in the prior setion, I found it incredibly difficult to draw
positively from this assignment. I did learn a lot about how POSIX timing and
signaling within C between threads works.

--- QUESTIONS ---

1. This configuration of signal blocking ensures that the correct threads are 
   acting on the correct signals. SIGUSR2 and SIGALRM are blocked in the worker 
   to prevent the worker from acting on those signals; similarly SIGUSR2 and 
   SIGTERM are unblocked in the scheduler so that it can manage them.
   
2. sigaction() (re-)configures a thread's response to a signal, whereas sigwait()
   suspends the thread until a specified signal.
   
3. The alternative would be to use an absolute clock, in that it would report its
   time relative to the real-world, physical time, instead of relative to a given
   or configured "starting time" for the relative timer.
   
4. When the scheduler kills the thread via SIGTERM.

5. The scheduler finishes when all its worker threads have died.

6. The scheduled thread remains in the queue until it is actually run. It gets
   moved to the back of the queue when the thread is suspended. It is removed from
   the queue when it has fully finished and is killed.
  
7. The completed variable acts as a counter of completed threads. It is an
   essential state variable for keeping track of if there are still threads to be
   scheduled and run, or if the entire process is in a state that it can finish.
   
8. You could easily modify suspend_worker to not reposition threads upon suspension.

9. A new set of signals could be configured to allow for the scheduler and worker
   to communicate and allow for arbitrary starting and stopping between quanta. This
   could be useful in a system that perhaps needed to maintain different priorities
   of processes, perhaps determined by their user/owner.