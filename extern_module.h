#ifndef EXTERN_MODULE_H
#define EXTERN_MODULE_H
#include <pthread.h>

/** the mutex's that protect access to no_waiting_dirs and no_waiting_files as well as the flags */
extern pthread_mutex_t dir_term_mutex;
extern pthread_mutex_t file_term_mutex;

/** track number of waiting directory threads and file threads */
extern int no_waiting_dirs;
extern int no_waiting_files;

/** conditional variables for use in signalling and waiting the helper, directory threads, and file threads */
extern pthread_cond_t cond_helper;
extern pthread_cond_t cond_dir;
extern pthread_cond_t cond_file;

/** flags that will let the threads determine whether to terminate or not */
extern int dir_threads_terminate;
extern int file_threads_terminate;
#endif
