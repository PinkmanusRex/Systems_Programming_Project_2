#ifndef JSD_H
#define JSD_H
/** struct to hold the jsd data */
typedef struct jsd_entry {
        char *file_1;
        char *file_2;
        int total_words;
        double jsd;
}jsd_entry;

/** creates an "empty" jsd_entry */
jsd_entry *jsd_create();

/** creates a list of jsd_entry of specified length, all indices holding pointers to a jsd_entry */
jsd_entry **jsd_create_list(int);

/** passes every pointer in the list to jsd_destroy_entry before destroying the list itself. Will not free the strings, which is a task done by the hash_destroy method */
int jsd_destroy_list(jsd_entry **, int);

int jsd_destroy_entry(jsd_entry *);

int jsd_comparator(const void *, const void *);
#endif
