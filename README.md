# Systems Programming Project 2: Plagiarism Checker
***

Michael Zhang&emsp;NetID: mz487

Kev Sharma&emsp;&emsp;&nbsp;NetID: kks107

***

## Description:
<p style="font-weight:500;">A program which checks for similarities between any pair of files, computing their similarity using the Jensen–Shannon divergence. The lower the Jensen-Shannon divergence is for a file pair, the more similar the pair are to each other. The higher, the lower the similarity.</p>

## Usage:
- To compile without defining DEBUG and to compile with DEBUG defined, respectively
    ```sh
    $ make
    ```
    ```sh
    $ make dcompare
    ```
- Running the program
    ```sh
    $ ./compare -dN -fN -aN -sS "list of directories..." "list of files..."
    ```
    or
    ```sh
    $ ./dcompare -dN -fN -aN -sS "list of directories..." "list of files..."
    ```
where ```-dN```, ```-fN```, and ```-aN``` specifies the number of directory threads the program should create respectively, and where ```-sS``` specifies the suffix of the files that files in the directories must end in to be valid for examination.  
Note: if ```-dN```, ```-fN```, or ```-aN``` are not specified, their respective number of threads will default to 1. As for ```-sS```, if that flag is not specified, the default suffix will be .txt

***

## Testing:
### Tesing correctness of compare.c

There are three phases to the program:  
&emsp;&emsp;1) Using the main thread to analyze options and send files/directories from the arguments to the file and directory queues respectively. This phase also initializes relevant global variables.  
&emsp;&emsp;2) The Collection Phase where ```-fN``` and ```-dN``` number of threads are created to run the file and directory thread routines respectively.  
&emsp;&emsp;3) The Analysis Phase where ```-aN``` number of of threads are created to run the analysis thread routine. 

Note: By compiling with the ```-DDEBUG``` flag, our program outputs various information which can be used to verify the correctness of a particular section of code. More on this in the collection and analysis test sections.

We first tested the Optional Flag Aggregation tests, followed by the Collection Phase tests, and lastly the Analysis Phase tests.


### Optional Flag Aggregation tests:

Strategy: Before collection phase begins, all optional arguments are checked for correctness. These tests ensured that all default values of optional flags were overwritten with their last occurence in set of arguments (assuming correctness). We confirmed those results by printing the values of the #threads and the string suffix & its length and compared that to the input arguments.
    
&emsp;&emsp;Single digit flags: ```$ ./compare -d5 -f9 -a2```

&emsp;&emsp;Multiple digit flags: ```$ ./compare -d10238 -f23948 -a35251```

&emsp;&emsp;No flags: ```$ ./compare file1```

&emsp;&emsp;Invalid flags: ```$ ./compare -d5 -f9 -a```

&emsp;&emsp;Invalid flags: ```$ ./compare -df```

&emsp;&emsp;Invalid flags: ```$ ./compare -df -f92c -s```

&emsp;&emsp;Invalid flags: ```$ ./compare -x -a -b -c -d -z```

&emsp;&emsp;Invalid flags: ```$ ./compare - file1 fil2```

&emsp;&emsp;Overwritten flags: ```$ ./compare -d68 -d23 file1 dir1 dir2 -d1 -f3 lastfile -a34 -f5 -a3 lastdir```

&emsp;&emsp;Suffix flag, optional order:  ```$ ./compare -s.nottxt -d10238 -f23948 -a35251 -sTEXT```

&emsp;&emsp;Empty Suffix flag: ```$ ./compare -s file1 file2```

&emsp;&emsp;Overwritten Suffix flags: ```$ ./compare -sTEXT -sTXT -sEXE -sRAR file2 dir2 -f234 -d89 -sZZZ```



### Collection Phase tests:

- Implementations of the ```file_thread_routine``` and ```dir_thread_routine``` can be found in ```collection_threads.c```.  

- The main thread creates file threads to execute the ```file_thread_routine``` and directory threads to execute the ```dir_thread_routine```.  

- We confirm that ```-fN``` number of file threads are created by printing the threadID of any thread which enters the ```file_thread_routine```.  

- We confirm that ```-dN``` number of dir threads are created by printing the threadID of any thread which enters the ```dir_thread_routine```.  

- These prints happen when we compile our executable having defined the ```DEBUG``` macro -> ```$ make dcompare``` compiles all .o files and links to create a separate program called dcompare which is used for debugging purposes.  

- When ```dcompare``` is executed with appropriate arguments, we can observe the number of different TIDs that are printed from, say the ```file_thread_routine```, and this alerts us to how many file threads were created using ```pthread_create``` to run the ```file_thread_routine``` from the main thread. The same happens for the directory threads.  

- Using this debug macro and the new executable ```dcompare```, we validate that our project uses concurrency appropriately.  

- Once all threads are terminated and get reaped by the main thread, the main thread resumes with the last part of the collection phase where we ensure that more than two files have been added to the WFD repo. If there are less than two files, then we print an appropriate message and return ```EXIT_FAILURE```.  

- We complete testing the collection phase using the debug flag turned on to run the ```debug_wf_repo_print``` method in debugger.c printing the contents of our populated ```wf_repo *wf_stack``` (WFD repo).  

- Recall that the contents that should be in our WFD repo by the end of the collection phase are all the files with their appropriate words/freqs/counts.  

- We manually compare the printed contents against a python helper script called ```word_freq_counter.py``` for any given file. This is in conjunction to manually computing frequencies by hand for smaller test cases. This ensured that the collection phase worked, no matter the amount of threads created.  

We then move onto the analysis phase.

### Analysis Phase tests:

- Implementation of the ```analysis_thread_routine``` can be found in ```analysis_threads.c``` 
The main thread creates the analysis threads to execute the ```analysis_thread_routine```.

- Similar to previous phase, we test that only ```-aN``` N number of threads enter the analysis_thread_routine during thread creation in the main method for the analysis threads. This is checked, again, by compiling our executable using the debug macro. See lines 12-14 in ```analysis_threads.c``` for more information. This validates that our project uses concurrency in the analysis phase when attempting to find the JSD between pairs of files in the WFD repo.  
&emsp;&emsp;Note: in the event that the number of jsd pairs M is less than N, then we only create M analysis threads.

- After all analysis threads are properly reaped, the results would be have been printed to ```stdout```.

- We compare these results printed to ```stdout```, using the ```jsd_check.py``` python script in python_helper. This script takes in two files and ouputs the JSD of them. We go through printed pairs in ```stdout``` (the results of our compare executable), and verify that the JSD of those pairs when passed to the ```jsd_check.py``` script is equivalent.

## Using Python Helper:
- ```word_freq_counter.py``` takes a file as an argument and produces ```file_freq.csv``` as an output. Two such csv's can then be inputted to ```jsd_check.py``` which will then print out the jsd results alongside some other information, such as kld. These can be checked against pairs in the ```./compare``` jsd outputs. Keep in mind rounding errors between the languages.
