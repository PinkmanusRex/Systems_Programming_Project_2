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
#include "analysis_threads.h"

unsigned int dN = 1, fN = 1, aN = 1;
char* suffix; 

char *file_suffix;
pthread_mutex_t dir_term_mutex;
pthread_mutex_t file_term_mutex;
unsigned int no_waiting_dirs;
unsigned int no_waiting_files;
unsigned int no_dir_threads;
unsigned int no_file_threads;
pthread_cond_t cond_dir;
pthread_cond_t cond_file;
unsigned int dir_threads_terminate;
sync_queue *directory_queue;
sync_queue *file_queue;
wf_repo *wf_stack;
wf_table *x;
wf_table *y;
unsigned int jsd_comp_iter;
unsigned int jsd_total_comp;
jsd_entry **jsd_list;
pthread_mutex_t analysis_mutex;

#ifdef DEBUG
pthread_mutex_t jsd_sync_mutex;
#endif

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
        fprintf(stdout, "Invalid -option arguments, program halt & terminated");
        free(suffix);
        exit(EXIT_FAILURE);
    }

    /** wrap these print statements inside an if(DEBUG) block */
    //printf("dN = %u \n", dN);
    //printf("fN = %u \n", fN);
    //printf("aN = %u \n", aN);
    //printf("suffix = %s + %ld\n", suffix, strlen(suffix));
    //free(suffix);
    /**
     * TODO:
     *  initialize the queues and the relevant extern variables
     *  fill the queues with the preliminary directories and files as taken from the command-line
     */

    // Initialize and populate the synchronized queues
    directory_queue = sync_q_create();
    file_queue = sync_q_create();
    wf_stack = wf_repo_create();

    if(directory_queue == NULL || file_queue == NULL || wf_stack == NULL){
        perror("Failure to initialize a syncronized queue (file and/or directory and/or wf repo)");
        exit(EXIT_FAILURE);
    }

    // Siphon off arguments from argv into respective queue.
    struct stat argumentData;
    int siphonedToFileQueue;
    int siphonedToDirQueue;
    for(int i=1; i<argc; ++i){
        /* I allow argv arguments to start with a .
            For example: ./compare ../ is a valid argument*/

        if(argv[i][0] == '-'){
            continue; // options have already been initialized.
        }
        if(stat(argv[i], &argumentData) == -1){
            // argv[i] doesn't exist 
            perror(argv[i]);
            continue;
        }

        // Now that we know argv[i] has a valid address and isn't an option flag
        // We can move onto siphoning to respective queue provided it has read permissions.
        char* pathname = (char *) malloc(sizeof(char) * strlen(argv[i]) + 2);
        strcpy(pathname, argv[i]); // Needed since directoryFunction_r frees.
        
        if(S_ISREG(argumentData.st_mode)){
            // If we can't open the regular file, continue forward.
            int inputFD = open(pathname, O_RDONLY);
            if(inputFD == -1){
                perror(pathname);
                free(pathname);  // important because we aren't exiting.
                continue;
            }
            close(inputFD);
            
            // Add to file Queue
            siphonedToFileQueue = sync_q_add(file_queue, pathname);
            if(siphonedToFileQueue == EXIT_FAILURE){
                perror("sync_q library line 62");
                free(pathname);
                continue;
            }
        }
        else if(S_ISDIR(argumentData.st_mode)){
            // If the directory cannot be opened, no point in adding it
            DIR* temporaryCheckForPerms = opendir(pathname);
            if(temporaryCheckForPerms == NULL){
                perror(pathname);
                free(pathname);
                continue;
            }
            closedir(temporaryCheckForPerms);

            // Add to directoryQueue
            siphonedToDirQueue = sync_q_add(directory_queue, pathname);
            if(siphonedToDirQueue == EXIT_FAILURE){
                perror("sync_q library line 62");
                free(pathname);
                continue;
            }
        }
        else{
            free(pathname);
        }
    }

        // There is no point in moving on if nothing further to do.
    if(sync_q_empty(file_queue) && sync_q_empty(directory_queue)){
        perror("main thread did not populate queues. Terminating");
        sync_q_destroy(directory_queue); sync_q_destroy(file_queue);
        free(suffix);
        exit(EXIT_FAILURE);
    }

    // Initialize relevant extern variables and flags
    no_waiting_dirs = 0;
    no_waiting_files = 0;
    no_dir_threads = dN;
    no_file_threads = fN;

    dir_threads_terminate = 0;
    
    // Initialize relevant extern mutexes and conditional vars
    if(pthread_mutex_init(&dir_term_mutex, NULL) != 0) {
            perror("mutex init failure"); 
            exit(EXIT_FAILURE);
    }
    
    if(pthread_mutex_init(&file_term_mutex, NULL) != 0) {
            perror("mutex init failure"); 
            exit(EXIT_FAILURE);
    }

    if(pthread_cond_init(&cond_dir, NULL) != 0) {
            perror("cond init failure"); 
            exit(EXIT_FAILURE);
    }
    
    if(pthread_cond_init(&cond_file, NULL) != 0) {
            perror("cond init failure"); 
            exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&analysis_mutex, NULL) != 0) {
            perror("mutex init failure"); 
            exit(EXIT_FAILURE);
    }

