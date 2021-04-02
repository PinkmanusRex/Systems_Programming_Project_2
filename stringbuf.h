typedef struct {
        size_t length;
        size_t used;
        char *data;
} stringbuf;

stringbuf *sb_init(size_t);
void sb_destroy(stringbuf *);
int sb_append(stringbuf *, char);
