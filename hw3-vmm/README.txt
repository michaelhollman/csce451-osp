CSCE 451
Programming Assignment 3

Michael Hollman
cse:  mhollman
nuid: 86987337

--------------------------------------------------------------------------------

--- COMPILING ---

The solution to this problem includes a makefile. Simply execute `make` to
compile a.c into an executable file called "a.out".

--- RUNNING ---

Running the program can be accomplished exactly how the assignment specified.

The program requires 2 arguments:
  - the filename of the binary backing store
  - the filename of the input address list
For example:
  ./a.out BACKING_STORE.bin addresses.txt
will run the program using BACKING_STORE.bin as the backing store and 
addresses.txt as the list of addresses.

--- TESTING ---

The program will print information about each input address, as well as final
statistics after all addresses, to stdout. It abides by the exact same format
as the correct.txt file supplied with the assignment. The output can easily be
piped to a text file or directly into a diff or comparison tool to check the
output against the correct or expected output.

In my testing, executing the program consistently produced identical output to
the provided test program and correct.txt.

--- NOTES, QUIRKS, THOUGHTS, & LEARNINGS ---

- While implementing the solution, I opted to keep things very simple and leave
  all logic within the main method. While this did yield a slightly bloated main
  method, it ended up being much cleaner and easier to work with than an
  approach that would have abstracted out logic to helper methods. I ended up
  having only one or two small bits of duplicated code for some handling of
  corner case errors, which I was fine with.
  
- For my TLB implementation, I decided to use a FIFO replacement strategy (over 
  something like a LRU strategy), simply because the FIFO approach was extremely
  easy to implement and test. My guess is that the sample executable provided
  also used a FIFO strategy, as I ended up with identical TLB statistics. Using
  an LRU strategy may have lead to a higher TLB hit rate, but I decided that it
  would not be worth the extra effort and risk of bugs to change approaches. 

- Due to the data structure that I used to represent the TLB (an int32_t 2-data
  array, behaving similarly to a dictionary from higher-level languages), TLB
  lookups had to be done linearly. In the case of this assignment, where the TLB
  was hard-coded to be very small (16 entries), this approach doesn't introduce
  exorbitant amounts of overhead. If the TLB were larger, or the assignment
  required further optimization, it would be advantageous to structure the TLB
  in a way that allowed for instantaneous (or at least better-than-linear) 
  lookup complexity.
  
 - We did not have to complete the modification portion of the assignment (which
   consisted of adjusting the program to use less physical memory than the size
   of the virtual address space). Within my code, I make a comment where my code
   would need to handle the case where there were no empty frames to place a
   freshly-loaded page into.