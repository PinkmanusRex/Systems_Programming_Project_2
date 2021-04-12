#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include "wf_repo.h"
#include <stdlib.h>

wf_repo* wf_repo_create(){
    wf_repo* stack = (wf_repo *) malloc(sizeof(wf_repo));
    stack->size = 0;    // # elements in stack 0
    stack->table = NULL; // of type wf_table
    int mutex_status = pthread_mutex_init(&stack->lock, NULL);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return stack;
}

int wf_repo_destroy(wf_repo* stack){
    // Check if empty, if not then do not delete.
    int mutex_status = pthread_mutex_lock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    if(stack->size != 0){
    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return EXIT_FAILURE;
    }
    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    // Destroy.
    mutex_status = pthread_mutex_destroy(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    free(stack);
    return EXIT_SUCCESS;
}

/** purpose is to clear out the stack at the end, since it is necessary to keep those items 
 * in the linked-list for the entirety of the analysis phase, meaning that the stack will not 
 * be empty at the end and thus wf_repo_destroy will not work */
int wf_repo_clear(wf_repo *stack) {
    int mutex_status = pthread_mutex_lock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    wf_table *ptr = stack->table;
    wf_table *next_it = 0;
    while (ptr) {
        next_it = ptr->next;
        hash_destroy(ptr);
        ptr = next_it;
    }
    stack->size = 0;
    stack->table = 0;
    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int wf_repo_empty(wf_repo* stack){
    int mutex_status = pthread_mutex_lock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    int numEntries = stack->size;
    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return numEntries == 0 ? 1 : 0; // returns 1 if stack empty
}

void wf_repo_push(wf_repo* stack, wf_table* toAddTable){
    int mutex_status = pthread_mutex_lock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    // Adds to the front of the linkedList.
    toAddTable->next = stack->table;
    stack->table = toAddTable; // if first thing, stack->table initally was NULL
    stack->size += 1;
    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
}

wf_table* wf_repo_pop(wf_repo* stack){
    // Returns null if stack is empty.
    int mutex_status = pthread_mutex_lock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    if(stack->size == 0){
    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return NULL;
    }

    // result->next will not be NULL->next bcuz stack isn't empty.
    // if this happens, check wf_repo_push
    wf_table* result = stack->table;
    stack->table = result->next;
    stack->size -= 1;

    mutex_status = pthread_mutex_unlock(&stack->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return result;
}
