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

char *sb_get_clean_word(stringbuf *list){
        int no_apost = 0;
        for (int i = 0; i < list->used; i += 1) {
                char c = (list->data)[i];
                if (c == '\'') {
                        no_apost += 1;
                }
        }
        /** plus 1 for the null terminator */
        int clean_word_len = (list->used) - no_apost + 1;
        char *clean_word = malloc(sizeof(char)*clean_word_len);
        if (!clean_word) {
                return 0;
        }
        int i = 0;
        int j = 0;
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
        /** set used to 0 so as to "clean out" the buffer */
        list->used = 0;
        clean_word[clean_word_len] = '\0';
        return clean_word;
}
