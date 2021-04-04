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

typedef struct hashtable {
        char *file_name;
        int no_words;
        int no_entries;
        int no_rows;
        /** y used to decide when to rehash, in an attempt to maintain O(y) lookup time */
        double y;
        wf_item **data;
} hashtable;

/**
 * takes in a string makes a hashcode for it
 */
unsigned long hash_func(char *);

/**
 * takes in a hashtable and the word to be inserted
 * */
int hash_insert(hashtable *, char *);

/**
 * takes in a hashtable and will attempt to rehash, moving entries into a larger 2x row size array
 * returns EXIT_FAILURE if malloc failed
 */
int hash_rehash(hashtable *);

/**
 * only use when you no longer want a hashtable at all. hash_rehash will do the job of freeing the space of the old hashtable without destroying the entries
 * note, it will not only destroy all entries and the hashtable itself, but also all the strings in the hashtable as well
 */
int hash_destroy(hashtable *);

/**
 * create a hashtable, initializing all fields to 0 except for filename, with some length no_rows and some double y
 */
hashtable *hash_create(char*, int, double);

/**
 * actually computes the frequencies for words, which can only be done once we are finished adding words in
 */
int hash_comp_freq(hashtable *);
