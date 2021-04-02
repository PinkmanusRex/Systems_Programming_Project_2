#include <stdlib.h>
#include <string.h>
#include "stringbuf.h"
#include <ctype.h>



stringbuf *sb_init(size_t length){
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
                size_t new_len = 2*(list->length - 1) + 1;
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

char *sb_get_clean_word(stringbuf *list){
        size_t no_apost = 0;
        for (size_t i = 0; i < list->used; i += 1) {
                char c = (list->data)[i];
                if (c == '\'') {
                        no_apost += 1;
                }
        }
        /** plus 1 for the null terminator */
        size_t clean_word_len = (list->used) - no_apost + 1;
        char *clean_word = malloc(sizeof(char)*clean_word_len);
        if (!clean_word) {
                return 0;
        }
        size_t i = 0;
        size_t j = 0;
        while (i < list->used) {
                char c = (list->data)[i];
                if (c == '\'') {
                        i += 1;
                        continue;
                } else {
                        c = tolower(c);
                        clean_word[j] = c;
                        i += 1;
                        j += 1;
                }
        }
        clean_word[clean_word_len] = '\0';
        return clean_word;
}
