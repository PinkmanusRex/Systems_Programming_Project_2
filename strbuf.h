 typedef struct {
    size_t length;
    size_t used;
    char *data;
} strbuf_t;

int sb_initk(strbuf_t *, size_t);
void sb_destroyk(strbuf_t *);
int sb_appendk(strbuf_t *, char); // put char at used, ++used (double if nec)
int sb_removek(strbuf_t *, char*); // removes last char -> used-1
int sb_insertk(strbuf_t *, int, char);
int sb_resetk(strbuf_t *); // used for project 1

/* additional one */
int sb_concatk(strbuf_t *, char*); // append cannot append strings
