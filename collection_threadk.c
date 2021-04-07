#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include "collection_threads.h"
#include "extern_module.h"
#include "helperR.h"
#include "stringbuf.h"
#include "strbuf.h"
#include "wf_repo.h"
#include "wf_table.h"
#include "sync_queue.h"
#include "tokenize.h"
#include "wf_repo.h"



void *dir_thread_routinek(void *arg){
    while(1)
    {
        //CRITICAL SECTION BEGINS
        // The thread begins routine waiting to dequeue.
        if(pthread_mutex_lock(&dir_term_mutex) != 0) {perror("lock error a1"); exit(EXIT_FAILURE);}
        ++no_waiting_dirs;

        // Avoid spurious wakeup call.
        while (sync_q_empty(directory_queue)){
            // If the queue is empty an remainder of threads are waiting, then terminate.
            if(no_waiting_dirs == no_dir_threads){
                --no_dir_threads;
                --no_waiting_dirs;
                if(pthread_mutex_unlock(&dir_term_mutex) != 0) {perror("unlock error a1"); exit(EXIT_FAILURE);}
                if(pthread_cond_signal(&cond_dir)!=0){perror("signal error a1"); exit(EXIT_FAILURE);}
                pthread_exit(NULL); // man 3 pthread_exit RETURN VALUE suggests this always succeeds.
            }
            // Will not have a race condition because dir_term_mutex protecting this critical section.
            if(pthread_cond_wait(&cond_dir, &dir_term_mutex) != 0) {perror("wait qEmpty loop"); exit(EXIT_FAILURE);}
        }

        /* To avoid a race condition, dequeue, then unlock */
        --no_waiting_dirs;
        queue_node *dir = sync_q_remove(directory_queue);
        if(pthread_mutex_unlock(&dir_term_mutex) != 0) {perror("unlock error a1.2"); exit(EXIT_FAILURE);}
        if (!dir) {
            continue;
        }
        // CRITICAL SECTION ENDS


        /* Thread safe computations which can run concurrently. At this point we are holding no locks. */
        /* Code similar to dir_thread_routine from here */
        Node *files = malloc(sizeof(Node));
        if (!files) {
            fprintf(stderr, "MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }
        Node *dirs = malloc(sizeof(Node));
        if (!dirs) {
            fprintf(stderr, "MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }
        files->value = 0;
        files->next = 0;
        dirs->value = 0;
        dirs->next = 0;
        directoryFunction_r(dir->name, files, dirs, file_suffix);
        
        Node *dirs_ptr = dirs;
        Node *files_ptr = files;
        /* Code similarity ends */

        /** since there must be an empty node which serves as the bottom of the stack, we cannot add that actual empty node
         * the empty node has a field called value which is 0
         */
        
        /** add the directories to the directory queue */
        while (dirs_ptr->value != 0) {
            if (sync_q_add(directory_queue, dirs_ptr->value)==EXIT_FAILURE){
                fprintf(stderr, "MALLOC FAILURE dirs_ptr->value 1");
                exit(EXIT_FAILURE);
            }
            if(pthread_cond_signal(&cond_dir)!=0){perror("signal error adding directory to directoryQueue"); exit(EXIT_FAILURE);}
            dirs_ptr = dirs_ptr->next;
        }

        while (files_ptr->value != 0) {
            if (sync_q_add(file_queue, files_ptr->value)==EXIT_FAILURE){
                fprintf(stderr, "MALLOC FAILURE!!! files_ptr->value");
                exit(EXIT_FAILURE);
            }
            if(pthread_cond_signal(&cond_file)!=0){perror("signal error adding file to fileQueue"); exit(EXIT_FAILURE);}
            files_ptr = files_ptr->next;
        }

        /* Code similarity begins */
        // Free all relevant materials.

        dirs_ptr = dirs;
        Node *next_dir = 0;
        files_ptr = files;
        Node *next_file = 0;
        /** these Nodes must be freed, but the strings inside of them must be kept for later use, so just free the Node and not the strings */
        while (dirs_ptr) {
            next_dir = dirs_ptr->next;
            free(dirs_ptr);
            dirs_ptr = next_dir;
        }
        while(files_ptr) {
            next_file = files_ptr->next;
            free(files_ptr);
            files_ptr = next_file;
        }

        /** the directory which was dequeued is no longer of use, so free it */
        free(dir->name);
        free(dir); 
    }
}


void *file_thread_routinek(void *arg){
    int mutex_status = 0;

    /** initialize the stringbuf here to save the amount of memory allocations needed */
    stringbuf *list = sb_create(10);

    while (1) {
    
        if(pthread_mutex_lock(&dir_term_mutex) != 0) {perror("lock error a1"); exit(EXIT_FAILURE);}
        /* Avoid spurious wakeups. */
        while (sync_q_empty(file_queue)) { // Method is thread-safe.
            /* If no more directory threads, then this file thread is obsolete */
            if(no_dir_threads == 0){
                if(pthread_mutex_unlock(&dir_term_mutex) != 0) {perror("unlock error a1"); exit(EXIT_FAILURE);}
                if(pthread_cond_signal(&cond_file)!=0){perror("signal error a1"); exit(EXIT_FAILURE);}
                pthread_exit(NULL); // man 3 pthread_exit RETURN VALUE suggests this always succeeds.
            }
            /* This thread can remain suspended until woken up because something inserted into file_queue */
            if(pthread_cond_wait(&cond_file, &dir_term_mutex) != 0) {perror("wait file Empty loop"); exit(EXIT_FAILURE);}
        }

        if(pthread_mutex_unlock(&dir_term_mutex) != 0) {perror("unlock error a2"); exit(EXIT_FAILURE);}


        queue_node *file = sync_q_remove(file_queue); // Thread safe Method
        if (!file) {
            /* Race condition cannot be ruled out. However this check ensures, no matter thread sheduling, we don't act upon NULL retrieved items */
            continue;
        }

        /* Similar code begins */

        int mutexStatus = 0;
        /** protect the file opening operation so such an int value is
         * not associated with any other files
         */
        if(pthread_mutex_lock(&file_term_mutex)!=0){perror("Lock file_term_mutex b1"); exit(EXIT_FAILURE);}
        int fd = open(file->name, O_RDONLY);
        if(pthread_mutex_unlock(&file_term_mutex)!=0){perror("Unlock file_term_mutex b1"); exit(EXIT_FAILURE);}

        if (fd == -1) {
            perror(file->name);
            continue;
        }


        /** create the table which we'll populate, initializing with no. rows 30 and a y of 3.0 */
        /* The pointer to table is in the local automatic stack, hence not shared between threads */
        wf_table *table = hash_create_table(file->name, 30, 3.0);
        if (!table) {
            /** something went wrong with malloc, so this is a major error */
            fprintf(stderr, "hashCreateTable - MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }

        if (tokenize(fd, table, list)==EXIT_FAILURE) {
            /** something went wrong with malloc, so this is a major error */
            fprintf(stderr, "tokenize - MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }

        /** protect the close operation just as extra safety measure */
        if(pthread_mutex_lock(&file_term_mutex)!=0){perror("Lock file_term_mutex c1"); exit(EXIT_FAILURE);}
        int close_stat = close(fd);
        if (close_stat == -1) {
            perror(file->name);
            exit(EXIT_FAILURE);
        }
        if(pthread_mutex_unlock(&file_term_mutex)!=0){perror("Unlock file_term_mutex c1"); exit(EXIT_FAILURE);}

        /* The wf_repo_push method is thread-safe, mutex for that stack found in struct wf_repo.h */
        wf_repo_push(wf_stack, table);
    }

    /** stringbuffer has no purpose, as the file thread must now terminate */
    sb_destroy(list);
    return 0;
}