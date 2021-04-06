#include <pthread.h>
#include "wf_repo.h"
#include <stdlib.h>

wf_repo* wf_repo_create(){
    wf_repo* stack = (wf_repo *) malloc(sizeof(wf_repo));
    stack->size = 0;    // # elements in stack 0
    stack->table = NULL; // of type wf_table
    pthread_mutex_init(&stack->lock, NULL);
    return stack;
}

int wf_repo_destroy(wf_repo* stack){
    // Check if empty, if not then delete.
    pthread_mutex_lock(&stack->lock);
    if(stack->size != 0){
        pthread_mutex_unlock(&stack->lock);
        return EXIT_FAILURE;
    }
    pthread_mutex_unlock(&stack->lock);

    // Destroy.
    pthread_mutex_destroy(&stack->lock);
    free(stack);
    return EXIT_SUCCESS;
}

int wf_repo_empty(wf_repo* stack){
    pthread_mutex_lock(&stack->lock);
    int numEntries = stack->size;
    pthread_mutex_unlock(&stack->lock);
    return numEntries == 0 ? 1 : 0; // returns 1 if stack empty
}



void wf_repo_push(wf_repo* stack, wf_table* toAddTable){
    pthread_mutex_lock(&stack->lock);
    // Adds to the front of the linkedList.
    toAddTable->next = stack->table;
    stack->table = toAddTable; // if first thing, stack->table initally was NULL
    ++stack->size;
    pthread_mutex_unlock(&stack->lock);
}

wf_table* wf_repo_pop(wf_repo* stack){
    // Returns null if stack is empty.
    pthread_mutex_lock(&stack->lock);
    if(stack->size == 0){
        pthread_mutex_unlock(&stack->lock);
        return NULL;
    }

    // result->next will not be NULL->next bcuz stack isn't empty.
    // if this happens, check wf_repo_push
    wf_table* result = stack->table;
    stack->table = result->next;
    --stack->size;

    pthread_mutex_unlock(&stack->lock);
    return result;
}