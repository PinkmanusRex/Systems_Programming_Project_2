#ifndef WF_REPO_H
#define WF_REPO_H
#include <pthread.h>
#include "wf_table.h"

/* Pre-Analysis Stage stack */
typedef struct wf_repo{
        int size; // number of items in the Stack
        wf_table* table;
        pthread_mutex_t lock;
} wf_repo;


/**
 * Creates and initalizes a synchronized stack (WRD repo)
 * Returns NULL as error
 */
wf_repo* wf_repo_create(); 

/** for the purpose of clearing the stack at the end of analysis */
int wf_repo_clear(wf_repo *);

/**
 * RETURN: EXIT_SUCCESS when deleting empty wf_repo, 
 * EXIT_FAILURE if non-empty stack passed (will not delete if not empty)
 */
int wf_repo_destroy(wf_repo*);

/** RETURN: 0 if stack empty, 1 otherwise.
 * */
int wf_repo_empty(wf_repo *);

/* Please check this */
void wf_repo_push(wf_repo*, wf_table* table);


/** RETURN: Item on top of stack, or NULL if stack empty.
 * */
wf_table* wf_repo_pop(wf_repo*);

#endif
