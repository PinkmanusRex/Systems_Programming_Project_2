#include "jsd.h"
#include <stdlib.h>

jsd_entry *jsd_create(){
        jsd_entry *entry = malloc(sizeof(jsd_entry));
        if (!entry) {
                /** null for failure */
                return 0;
        }
        entry->file_1 = 0;
        entry->file_2 = 0;
        entry->total_words = 0;
        entry->jsd = 0;
        return entry;
}

jsd_entry **jsd_create_list(int len) {
        jsd_entry **list = malloc(sizeof(jsd_entry *) * len);
        if (!list) {
                /** null for error */
                return 0;
        }
        for (int i = 0; i < len; i += 1) {
                jsd_entry *entry = jsd_create();
                if (!entry) {
                        return 0;
                }
                list[i] = entry;
        }
        return list;
}

int jsd_destroy_entry(jsd_entry *entry) {
        free(entry);
        return EXIT_SUCCESS;
}

int jsd_destroy_list(jsd_entry **list, int len) {
        for (int i = 0; i < len; i += 1) {
                free(list[i]);
        }
        free(list);
        return EXIT_SUCCESS;
}

int jsd_comparator(const void *entry_1, const void *entry_2) {
        jsd_entry *jsd_entry_1 = *(jsd_entry **)entry_1;
        jsd_entry *jsd_entry_2 = *(jsd_entry **)entry_2;
        return jsd_entry_1->total_words - jsd_entry_2->total_words;
}
