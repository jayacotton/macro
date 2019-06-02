# macro
macro pre processor
First attempt at geting a macro32 like preprocessor running.

2/11/18 - The code is starting to get solid.  It will compile most of the test code
there may be some broken casses in the test code.....

I have tested this code using 3 compilers on solaris - clang, cc, and gcc.  So far 
gcc is giving the best results.  With clang and cc I am getting a memory fault in the
ExpandText code.  Need to chase that down real soon now.

This code is also known to work on cygwin with gcc, ubuntu, and solaris.

TO BUILD:

git the code to your machine.

cd to the macro directory
touch NEWS
touch README
touch AUTHORS
sh ./autogen.sh
./configure
make
make install

After these steps you can make -f Makefile.z80 ; make -f Makefile.z80 install
This will create a macro.com file for cp/m.  This allows you to run macro on 
cp/m as well as on linux, solaris, windows/cygwin etc.



