#include <stdlib.h>
#include <string.h>
#include "stringbuf.h"
#include <ctype.h>

stringbuf *sb_init(int length){
        if (length < 10) {
                length = 10;
        }
        stringbuf *list = malloc(sizeof(stringbuf));
        if (!list) {
                return 0;
        }
        /** 0 out the fields for error check later */
        list->used = 0;
        list->length = 0;
        list->data = 0;

        list->data = malloc(sizeof(char)*length);
        if (!list->data) {
                return 0;
        }
        list->length = length;
        list->data[0] = '\0';
        list->used = 0;
        return list;
}

void sb_destroy(stringbuf *list) {
        free(list->data);
        free(list);
}

int sb_append(stringbuf *list, char item){
        if (list->used == list->length - 1){
                int new_len = 2*(list->length - 1) + 1;
                char *p = realloc(list->data, sizeof(char)*new_len);
                if (!p){
                        return EXIT_FAILURE;
                }
                list->length = new_len;
                list->data = p;
        }
        (list->data)[list->used] = item;
        list->used += 1;
        (list->data)[list->used] = '\0';
        return EXIT_SUCCESS;
}

char *sb_get_lower_word(stringbuf *list){
        char *lower_word = malloc(sizeof(char)*(list->used + 1));
        /** returns NULL */
        if (!lower_word) {
                return 0;
        }
        for (int i = 0; i < list->used; i += 1) {
                /** uppercase alphabet will be lowercased, all else is returned same as is */
                lower_word[i] = tolower((list->data)[i]);
        }
        lower_word[list->used] = '\0';
        list->used = 0;
        list->data[0] = '\0';
        return lower_word;
}
