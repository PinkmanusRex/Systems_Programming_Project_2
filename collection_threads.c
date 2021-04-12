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
#include "wf_repo.h"
#include "wf_table.h"
#include "sync_queue.h"
#include "tokenize.h"
#include "debugger.h"

void *dir_thread_routine(void *arg){
    int is_err = 0;
#ifdef DEBUG
    fprintf(stdout, "dir_thread_routine %lu started\n", pthread_self());
#endif
    int mutex_status = 0;
    while (1) {
        /** check if there is anything in the directory queue */
        mutex_status = pthread_mutex_lock(&dir_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
#ifdef DEBUG
        fprintf(stdout, "dir_thread_routine %lu :\n\tno. items in directory queue: %d\n", pthread_self(), directory_queue->entries);
#endif
        while (sync_q_empty(directory_queue)) {
            no_waiting_dirs += 1;
            /** once the last directory thread is waiting, there can be no more directories that will be added
             * since there are no more directories, set up the necessary variables and send the signal to
             * wake up the sleeping directory threads and any sleeping file threads to terminate
             */
            if (no_waiting_dirs == no_dir_threads) {
                /** protect the file term mutex in order to ensure that the file threads get
                 * accurate reads of the directory termination flag
                 */
                mutex_status = pthread_mutex_lock(&file_term_mutex);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                dir_threads_terminate = 1;
                /** awaken the file threads for termination */
                mutex_status = pthread_cond_broadcast(&cond_file);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                mutex_status = pthread_mutex_unlock(&file_term_mutex);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                /** awaken the directories for termination */
                mutex_status = pthread_cond_broadcast(&cond_dir);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                mutex_status = pthread_mutex_unlock(&dir_term_mutex);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                if (is_err) {
                    return (void *) EXIT_FAILURE;
                }
                return (void *) EXIT_SUCCESS;
            }
            /** this thread was not the last thread, therefore it does not know if any other directory threads
             * will add anything to the queue. This thread must sleep until awoken
             * the awakening conditions are either
             * a) something added to the queue
             * b) time to terminate
             */
            mutex_status = pthread_cond_wait(&cond_dir, &dir_term_mutex);
            if (mutex_status != 0) {
                perror("ERROR!!!");
                exit(EXIT_FAILURE);
            }
            /** where the wake up signal was the result of termination */
            if (dir_threads_terminate) {
                mutex_status = pthread_mutex_unlock(&dir_term_mutex);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                if (is_err) {
                    return (void *) EXIT_FAILURE;
                }
                return (void *) EXIT_SUCCESS;
            }
            no_waiting_dirs -= 1;
        }
        mutex_status = pthread_mutex_unlock(&dir_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }

        queue_node *dir = sync_q_remove(directory_queue);
        if (!dir) {
            continue;
        }

        Node *files = malloc(sizeof(Node));
        if (!files) {
            perror("collection_threads.c: directory_thread_routine - MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }
        Node *dirs = malloc(sizeof(Node));
        if (!dirs) {
            perror("collection_threads.c: directory_thread_routine - MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }
        files->value = 0;
        files->next = 0;
        dirs->value = 0;
        dirs->next = 0;
#ifdef DEBUG
        fprintf(stdout, "\t%lu: dir->name: %s, file_suffix: %s\n", pthread_self(), dir->name, file_suffix);
#endif
        if(directoryFunction_r(dir->name, files, dirs, file_suffix)==EXIT_FAILURE) {
            is_err = 1;
        }

        /** flags to track if anything added to their respective queues */
        int dirs_added = 0;
        int files_added = 0;

        Node *dirs_ptr = dirs;
        Node *files_ptr = files;
        /** since there must be an empty node which serves as the bottom of the stack, we cannot add that actual empty node
         * the empty node has a field called value which is 0
         */
        
        /** add the directories to the directory queue */
        while (dirs_ptr->value != 0) {
            if (sync_q_add(directory_queue, dirs_ptr->value)==EXIT_FAILURE){
                perror("collection_threads.c: directory_thread_routine - SYNC_Q_ADD MALLOC FAILURE!!!");
                exit(EXIT_FAILURE);
            }
            dirs_added = 1;
            dirs_ptr = dirs_ptr->next;
        }

        mutex_status = pthread_mutex_lock(&dir_term_mutex);
        if (mutex_status != 0){
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
        /** since something was added to the directory queue, the sleeping directory threads that
         * can wake up.
         */
        if (dirs_added) {
            mutex_status = pthread_cond_broadcast(&cond_dir);
            if (mutex_status != 0) {
                perror("ERROR!!!");
                exit(EXIT_FAILURE);
            }
        }
        mutex_status = pthread_mutex_unlock(&dir_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }

        /** add the files to the file queue */
        while (files_ptr->value != 0) {
            if (sync_q_add(file_queue, files_ptr->value)==EXIT_FAILURE){
                perror("collection_threads.c: directory_thread_routine - SYNC_Q_ADD MALLOC FAILURE!!!");
                exit(EXIT_FAILURE);
            }
            files_added = 1;
            files_ptr = files_ptr->next;
        }

        mutex_status = pthread_mutex_lock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
        /** same line of reasoning as from the if(dirs_added) statement */
        if (files_added) {
            mutex_status = pthread_cond_broadcast(&cond_file);
            if (mutex_status != 0) {
                perror("ERROR!!!");
                exit(EXIT_FAILURE);
            }
        }
        mutex_status = pthread_mutex_unlock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }

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
        free(dir);
    }
}

void *file_thread_routine(void *arg){
    int is_err = 0;
    int mutex_status = 0;
    /** initialize the stringbuf here to save the amount of memory allocations needed */
    stringbuf *list = sb_create(10);
    if (!list) {
        perror("collection_threads.c: file_thread_routine - MALLOC FAILURE!!!");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG
    fprintf(stdout, "file_thread_routine %lu started\n", pthread_self());
#endif
    while (1) {
        mutex_status = pthread_mutex_lock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
#ifdef DEBUG
        fprintf(stdout, "file_thread_routine %lu:\n\tno entries in file queue: %d\n", pthread_self(), file_queue->entries);
#endif
        /** check similarly to the directory_thread_routine, where an empty file queue is not necessarily the end of all work */
        while (sync_q_empty(file_queue)) {
            /** suppose that the directory threads set termination before a file thread sees the file queue as empty
             * this check is for that purpose
             */
            if (dir_threads_terminate) {
                mutex_status = pthread_mutex_unlock(&file_term_mutex);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                sb_destroy(list);
                if (is_err) {
                    return (void *) EXIT_FAILURE;
                }
                return (void *) EXIT_SUCCESS;
            }
            /** if the above is not seen, then the file thread cannot know if it must truly be terminated just yet, therefore
             * it should sleep until awoken, either because the directory thread added to the file queue, or because
             * the directory threads are set to terminate, thus sending a signal to wake up all the file threads
             */
            mutex_status = pthread_cond_wait(&cond_file, &file_term_mutex);
            if (mutex_status != 0) {
                perror("ERROR!!!");
                exit(EXIT_FAILURE);
            }
            /** this is one of the two situations for waking up
             * a) because the directory threads added to the file queue or, as in this case
             * b) the last directory thread has sent a termination signal
             */
            if (dir_threads_terminate && sync_q_empty(file_queue)) {
                mutex_status = pthread_mutex_unlock(&file_term_mutex);
                if (mutex_status != 0) {
                    perror("ERROR!!!");
                    exit(EXIT_FAILURE);
                }
                sb_destroy(list);
                if (is_err) {
                    return (void *) EXIT_FAILURE;
                }
                return (void *) EXIT_SUCCESS;
            }
        }
        mutex_status = pthread_mutex_unlock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }

        queue_node *file = sync_q_remove(file_queue);
        if (!file) {
            continue;
        }

        /** protect the file opening operation so such an int value is
         * not associated with any other files
         */
        mutex_status = pthread_mutex_lock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
        int fd = open(file->name, O_RDONLY);
        mutex_status = pthread_mutex_unlock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }

        /** could not open the file */
        if (fd == -1) {
            perror(file->name);
            is_err = 1;
            continue;
        }

        /** create the table which we'll populate, initializing with no. rows 30 and a y of 3.0 */
        wf_table *table = hash_create_table(file->name, 30, 3.0);
        if (!table) {
            /** something went wrong with malloc, so this is a major error */
            perror("wf_table.c: hash_create_table - MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }

        if (tokenize(fd, table, list)==EXIT_FAILURE) {
            /** something went wrong with malloc, so this is a major error */
            perror("tokenize.c: tokenize - MALLOC FAILURE!!!");
            exit(EXIT_FAILURE);
        }

        /** protect the close operation just as extra safety measure */
        mutex_status = pthread_mutex_lock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
        int close_stat = close(fd);
        if (close_stat == -1) {
            perror(file->name);
            exit(EXIT_FAILURE);
        }
        mutex_status = pthread_mutex_unlock(&file_term_mutex);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }

        free(file);

        wf_repo_push(wf_stack, table);
    }

    /** stringbuffer has no purpose, as the file thread must now terminate */
    sb_destroy(list);
    if (is_err) {
        return (void *) EXIT_FAILURE;
    }
    return (void *) EXIT_SUCCESS;
}
