#ifndef SYNC_QUEUE_H
#define SYNC_QUEUE_H
#include <pthread.h>
typedef struct queue_node {
        char *name;
        struct queue_node *next;
} queue_node;

typedef struct sync_queue{
        int entries;
        queue_node *front;
        queue_node *rear;
        pthread_mutex_t lock;
} sync_queue;

/** 0 is false, 1 is true */
int sync_q_empty(sync_queue *);

/** 
 * enqueues an string
 * EXIT_SUCCESS for successful add, EXIT_FAILURE for failure to add */
int sync_q_add(sync_queue *, char *);

/** dequeues a node, but will return NULL if nothing inside */
queue_node *sync_q_remove(sync_queue *);

/** creates and returns a synchronized queue. returns NULL as error */
sync_queue *sync_q_create();

int sync_q_destroy(sync_queue *);
#endif