#ifdef DEBUG
    if (pthread_mutex_init(&jsd_sync_mutex, NULL) != 0) {
            perror("utex init failure");
            exit(EXIT_FAILURE);
    }
#endif

    /**
     * TODO:
     *  declare and initialize array of directory thread and file thread ids for later joining
     *  actually start the directory and file threads
     *  join the directory threads and then the file threads
     *  cleanup the queues and all the shared resources that are not relevant or needed for the analysis phase
     */

    // pthread_t is a variable used to track the Thread id of created thread
    pthread_t* directoryTID = (pthread_t*) malloc(no_dir_threads * sizeof(pthread_t));
    if(directoryTID == NULL){
        perror("malloc failed to initialize array of directory pthreads");
        exit(EXIT_FAILURE);
    }

    pthread_t* filesTID = (pthread_t*) malloc(no_file_threads * sizeof(pthread_t));
    if(filesTID == NULL){
        perror("malloc failed to initialize array of file pthreads");
        exit(EXIT_FAILURE);
    }

    file_suffix = suffix;
    
    /* Create directory and file threads, and pass in their routines */
    int create;
    for(int i=0; i < no_dir_threads; i++){
        // check to make sure that the last condition is true, our file_queue/dir_queue are global so I pass NULL
        create = pthread_create(&directoryTID[i], NULL, dir_thread_routine, NULL); 
        if(create != 0){
            perror("Error creating directory thread");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0; i < no_file_threads; i++){
        // check to make sure that the last condition is true, our file_queue/dir_queue are global so I pass NULL
        create = pthread_create(&filesTID[i], NULL, file_thread_routine, NULL); 
        if(create != 0){
            perror("Error creating file thread");
            exit(EXIT_FAILURE);
        }
    }

        /* !! Wait for directory threads and file threads to do their job !! */
    int join;
    for(int i=0; i < no_dir_threads; i++){
        join = pthread_join(directoryTID[i], NULL); // thread routines don't return anything.
        if(join != 0){
            perror("Error joining directory thread");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=0; i < no_file_threads; i++){
        join = pthread_join(filesTID[i], NULL); // thread routines don't return anything.
        if(join != 0){
            perror("Error joining directory thread");
            exit(EXIT_FAILURE);
        }
    }

    // Now that they are done doing their work, we can free memory allocated for their jobs.
    // But first ensure they are empty.
    if(!(sync_q_empty(file_queue) && sync_q_empty(directory_queue))){
        fprintf(stdout, "No items in file queue: %d, No items in directory queue: %d\n", file_queue->entries, directory_queue->entries);
        perror("Something went wrong in routines.");
        free(suffix);
        exit(EXIT_FAILURE);
    }

    // Deallocate all the things that aren't necessary for analysis phase.

    sync_q_destroy(file_queue); // Don't have to check return value since it will always be success.
    sync_q_destroy(directory_queue); // if it isnt' success, then it terminates and we don't get here anyway.

    free(suffix); // Don't need this for analysis phase.

    int mutex_status = 0;
    mutex_status = pthread_mutex_destroy(&dir_term_mutex);
    if (mutex_status != 0) {
        perror("Mutex destroy error dirTermMutex");
        exit(EXIT_FAILURE);
    }
    mutex_status = pthread_mutex_destroy(&file_term_mutex);
    if (mutex_status != 0) {
        perror("Mutex destroy error fileTermMutex");
        exit(EXIT_FAILURE);
    }

    int cond_status = 0;
    cond_status = pthread_cond_destroy(&cond_dir);
    if(cond_status != 0){
        perror("condition variable destroy error cond_dir");
        exit(EXIT_FAILURE);
    }

    cond_status = pthread_cond_destroy(&cond_file);
    if(cond_status != 0){
        perror("condition variable destroy error cond_file");
        exit(EXIT_FAILURE);
    }

    free(filesTID);
    free(directoryTID);

    /** the debug print of the contents of the wf_repo */
#ifdef DEBUG
     debug_wf_repo_print(wf_stack);
#endif

    /**
     * TODO:
     *  analysis phase, where the number of analysis threads should not exceed the number of comparisons (1/2)(n)(n-1)
     *  once analysis threads all finish, the final array of jsd results must be quicksorted utilizing the provided jsd_comparator
     */
     int err_flag = 0;
     if (wf_stack->size >= 2) {
             x = wf_stack->table;
             y = x->next;
             jsd_comp_iter = 0;
             jsd_total_comp = wf_stack->size * (wf_stack->size - 1) / 2;
             jsd_list = jsd_create_list(jsd_total_comp);
             if (!jsd_list) {
                     fprintf(stderr, "malloc failure: could not allocate space for jsd_list\n");
                     exit(EXIT_FAILURE);
             }
             if (aN > jsd_total_comp) {
                     aN = jsd_total_comp;
             }
             pthread_t *analysisTID = malloc(sizeof(pthread_t) * aN);
             if (!analysisTID) {
                     fprintf(stderr, "malloc failure: could not allocate space for analysis thread array\n");
                     exit(EXIT_FAILURE);
             }
             int create = 0;
             for (int i = 0; i < aN; i += 1) {
                     create = pthread_create(&analysisTID[i], NULL, analysis_thread_routine, NULL);
                     if (create != 0) {
                             perror("error creating analysis thread");
                             exit(EXIT_FAILURE);
                     }
             }
             int join = 0;
             for (int i = 0; i < aN; i += 1) {
                     join = pthread_join(analysisTID[i], NULL);
                     if (join != 0) {
                             perror("error joining analysis thread");
                             exit(EXIT_FAILURE);
                     }
             }
             free(analysisTID);
             qsort((void *)(jsd_list), jsd_total_comp, sizeof(jsd_entry *), jsd_comparator);
             jsd_print_list(jsd_list, jsd_total_comp);
             jsd_destroy_list(jsd_list, jsd_total_comp);
     }
     else {
             err_flag = 1;
     }
     mutex_status = pthread_mutex_destroy(&analysis_mutex);
     if (mutex_status != 0) {
             perror("mutex destroy error analysis_mutex");
             exit(EXIT_FAILURE);
     }
#ifdef DEBUG
     mutex_status = pthread_mutex_destroy(&jsd_sync_mutex);
     if (mutex_status != 0) {
             perror("mutex destroy error jsd_sync_mutex");
             exit(EXIT_FAILURE);
     }
#endif

    /**
     * TODO:
     *  cleanup the repo
     */
     wf_repo_clear(wf_stack);
     wf_repo_destroy(wf_stack);

     if (err_flag) {
             return EXIT_FAILURE;
     }
     return EXIT_SUCCESS;
}
