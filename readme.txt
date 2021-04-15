Testing Strategy:

Tesing correctness of compare.c
--------------------------------
There are three phases to the program:
1) Using the main thread to analyze options and send files/directories from the arguments to the file and directory queues respectively. This phase also initializes relevant global variables.
2) The Collection Phase where -f and -d number of threads are created to run the file and directory thread routines respectively.
3) The Analysis Phase where -a number of of threads are created to run the analysis thread routine. 

Note: By compiling with the -DDEBUG flag, our program outputs various information which can be used to verify the correctness of a particular section of code. More on this in the collection and analysis test sections.

We first tested the Optional Flag Aggregation tests, followed by the Collection Phase tests, and lastly the Analysis Phase tests.

================================


Optional Flag Aggregation tests:
-----------------------------------
Strategy: Before collection phase begins, all optional arguments are checked for correctness. These tests ensured that all default values of optional flags were overwritten with their last occurence in set of arguments (assuming correctness). We confirmed those results by printing the values of the #threads and the string suffix & its length and compared that to the input arguments.
    
Single digit flags: ./compare -d5 -f9 -a2

Multiple digit flags: ./compare -d10238 -f23948 -a35251

No flags: ./compare file1

Invalid flags: ./compare -d5 -f9 -a

Invalid flags: ./compare -df

Invalid flags: ./compare -df -f92c -s

Invalid flags: ./compare -x -a -b -c -d -z

Invalid flags: ./compare - file1 fil2

Overwritten flags: ./compare -d68 -d23 file1 dir1 dir2 -d1 -f3 lastfile -a34 -f5 -a3 lastdir

Suffix flag, optional order:  ./compare -s.nottxt -d10238 -f23948 -a35251 -sTEXT

Empty Suffix flag: ./compare -s file1 file2

Overwritten Suffix flags: ./compare -sTEXT -sTXT -sEXE -sRAR file2 dir2 -f234 -d89 -sZZZ

=====================================

Collection Phase tests:
----------------------
Implementations of the file_thread_routine and dir_thread_routine can be found in collection_threads.c.
The main thread creates file threads to execute the file_thread_routine and directory threads to execute the dir_thread_routine.

We confirm that -fN number of file threads are created by printing the threadID of any thread which enters the file_thread_routine.
We confirm that -dN number of dir threads are created by printing the threadID of any thread which enters the dir_thread_routine.

These prints happen when we compile our executable having defined the DEBUG macro -> 'make dcompare' compiles all .o files and links to create a separate program called dcompare which is used for debugging purposes.

When dcompare is executed with appropriate arguments, we can observe the number of different TIDs that are printed from, say the file_thread_routine, and this alerts us to how many file threads were created using pthread_create to run the file_thread_routine from the main method. The same happens for the directory threads.
Using this debug macro and the new executable dcompare, we validate that our project uses concurrency appropriately. 

Once all threads are terminated and get reaped by the main thread, the main thread resumes with the last part of the collection phase where we ensure that more than two files have been added to the WFD repo. If there are less than two files, then we print an appropriate message and return exit failure.

We complete testing the collection phase using the debug flag turned on to run the debug_wf_repo_print method in debugger.c printing the contents of our populated wf_repo *wf_stack (WFD repo).
Recall that the contents that should be in our WFD repo by the end of the collection phase are all the files with their appropriate words/freqs/counts.
We manually compare the printed contents against a python helper script called word_freq_counter.py for any given file. This is in conjunction to manually computing frequencies by hand for smaller test cases. This ensured that the collection phase worked, no matter the amount of threads created.

We then move onto the analysis phase.
===============================================

Analysis Phase tests:
---------------------
Implementation of the analysis_thread_routine can be found in analysis_threads.c 
The main phase creates the analysis threads to execute the analysis_thread_routine.

Similar to previous phase, we test that only (-aN) N number of threads enter the analysis_thread_routine during thread creation in the main method for the analysis threads. This is checked, again, by compiling our executable using the debug macro. See lines 12-14 in analysis_threads.c for more information. This validates that our project uses concurrency in the analysis phase when attempting to find the JSD between pairs of files in the WFD repo.

Note: in the event that the number of jsd pairs M is less than N, then we only create M analysis threads.

After all analysis threads are properly reaped, the results would be have been printed to standard output.

We compare these results printed to standard output, using the jsd_check.py python script in python_helper. This script takes in two files and ouputs the JSD of them. We go through printed pairs in standard output (the results of our compare executable), and verify that the JSD of those pairs when passed to the jsd_check.py script is equivalent.
===================================================
