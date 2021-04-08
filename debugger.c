#include <stdio.h>
#include <stdlib.h>
#include "wf_repo.h"
#include "wf_table.h"
#include "debugger.h"

int debug_wf_repo_print(wf_repo *stack){
        if (!stack) {
                return EXIT_FAILURE;
        }
        fprintf(stdout, "Size of repo: %d\n", stack->size);
        wf_table *ptr = stack->table;
        while (ptr) {
                fprintf(stdout, "\tFileName: %s, No. Words: %d, No. Entries: %d\n", ptr->file_name, ptr->no_words, ptr->no_entries);
                for (int i = 0; i < ptr->no_entries; i += 1) {
                        fprintf(stdout, "\t\t%s, %d, %.18lf\n", ptr->list[i]->word, ptr->list[i]->count, ptr->list[i]->freq);
                }
                ptr = ptr->next;
        }
        return EXIT_SUCCESS;
}
