#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <dirent.h>

#include "jsd.h"
#include "tokenize.h"
#include "stringbuf.h"
#include "wf_table.h"
#include "helperR.h"
#include "collection_threads.h"
#include "debugger.h"
#include "sync_queue.h"
#include "extern_module.h"
#include "wf_repo.h"

unsigned int dN = 1, fN = 1, aN = 1;
char* suffix; 



/* Returns 0 upon invalid */
unsigned int getDigits(char* toConvert){
    for(int i=2; i<strlen(toConvert); i++){
        if(!isdigit(toConvert[i])){
            return 0;
        }
    }
    
    unsigned int result = atoi(&toConvert[2]);
    return result;
}

/* Not thread_safe */
int initializeOptions(int numArgs, char** Args){
    for(int i=1; i<numArgs; ++i){
        char* current = Args[i];
        // We know it is an option.
        if(current[0] == '-'){
            // Check whether invalid option
            int lengthOfCurrent = strlen(current);
            if(lengthOfCurrent == 1){
                // String is only "-"
                return EXIT_FAILURE;
            }

            char optionFlag = current[1];
            if(optionFlag == 'd'){
                if(lengthOfCurrent == 2){return EXIT_FAILURE;}
                dN = getDigits(current);
                if(dN == 0){
                    return EXIT_FAILURE;
                }
            }
            else if(optionFlag == 'f'){
                if(lengthOfCurrent == 2){return EXIT_FAILURE;}
                fN = getDigits(current);
                if(fN == 0){
                    return EXIT_FAILURE;
                }
            }
            else if(optionFlag == 'a'){
                if(lengthOfCurrent == 2){return EXIT_FAILURE;}
                aN = getDigits(current);
                if(aN == 0){
                    return EXIT_FAILURE;
                }

            }
            else if(optionFlag == 's'){
                free(suffix);
                suffix = (char*) malloc(lengthOfCurrent * sizeof(char));
                strcpy(suffix, (current+2));
            }
            else{
                // Invalid option.
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char** argv){
    /* Will not need mutex to check flags */
    /* Options can be given in any order, any # of times
        The last flag overwrites the value.
    */
    if(argc < 2){
        perror("Provide a file or directory.");
        return EXIT_FAILURE;
    }

    suffix = malloc(10 * sizeof(char));
    strcpy(suffix, ".txt");

    if(initializeOptions(argc, argv) == EXIT_FAILURE){
        free(suffix);
        perror("Invalid -option arguments, program halt & terminated");
        return EXIT_FAILURE;
    }

    /** wrap these print statements inside an if(DEBUG) block */
    printf("dN = %u \n", dN);
    printf("fN = %u \n", fN);
    printf("aN = %u \n", aN);
    printf("suffix = %s + %ld\n", suffix, strlen(suffix));
    free(suffix);
    /**
     * TODO:
     *  initialize the queues and the relevant extern variables
     *  fill the queues with the preliminary directories and files as taken from the command-line
     */

    /**
     * TODO:
     *  declare and initialize array of directory thread and file thread ids for later joining
     *  actually start the directory and file threads
     *  join the directory threads and then the file threads
     *  cleanup the queues and all the shared resources that are not relevant or needed for the analysis phase
     */

    /** the debug print of the contents of the wf_repo */
    if (DEBUG) {
            debug_wf_repo_print(wf_stack);
    }

    /**
     * TODO:
     *  analysis phase, where the number of analysis threads should not exceed the number of comparisons (1/2)(n)(n-1)
     *  once analysis threads all finish, the final array of jsd results must be quicksorted utilizing the provided jsd_comparator
     */

    /**
     * TODO:
     *  cleanup the repo
     */

    return EXIT_SUCCESS;
}
