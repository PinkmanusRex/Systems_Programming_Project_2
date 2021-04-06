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

void *dir_thread_routine(void *arg){

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
