#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "wf_table.h"
#include "debugger.h"

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

int hash_insert(wf_table *table, char *word) {
    unsigned long hash = hash_func(word);
    int idx = (int)((hash) % table->no_rows);
    wf_item *row = table->data[idx];
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
        table->data[idx] = entry;
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

int hash_rehash(wf_table *table){
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
    table->no_rows = new_no_rows;
    table->data = new_data;
    return EXIT_SUCCESS;
}

/**
int hash_comp_freq(wf_table *table) {
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
*/

/** method based on the list portion of wf_table */
int hash_comp_freq(wf_table *table) {
    for (int i = 0; i < table->no_entries; i += 1) {
        wf_item *entry = table->list[i];
        entry->freq = (double)entry->count/(double)table->no_words;
    }
    return EXIT_SUCCESS;
}

/**
int hash_destroy(wf_table *table) {
    free(table->file_name);
    for (int i = 0; i < table->no_rows; i += 1) {
        wf_item *row = table->data[i];
        if (!row) {
            continue;
        }
        else {
            wf_item *ptr = row;
            wf_item *next_it = 0;
            while(ptr) {
                next_it = ptr->next;
                free(ptr->word);
                free(ptr);
                ptr = next_it;
            }
        }
    }
    free(table->list);
    free(table->data);
    free(table);
    return EXIT_SUCCESS;
}
*/

/** method based on assumption that the actual hashtable was freed up, and all is left is the list */
int hash_destroy(wf_table *table) {
    for (int i = 0; i < table->no_entries; i += 1) {
        wf_item *entry = table->list[i];
        free(entry->word);
        free(entry);
    }
    free(table->list);
    free(table->file_name);
    free(table);
    return EXIT_SUCCESS;
}

wf_table *hash_create_table(char *file_name, int no_rows, double y) {
    wf_table *table = malloc(sizeof(wf_table));
    if (!table) {
        return 0;
    }
    table->list = 0;
    table->no_rows = no_rows;
    table->file_name = file_name;
    table->no_words = 0;
    table->no_entries = 0;
    table->y = y;
    table->next = 0;
    table->data = malloc(sizeof(wf_item *) * no_rows);
    if (!table->data) {
        free(table);
        return 0;
    }
    for (int i = 0; i < no_rows; i += 1) {
        table->data[i] = 0;
    }
    return table;
}

double hash_get(wf_table *table, char *word) {
    unsigned long hash = hash_func(word);
    int idx = (int)(hash % table->no_rows);
    wf_item *row = table->data[idx];
    if (!row) {
        return 0;
    }
    else {
        wf_item *ptr = row;
        while (ptr) {
            if (strcmp(ptr->word, word)==0) {
                return ptr->freq;
            }
            ptr = ptr->next;
        }
        return 0;
    }
}

int hash_lexical_list(wf_table *table) {
    table->list = malloc(sizeof(wf_item *) * table->no_entries);
    if (!table->list) {
        return EXIT_FAILURE;
    }
    int j = 0;
    for (int i = 0; i < table->no_rows; i += 1) {
        wf_item *row = table->data[i];
        if (!row) {
            continue;
        }
        else {
            wf_item *ptr = row;
            while (ptr) {
                table->list[j] = ptr;
                j += 1;
                ptr = ptr->next;
            }
        }
    }
    qsort((void *)(table->list), table->no_entries, sizeof(wf_item *), wf_item_comparator);
    return EXIT_SUCCESS;
}

int wf_item_comparator(const void *wf_1_void, const void *wf_2_void) {
    wf_item *wf_1 = *(wf_item **) wf_1_void;
    wf_item *wf_2 = *(wf_item **) wf_2_void;
    return strcmp(wf_1->word, wf_2->word);
}
