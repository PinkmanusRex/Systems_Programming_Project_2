#include "jsd.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "debugger.h"
#include "wf_table.h"
#include "string.h"
#include "extern_module.h"

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
    return jsd_entry_2->total_words - jsd_entry_1->total_words;
}

double jsd_comp(wf_table *tab1, wf_table *tab2) {
#ifdef DEBUG
    int mutex_status = 0;
    mutex_status = pthread_mutex_lock(&jsd_sync_mutex);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
    fprintf(stdout, "Working on %s & %s\n", tab1->file_name, tab2->file_name);
#endif
    double kld_tab1 = 0;
    double kld_tab2 = 0;
    wf_item **tab1_list = tab1->list;
    wf_item **tab2_list = tab2->list;
    int tab1_len = tab1->no_entries;
    int tab2_len = tab2->no_entries;
    /** i tracks tab1_list, j tracks tab2_list */
    int i = 0;
    int j = 0;
    while (i < tab1_len && j < tab2_len) {
        int cmp = strcmp(tab1_list[i]->word, tab2_list[j]->word);
        double mean_freq = 0;
        if (cmp==0) {
            mean_freq = (0.5)*(tab1_list[i]->freq + tab2_list[j]->freq);
            kld_tab1 += tab1_list[i]->freq * log2(tab1_list[i]->freq/mean_freq);
            kld_tab2 += tab2_list[j]->freq * log2(tab2_list[j]->freq/mean_freq);
#ifdef DEBUG
            fprintf(stdout, "\t%s & %s: %s, mean_freq: %.18lf\n\tkld_tab1: %.18lf, kld_tab2: %.18lf\n", tab1->file_name, tab2->file_name, tab2_list[j]->word, mean_freq, kld_tab1, kld_tab2);
#endif
            i += 1;
            j += 1;
        }
        else if (cmp < 0) {
            mean_freq = (0.5)*(tab1_list[i]->freq);
            kld_tab1 += tab1_list[i]->freq * log2(tab1_list[i]->freq/mean_freq);
#ifdef DEBUG
            fprintf(stdout, "\t%s: %s, mean_freq: %.18lf\n\tkld_tab1: %.18lf\n", tab1->file_name, tab1_list[i]->word, mean_freq, kld_tab1);
#endif
            i += 1;
        }
        else {
            mean_freq = (0.5)*(tab2_list[j]->freq);
            kld_tab2 += tab2_list[j]->freq * log2(tab2_list[j]->freq/mean_freq);
#ifdef DEBUG
            fprintf(stdout, "\t%s: %s, mean_freq: %.18lf\n\tkld_tab2: %.18lf\n", tab2->file_name, tab2_list[j]->word, mean_freq, kld_tab2);
#endif
            j += 1;
        }
    }

    while (i < tab1_len) {
        double mean_freq = (0.5)*(tab1_list[i]->freq);
        kld_tab1 += tab1_list[i]->freq * log2(tab1_list[i]->freq/mean_freq);
#ifdef DEBUG
        fprintf(stdout, "\t%s: %s, mean_freq: %.18lf\n\tkld_tab1: %.18lf\n", tab1->file_name, tab1_list[i]->word, mean_freq, kld_tab1);
#endif
        i += 1;
    }

    while (j < tab2_len) {
        double mean_freq = (0.5)*(tab2_list[j]->freq);
        kld_tab2 += tab2_list[j]->freq * log2(tab2_list[j]->freq/mean_freq);
#ifdef DEBUG
        fprintf(stdout, "\t%s: %s, mean_freq: %.18lf\n\tkld_tab2: %.18lf\n", tab2->file_name, tab2_list[j]->word, mean_freq, kld_tab2);
#endif
        j += 1;
    }

    double jsd_res = sqrt((0.5)*kld_tab1 + (0.5)*kld_tab2);
#ifdef DEBUG
    fprintf(stdout, "\tjsd_res: %.18lf\n", jsd_res);
    mutex_status = pthread_mutex_unlock(&jsd_sync_mutex);
    if (mutex_status != 0) {
        perror("ERROR!!!");
        exit(EXIT_FAILURE);
    }
#endif
    return jsd_res;
}

int jsd_print_list(jsd_entry **list, int len) {
    for (int i = 0; i < len; i += 1) {
#ifdef DEBUG
        fprintf(stdout, "%d\t", list[i]->total_words);
#endif
        fprintf(stdout, "%lf %s %s\n", list[i]->jsd, list[i]->file_1, list[i]->file_2);
    }
    return EXIT_SUCCESS;
}
