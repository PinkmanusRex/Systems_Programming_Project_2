#ifndef EXTERN_MODULE_H
#define EXTERN_MODULE_H
#include <pthread.h>
#include "sync_queue.h"
#include "jsd.h"
#include "wf_table.h"
#include "wf_repo.h"

/** the suffix */
extern char *file_suffix;

/** the mutex's that protect access to no_waiting_dirs and no_waiting_files as well as the flags */
extern pthread_mutex_t dir_term_mutex;
extern pthread_mutex_t file_term_mutex;

/** track number of waiting directory threads and file threads */
extern unsigned int no_waiting_dirs;
extern unsigned int no_waiting_files;

extern unsigned int no_dir_threads;
extern unsigned int no_file_threads;

/** conditional variables for use in signalling and waiting the directory threads, and file threads */
extern pthread_cond_t cond_dir;
extern pthread_cond_t cond_file;

/** flags that will let the threads determine whether to terminate or not */
extern unsigned int dir_threads_terminate;

/** the directory and file queues that should be shared with all directory and file threads */
extern sync_queue *directory_queue;
extern sync_queue *file_queue;

/** the wf repo that will be populated in the collection phase and utilized in the analysis phase */
extern wf_repo *wf_stack;

/** pointers to wf_tables to help in the jsd/analysis work assigning portion */
extern wf_table *x;
extern wf_table *y;
extern unsigned int jsd_comp_iter;
extern unsigned int jsd_total_comp;
extern jsd_entry **jsd_list;
#endif
