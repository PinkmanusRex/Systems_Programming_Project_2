#ifndef TOKENIZE_H
#define TOKENIZE_H
#include "stringbuf.h"
#include "wf_table.h"

enum word_states {wEmpty, wIncomplete, wComplete};

/** 
 * takes in an int for the file descriptor, a hashtable to fill, and a stringbuf for assistance in storing and parsing tokens
 */
int tokenize(int, wf_table *, stringbuf *);
#endif
