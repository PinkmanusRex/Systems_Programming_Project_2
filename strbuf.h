 typedef struct {
    size_t length;
    size_t used;
    char *data;
} strbuf_t;

int sb_init(strbuf_t *, size_t);
void sb_destroy(strbuf_t *);
int sb_append(strbuf_t *, char); // put char at used, ++used (double if nec)
int sb_remove(strbuf_t *, char*); // removes last char -> used-1
int sb_insert(strbuf_t *, int, char);
int sb_reset(strbuf_t *); // used for project 1

/* additional one */
int sb_concat(strbuf_t *, char*); // append cannot append strings
