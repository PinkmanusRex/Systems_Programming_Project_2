enum word_states {wEmpty, wIncomplete, wComplete};

/** 
 * takes in an int for the file descriptor, a hashtable to fill, and a stringbuf for assistance in storing and parsing tokens
 */
int tokenize(int, hashtable *, stringbuf *);
