#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "hashtable.h"

unsigned long hash_func(char *word){
        /** credit to djb2 algorithm by dan bernstein */
        unsigned long hash = 5381;
        int c = *word;
        while (c) {
                hash = ((hash << 5) + hash) + c;
                word++;
                c = *word;
        }
        return hash;
}

int hash_insert(hashtable *table, char *word) {
        unsigned long hash = hash_func(word);
        int idx = (int)(hash % table->no_rows);
        wf_item *row = (table->data)[idx];
        /** if the row is empty, then just put it as the head */
        if (!row) {
                wf_item *entry = malloc(sizeof(wf_item));
                if (!entry){
                        return EXIT_FAILURE;
                }
                entry->word = word;
                entry->count = 1;
                entry->freq = -1;
                entry->hashcode = hash;
                entry->next = 0;
                row = entry;
                table->no_words += 1;
                table->no_entries += 1;
        } else {
                wf_item *ptr = row;
                wf_item *prev = 0;
                while (ptr) {
                        if (strcmp(ptr->word, word)==0) {
                                ptr->count += 1;
                                /** word was found in the table, but we do not need to store the duplicate word in the heap */
                                free(word);
                                break;
                        } else {
                                prev = ptr;
                                ptr = ptr->next;
                        }
                }
                if (!ptr) {
                        wf_item *entry = malloc(sizeof(wf_item));
                        if (!entry){
                                return EXIT_FAILURE;
                        }
                        entry->word = word;
                        entry->count = 1;
                        entry->freq = -1;
                        entry->hashcode = hash;
                        entry->next = 0;
                        prev->next = entry;
                        table->no_entries += 1;
                }
                table->no_words += 1;
        }
        /** now check if we have to rehash due to exceeding y */
        double y_prime = (double)table->no_entries/table->no_rows;
        if (y_prime > table->y) {
                if (hash_rehash(table)==EXIT_FAILURE) {
                        return EXIT_FAILURE;
                }
        }
        return EXIT_SUCCESS;
}

int hash_rehash(hashtable *table){
        int new_no_rows = 2 * table->no_rows;
        wf_item **new_data = malloc(sizeof(wf_item *)*new_no_rows);
        if (!new_data){
                return EXIT_FAILURE;
        }
        for (int i = 0; i < new_no_rows; i += 1) {
                new_data[i] = 0;
        }
        for (int i = 0; i < table->no_rows; i += 1) {
                wf_item *row = (table->data)[i];
                if (!row) {
                        continue;
                } else {
                        wf_item *ptr = row;
                        wf_item *next_it = 0;
                        while (ptr) {
                                next_it = ptr->next;
                                int new_idx = ptr->hashcode % new_no_rows;
                                ptr->next = new_data[new_idx];
                                new_data[new_idx] = ptr;
                                ptr = next_it;
                        }
                }
        }
        /** free the old table since it won't be used anymore, and replace it with the new table */
        free(table->data);
        table->data = new_data;
        return EXIT_SUCCESS;
}

int hash_comp_freq(hashtable *table) {
        for (int i = 0; i < table->no_rows; i += 1) {
                wf_item *row = (table->data)[i];
                if (!row) {
                        continue;
                }
                else {
                        wf_item *ptr = row;
                        while (ptr) {
                                ptr->freq = (double)ptr->count/table->no_words;
                                ptr = ptr->next;
                        }
                }
        }
        return EXIT_SUCCESS;
}
