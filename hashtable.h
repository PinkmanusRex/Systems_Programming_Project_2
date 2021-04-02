/**
 * hashcode: stored to save computations during hash_rehash
 */
typedef struct wf_item{
        char *word;
        int count;
        int freq;
        int hashcode;
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
 * takes in a string and the length of the string and makes a hashcode for it
 */
int hash_func(char *, int);

/**
 * takes in a hashtable and the word to be inserted, alongside an int for use with hash_func
 */
int hash_insert(hashtable *, char *, int);

/**
 * takes in a hashtable and will attempt to rehash, moving entries into a larger 2x row size array
 * returns EXIT_FAILURE if malloc failed
 */
int hash_rehash(hashtable *);

/**
 * only use when you no longer want a hashtable at all. hash_rehash will do the job of freeing the space of the old hashtable without destroying the entries
 */
int hash_destroy(hashtable *);
