#ifndef WF_REPO_H
#define WF_REPO_H
#include <pthread.h>
#include "wf_table.h"
typedef struct wf_repo {
        int entries;
        wf_table *front;
        wf_table *rear;
        pthread_mutex_t lock;
} wf_repo;

/** likely not very necessary, but perhaps nice to have? 
 *  0 is false, 1 is true
 * */
int wf_repo_empty(wf_repo *);

/** similarly, we won't really dequeue the wf_repo, but might be nice to have */


#endif
