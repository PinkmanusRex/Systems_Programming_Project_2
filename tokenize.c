#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "wf_table.h"
#include "stringbuf.h"
#include "tokenize.h"
#include "debugger.h"

int tokenize(int fd, wf_table *table, stringbuf *list) {
        if (DEBUG) {
            fprintf(stdout, "The wf_table: %s, and it's row count is %d\n", table->file_name, table->no_rows);
        }
        enum word_states word_state = wEmpty;
        int bytesRead = 0;
        char inter_buf[50];
        bytesRead = read(fd, inter_buf, 50);
        while ((bytesRead > 0)) {
                int buf_read = 0;
                while (buf_read < bytesRead) {
                        if (word_state == wEmpty) { 
                                if (isalpha(inter_buf[buf_read]) || isdigit(inter_buf[buf_read]) || inter_buf[buf_read]=='-') {
                                        int status = sb_append(list, inter_buf[buf_read]);
                                        if (status==EXIT_FAILURE) {
                                                return EXIT_FAILURE;
                                        }
                                        word_state = wIncomplete;
                                }
                        } else {
                                if (isalpha(inter_buf[buf_read]) || isdigit(inter_buf[buf_read]) || inter_buf[buf_read]=='-') {
                                        int status = sb_append(list, inter_buf[buf_read]);
                                        if (status==EXIT_FAILURE) {
                                                return EXIT_FAILURE;
                                        }
                                }
                                else if (isspace(inter_buf[buf_read])) {
                                        word_state = wComplete;
                                }
                        }
                        if (word_state==wComplete) {
                                char *word = sb_get_lower_word(list);
                                if (DEBUG) {
                                        fprintf(stdout, "%s\n", word);
                                }
                                /** if word is null, that means malloc failed */
                                if (!word) {
                                        return EXIT_FAILURE;
                                }
                                word_state = wEmpty;
                                int status = hash_insert(table, word);
                                if (status==EXIT_FAILURE) {
                                        return EXIT_FAILURE;
                                }
                        }
                        buf_read += 1;
                }
                bytesRead = read(fd, inter_buf, 50);
        }
        if (word_state==wIncomplete) {
                char *word = sb_get_lower_word(list);
                if (DEBUG) {
                        fprintf(stdout, "%s\n", word);
                }
                if (!word) {
                        return EXIT_FAILURE;
                }
                int status = hash_insert(table, word);
                if (status==EXIT_FAILURE) {
                        return EXIT_FAILURE;
                }
        }
        if (hash_lexical_list(table)==EXIT_FAILURE) {
                return EXIT_FAILURE;
        }
        hash_comp_freq(table);
        free(table->data);
        if (DEBUG) {
            for (int i = 0; i < table->no_entries; i += 1) {
                fprintf(stdout, "entry[%d] = %s\n", i, table->list[i]->word);
            }
        }
        return EXIT_SUCCESS;
}
