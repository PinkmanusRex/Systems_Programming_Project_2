#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "analysis_threads.h"
#include "wf_table.h"
#include "debugger.h"
#include "extern_module.h"

void *analysis_thread_routine(void *arg){
        int mutex_status = 0;
#ifdef DEBUG
        fprintf(stdout, "Start of analysis thread\n");
#endif
        while (1) {
                mutex_status = pthread_mutex_lock(&analysis_mutex);
                if (mutex_status != 0) {
                        perror("ERROR!!!");
                        exit(EXIT_FAILURE);
                }
                /** no more analysis' to be done */
                if (jsd_comp_iter == jsd_total_comp) {
                        mutex_status = pthread_mutex_unlock(&analysis_mutex);
                        if (mutex_status != 0) {
                                perror("ERROR!!!");
                                exit(EXIT_FAILURE);
                        }
                        return 0;
                }
                /** save the two files to be analyzed to the local stack frame */
                wf_table *f1 = x;
                wf_table *f2 = y;
                unsigned int cur_idx = jsd_comp_iter;
                /** update the next available piece of work for whichever analysis thread will check next */
                jsd_comp_iter += 1;
                y = y->next;
                if (!y) {
                        x = x->next;
                        y = x->next;
                }
#ifdef DEBUG
                fprintf(stdout, "f1: %s, f2: %s, idx: %d\n", f1->file_name, f2->file_name, cur_idx);
#endif
                mutex_status = pthread_mutex_unlock(&analysis_mutex);
                if (mutex_status != 0) {
                        perror("ERROR!!!");
                        exit(EXIT_FAILURE);
                }
        }
        return 0;
}
