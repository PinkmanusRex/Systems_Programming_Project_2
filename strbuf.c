#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "strbuf.h"

#ifndef DEBUG
#define DEBUG 0
#endif

int sb_reset(strbuf_t *sb){
    sb->used = 0;
    sb->data[sb->used] = '\0';
    return 0;
}

int sb_init(strbuf_t *sb, size_t length)
{
    if(length < 1){
        return 1;
    }
    
    sb->data = malloc(sizeof(char) * length);
    if (!sb->data) return 1;

    sb->length = length;
    sb->used   = 0;
    sb->data[sb->used] = '\0';

    return 0;
}

int sb_remove(strbuf_t *sb, char* item){
    // if nothing to remove, can't return anything
    if(sb->used == 0)
        return 1;
    
    --sb->used;
    if (item)
        *item = sb->data[sb->used];

    sb->data[sb->used] = '\0';
    return 0;
}

void sb_destroy(strbuf_t *sb)
{
    free(sb->data);
}

int sb_append(strbuf_t *sb, char item){
    if((sb->used+1) == sb->length){
        size_t size = sb->length * 2;
        char *p = realloc(sb->data, sizeof(char)* size);
        if(!p) return 1;
        // successful
        sb->data = p;
        sb->length = size;

        if (DEBUG)
            printf("Increased size to %lu\n", size);
    }

    sb->data[sb->used] = item;
    ++sb->used;
    sb->data[sb->used] = '\0';
    return 0;
}

/*
    sb_concat adds the string str to the end of the string held in sb. 
    Assume that str is a null-terminated C string. 
    Return 0 if successful, and 1 otherwise.
*/
int sb_concat(strbuf_t *sb, char *str){
    if((sb->used + strlen(str)) >= sb->length){
        size_t size = ((sb->used + strlen(str) + 1) > 2 * sb->length) ? (sb->used + strlen(str) + 1) : 2*sb->length;
        char *p = realloc(sb->data, sizeof(char)* size);
        if(!p) return 1;
        // successful
        sb->data = p;
        sb->length = size;
        //if (DEBUG) idk what this is for
        //    printf("Increased size to %lu\n", size);
    }
    
    int count = 0;
    while(count < strlen(str)){
        sb->data[sb->used+count] = *(str+count);
        ++count;
    }
    sb->data[sb->used+count] = '\0';
    sb->used += count;
    return 0;
}

int sb_insert(strbuf_t *sb, int index, char item){
    if(index <= sb->used){
        ++sb->used;
        if(sb->used == sb->length){
            size_t size = 2*sb->length;
            char *p = realloc(sb->data, sizeof(char)* size);
            if(!p) return 1;
            // successful
            sb->data = p;
            sb->length = size;
        } // we have enough space to shift everything over
    
        sb->data[sb->used] = '\0';
        // start from index, shift everything over until you get to null term
        char temp;
        for(int i=index; i<sb->used; i++){
            temp = sb->data[i];
            sb->data[i] = item;
            item = temp;
        }
        
        return 0;
    }
    else{ // we know that index > used
        if((index+1) >= (2*sb->length)){
            size_t size = index+2;
            char *p = realloc(sb->data, sizeof(char)* size);
            if(!p) return 1;
            // successful
            sb->data = p;
            sb->length = size;
            sb->data[index] = item;
            sb->data[index+1] = '\0';
            sb->used = index+1;
            return 0;
        } // we know that index+1 is less than 2*sb->length
        // so it will fit between used and length
        // need to check if enough length to cover, if not double
        else{
            if(index+1 >= sb->length){
                size_t size = 2*sb->length;
                char *p = realloc(sb->data, sizeof(char)* size);
                if(!p) return 1;
                // successful
                sb->data = p;
                sb->length = size;
                sb->data[index] = item;
                sb->data[index+1] = '\0';
                sb->used = index+1;
                return 0;
            }
            else{
                // index+1 is > used but less than length
                sb->data[index] = item;
                sb->data[index+1] = '\0';
                sb->used = index+1;
                return 0;
            }
        }
    }
}

/*
void dump(strbuf_t *sb){
    int i=0;
    printf("[%ld/%ld]\t", sb->used, sb->length);
    for(; i<sb->used; i++){
        printf("%c ", sb->data[i]);
    }
    printf("/0 "); ++i;
    for(; i<sb->length; i++){
        printf("_ ");
    }
    putchar('\n');
}
*/