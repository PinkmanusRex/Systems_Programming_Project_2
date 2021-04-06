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

void *dir_thread_routine(void *arg){
        // fill in as necessary
        int mutex_status = 0;
        while (1) {
                mutex_status = pthread_mutex_lock(&dir_term_mutex);
                if (mutex_status != 0) {
                        perror("ERROR!!!");
                        exit(EXIT_FAILURE);
                }
                while (sync_q_empty(directory_queue)) {
                        no_waiting_dirs += 1;
                        if (no_waiting_dirs == no_dir_threads) {
                                mutex_status = pthread_mutex_lock(&file_term_mutex);
                                if (mutex_status != 0) {
                                        perror("ERROR!!!");
                                        exit(EXIT_FAILURE);
                                }
                                dir_threads_terminate = 1;
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
                                return 0;
                        }
                        mutex_status = pthread_cond_wait(&cond_dir, &dir_term_mutex);
                        if (mutex_status != 0) {
                                perror("ERROR!!!");
                                exit(EXIT_FAILURE);
                        }
                        if (dir_threads_terminate) {
                                mutex_status = pthread_mutex_unlock(&dir_term_mutex);
                                if (mutex_status != 0) {
                                        perror("ERROR!!!");
                                        exit(EXIT_FAILURE);
                                }
                                return 0;
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
                /*
                 * fill in as necessary
                 * must track if anything was added to the directory queue
                 * must track if anything was added to the file queue
                 */
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
                int dirs_added = 0;
                int files_added = 0;
                Node *dirs_ptr = dirs;
                Node *files_ptr = files;
                while (dirs_ptr->value != 0) {
                        if (sync_q_add(directory_queue, dirs_ptr->value)==EXIT_FAILURE){
                                fprintf(stderr, "MALLOC FAILURE!!!");
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
                while (files_ptr->value != 0) {
                        if (sync_q_add(file_queue, files_ptr->value)==EXIT_FAILURE){
                                fprintf(stderr, "MALLOC FAILURE!!!");
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
                free(dir->name);
                free(dir);
        }
}

void *file_thread_routine(void *arg){
        int mutex_status = 0;
        stringbuf *list = sb_create(10);
        while (1) {
                mutex_status = pthread_mutex_lock(&file_term_mutex);
                if (mutex_status != 0) {
                        perror("ERROR!!!");
                        exit(EXIT_FAILURE);
                }
                while (sync_q_empty(file_queue)) {
                        if (dir_threads_terminate) {
                                mutex_status = pthread_mutex_unlock(&file_term_mutex);
                                if (mutex_status != 0) {
                                        perror("ERROR!!!");
                                        exit(EXIT_FAILURE);
                                }
                                return 0;
                        }
                        mutex_status = pthread_cond_wait(&cond_file, &file_term_mutex);
                        if (mutex_status != 0) {
                                perror("ERROR!!!");
                                exit(EXIT_FAILURE);
                        }
                        if (dir_threads_terminate) {
                                mutex_status = pthread_mutex_unlock(&file_term_mutex);
                                if (mutex_status != 0) {
                                        perror("ERROR!!!");
                                        exit(EXIT_FAILURE);
                                }
                                return 0;
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
                if (fd == -1) {
                        perror(file->name);
                        continue;
                }
                wf_table *table = hash_create_table(file->name, 30, 3.0);
                if (!table) {
                        /** something went wrong with malloc, so this is a major error */
                        fprintf(stderr, "MALLOC FAILURE!!!");
                        exit(EXIT_FAILURE);
                }
                if (tokenize(fd, table, list)==EXIT_FAILURE) {
                        /** something went wrong with malloc, so this is a major error */
                        fprintf(stderr, "MALLOC FAILURE!!!");
                        exit(EXIT_FAILURE);
                }
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
                wf_repo_push(wf_stack, table);
        }
        sb_destroy(list);
        return 0;
}
