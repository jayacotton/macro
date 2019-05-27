# macro
macro pre processor
First attempt at geting a macro32 like preprocessor running.

2/11/18 - The code is starting to get solid.  It will compile most of the test code
there may be some broken casses in the test code.....

I have tested this code using 3 compilers on solaris - clang, cc, and gcc.  So far 
gcc is giving the best results.  With clang and cc I am getting a memory fault in the
ExpandText code.  Need to chase that down real soon now.

This code is also known to work on cygwin with gcc.  
