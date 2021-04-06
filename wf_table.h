#ifndef WF_TABLE_H
#define WF_TABLE_H
/**
 * hashcode: stored to save computations during hash_rehash
 */
typedef struct wf_item{
        char *word;
        int count;
        double freq;
        unsigned long hashcode;
        struct wf_item *next;
} wf_item;

typedef struct wf_table {
        char *file_name;
        int no_words;
        int no_entries;
        int no_rows;
        /** y used to decide when to rehash, in an attempt to maintain O(y) lookup time */
        double y;
        wf_item **data;
        wf_item **list;
        struct wf_table *next;
} wf_table;

/**
 * takes in a string makes a hashcode for it
 * RETURN: the hashcode generated from the string
 */
unsigned long hash_func(char *);

/**
 * takes in a hashtable and the word to be inserted
 * RETURN: EXIT_FAILURE if malloc failed, else EXIT_SUCCESS;
 * */
int hash_insert(wf_table *, char *);

/**
 * takes in a hashtable and will attempt to rehash, moving entries into a larger 2x row size array
 * returns EXIT_FAILURE if malloc failed
 */
int hash_rehash(wf_table *);

/**
 * only use when you no longer want a hashtable at all. hash_rehash will do the job of freeing the space of the old hashtable without destroying the entries
 * note, it will not only destroy all entries and the hashtable itself, but also all the strings in the hashtable as well
 */
int hash_destroy(wf_table *);

/**
 * actually computes the frequencies for words, which can only be done once we are finished adding words in
 */
int hash_comp_freq(wf_table *);

/** creates the hashtable with a filename, a y value, and a number of starting rows */
/** RETURN: either a hashtable or NULL on the condition of malloc failure */
wf_table *hash_create_table(char *, int, double);

/** look for the word in the hashtable. return frequency of word. returns 0 if not found */
double hash_get(wf_table *, char *);

/** will create a lexicographically sorted list for use in the later analysis phase */
int hash_lexical_list(wf_table *);

int wf_item_comparator(const void *, const void *);
#endif
