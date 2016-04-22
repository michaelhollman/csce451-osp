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

While developing this solution, I initially feared that I had encountered
some very obscure system-specific bug in the provided code. Fortunately, I was
able to determine that I had simply made a typographical error within 
setup_sig_handlers. After much panic, I am confident in my implementation, and
it passes all the provided tests.

--- LEARNINGS ---

I found this assignment initially challenging, mostly due to the issues I
mentioned previously that ended up just being a stupid mistake. I did end up 
learning a lot about how POSIX timing and signaling within C between threads 
works. While I can't say I necessarily enjoyed working with these APIs and
libraries, it was at least very interesting.

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