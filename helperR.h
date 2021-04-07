#ifndef HELPERR_H
#define HELPERR_H
#include <stdlib.h>
/* Used for generateFilePath */
typedef struct {
    size_t length;
    size_t used;
    char *data;
} strbuf_t;

int sb_initk(strbuf_t *, size_t);
void sb_destroyk(strbuf_t *);
int sb_appendk(strbuf_t *, char); // put char at used, ++used (double if nec)
int sb_removek(strbuf_t *, char*); // removes last char -> used-1
int sb_resetk(strbuf_t *); // used for project 1

/* additional one */
int sb_concatk(strbuf_t *, char*); // append cannot append strings


/* Used for arraylists amortized concurrent opertaions within ddirectory routine */
typedef struct node{
    char* value;
    struct node* next;
} Node;

/* The only item we can dequeue from a Directory Queue is a directory
    Parameter dequeuedItem refers to the name of the directory dequeued.
    Parameter files refers to the arraylist of files
    Parameter dirs refers to the arraylist of directories.
    Parameter fileSuffix such as ".txt" need for comparison of directory entries that are files.
    Return: we populate files and dirs by having a pointer to them, nothing returned.
*/
int directoryFunction_r(char* dequeuedItem, Node* files, Node* dirs, char* fileSuffix);

/*  Generate file path relative to current working directory for deqeued item 
    Implicitly reentrant
*/
char* generateFilePath(char* directoryName, char* currPath);

/*  endsWithSuffix(".txt", "coronavirus.txt") returns 1.
    If fileName doesn't end with fileSuffix, returns 0
*/
int endsWithSuffix(char* fileSuffix, char* fileName);
#endif
