#include "sync_queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

sync_queue *sync_q_create() {
    sync_queue *q = malloc(sizeof(sync_queue));
    q->entries = 0;
    q->front = 0;
    q->rear = 0;
    pthread_mutex_init(&q->lock, NULL);
    return q;
}

/** assumes that the queue has been completely emptied*/
int sync_q_destroy(sync_queue *q) {
    int mutex_status = 0;
    mutex_status = pthread_mutex_destroy(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    free(q);
    return EXIT_SUCCESS;
}

int sync_q_empty(sync_queue *q) {
    int mutex_status = 0;
    mutex_status = pthread_mutex_lock(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    int no_entries = q->entries;
    mutex_status = pthread_mutex_unlock(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    if (no_entries == 0) {
        return 1;
    } else {
        return 0;
    }
}

int sync_q_add(sync_queue *q, char *name) {
    int mutex_status = 0;
    mutex_status = pthread_mutex_lock(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    queue_node *entry = malloc(sizeof(queue_node));
    if (!entry) {
        mutex_status = pthread_mutex_unlock(&q->lock);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
        return EXIT_FAILURE;
    }
    entry->name = name;
    entry->next = 0;
    if (q->entries == 0) {
        q->front = entry;
        q->rear = entry;
    }
    else {
        q->rear->next = entry;
        q->rear = entry;
    }
    q->entries += 1;
    mutex_status = pthread_mutex_unlock(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

queue_node *sync_q_remove(sync_queue *q) {
    int mutex_status = 0;
    mutex_status = pthread_mutex_lock(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    if (q->entries == 0) {
        mutex_status = pthread_mutex_unlock(&q->lock);
        if (mutex_status != 0) {
            perror("ERROR!!!");
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    queue_node *entry = q->front;
    q->front = q->front->next;
    q->entries -= 1;
    /** removed the last node */
    if (q->entries == 0) {
        q->rear = 0;
    }
    mutex_status = pthread_mutex_unlock(&q->lock);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    return entry;
}
