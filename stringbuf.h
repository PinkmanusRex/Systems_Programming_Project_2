#ifndef STRINGBUF_H
#define STRINGBUF_H
typedef struct stringbuf {
        int length;
        int used;
        char *data;
} stringbuf;

stringbuf *sb_init(int);
void sb_destroy(stringbuf *);
int sb_append(stringbuf *, char);
char *sb_get_lower_word(stringbuf *);
#endif
