# CSCE 451 - Programming assignment 1

Michael Hollman
cse username: mhollman
nuid:         86987337

## Running instructions

Compile the program by running `make`. This will produce an executable binary `osh`.

A first argument of `-v` can be given to `osh` to start it in verbose mode. Note that verbose mode is _very, very verbose_. Any other or additional arguments will be completely ignored.

## Problems faced

The two most challenging aspects of implementing this program were
- getting back into writing c.
- wrestling what essentially became a contrived state machine.

I knew that once I had the command structures correctly filled out, executing the commands would be relatively straightforward, as there are many examples of how to handle the process creation, waiting, trapping, etc. in our textbook, the class materials, numerous c library documentation websites, and scholarly examples online. That is the primary reason why there are so many `verbose` debugging statements in the parsing section of my program, but very few in the the execution portion.

## What I learned

I learned…
- I still have a bit of a nack for developing in c.
- I still odly enjoy developing in c.
- If I want to do any more development in c in the near future, I really need to get a better environment and/or IDE set up for it.
- Once I wrapped my head around how to actually use process management calls like `fork()` and `wait()`, it made a lot of sense.
- Using c's process management APIs feels like working with blunt rocks compared to the convenient threading/process APIs in languages like C# and Ojbective-C.
- Piping is still largely a mystery to me. I understand it perfectly conceptually, but I'm mildly mystified at how the code I wrote acomplished what it did and have very little confidence that it's actually accomplishing what it should _correctly_.