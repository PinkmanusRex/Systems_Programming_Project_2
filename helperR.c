#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <dirent.h>
#include "helperR.h"
#include "extern_module.h"

#define TRUE 1
#define FALSE 0

/* The only item we can dequeue from a Directory Queue is a directory
    Parameter dequeuedItem refers to the name of the directory dequeued.
    Parameter files refers to the arraylist of files
    Parameter dirs refers to the arraylist of directories.
    Parameter fileSuffix such as ".txt" need for comparison of directory entries that are files.
    Return: we populate files and dirs by having a pointer to them, nothing returned.
*/
int directoryFunction_r(char* dequeuedItem, Node* files, Node* dirs, char* fileSuffix){
    int err_flag = 0;
    DIR* dirp = opendir(dequeuedItem);
    // Check if we have perms to read it.
    if(dirp == NULL){
        perror(dequeuedItem);
        return EXIT_FAILURE;
    }

    struct dirent* entry; // This structure may be statically allocated. Don't attempt to free it. man 3 readdir
    struct stat entryStat; // temporary stat struct for each entry read from directory stream in loop.
    int statReturn;

    while((entry = readdir(dirp))!= NULL){
        // EXCEPTION: Any entry in a directory whose name begins with a period is ignored.
        if(strncmp(".", entry->d_name, strlen(".")) == 0){
            continue;
        }

        // Relative path fully formed.
        char* currentPath = generateFilePath(dequeuedItem, entry->d_name);

#ifdef DEBUG
        fprintf(stdout, "currentPath: %s\n", currentPath);
#endif
    
        statReturn = stat(currentPath, &entryStat); // Variety of reasons -> man 2 stat
        if(statReturn == -1) {
            perror(currentPath);
            free(currentPath);
            err_flag = 1;
            continue;
        }

        // Check whether file or directory and whether each has open perms. Then add to their respective linked lists.
        /* No permissions required on the file itself, but are required on all directories in pathname that lead to file */
        if(S_ISREG(entryStat.st_mode)){
            // If we can't open it continue forward.
            int inputFD = open(currentPath, O_RDONLY);
            if(inputFD == -1){
                perror(currentPath);
                free(currentPath);
                err_flag = 1;
                continue;
            }
            close(inputFD);
        
            if(endsWithSuffix(fileSuffix, currentPath) == FALSE) {
                free(currentPath);
                continue;
            }
        
            // If we can open it and has matching suffix then add it to files
            Node* temp = (Node *)malloc(sizeof(Node));
            if (!temp) {
                perror("helperR: directoryFunction - Node malloc failure!");
                exit(EXIT_FAILURE);
            }
            temp->value = files->value;
            temp->next = files->next;
            files->value = currentPath;
            files->next = temp;
        }
        else if(S_ISDIR(entryStat.st_mode)){
            // If it has read perms then add it.
            DIR* temporaryCheckForPerms = opendir(currentPath);
            if(temporaryCheckForPerms == NULL){
                perror(currentPath);
                free(currentPath);
                err_flag = 1;
                continue;
            }
            closedir(temporaryCheckForPerms);
        
            // Add currentPath to dir linkedlist.
            Node* temp = (Node *)malloc(sizeof(Node));
            if (!temp) {
                perror("helperR: directoryFunction - Node malloc failure!");
                exit(EXIT_FAILURE);
            }
            temp->value = dirs->value;
            temp->next = dirs->next;
            dirs->value = currentPath;
            dirs->next = temp;
        }
    }
    closedir(dirp);
    free(dequeuedItem);
    if (err_flag) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// 1 if true, 0 if false
int endsWithSuffix(char* fileSuffix, char* fileName){
    int suffixLen = strlen(fileSuffix);
    int fileNameLen = strlen(fileName);
    if(suffixLen > fileNameLen){
        return FALSE;
    }
    for(int i=0; i<suffixLen; ++i){
        if(fileSuffix[suffixLen-i-1] != fileName[fileNameLen-i-1]){
            return FALSE;
        }
    }
    return TRUE;
}

/*  Generate file path relative to current working directory for deqeued item 
    Implicitly reentrant
*/
char* generateFilePath(char* directoryName, char* currPath){
    strbuf_t path;
    sb_initk(&path, 10);
    sb_concatk(&path, directoryName);
    sb_appendk(&path, '/');
    sb_concatk(&path, currPath);

    char* returnString = malloc(sizeof(char)* (path.used+1));
    if (!returnString) {
        perror("helperR: generateFilePath - malloc failure!");
        exit(EXIT_FAILURE);
    }
    strcpy(returnString, path.data);
    
    sb_destroyk(&path);
    return returnString;
}

/* Used for generateFilepath */

int sb_resetk(strbuf_t *sb){
    sb->used = 0;
    sb->data[sb->used] = '\0';
    return EXIT_SUCCESS;
}

int sb_initk(strbuf_t *sb, size_t length)
{
    if(length < 1){
        return EXIT_FAILURE;
    }
    
    sb->data = malloc(sizeof(char) * length);
    if (!sb->data) {
        perror("helperR: sb_initk - malloc failure!");
        exit(EXIT_FAILURE);
    }
    sb->length = length;
    sb->used   = 0;
    sb->data[sb->used] = '\0';

    return EXIT_SUCCESS;
}

int sb_removek(strbuf_t *sb, char* item){
    // if nothing to remove, can't return anything
    if(sb->used == 0) return EXIT_FAILURE;
    
    --sb->used;
    if (item)
    *item = sb->data[sb->used];

    sb->data[sb->used] = '\0';
    return EXIT_SUCCESS;
}

void sb_destroyk(strbuf_t *sb)
{
    free(sb->data);
}

int sb_appendk(strbuf_t *sb, char item){
    if((sb->used+1) == sb->length){
        size_t size = sb->length * 2;
        char *p = realloc(sb->data, sizeof(char)* size);
        if(!p) {
            perror("helperR: sb_appendk - realloc failure!");
            exit(EXIT_FAILURE);
        }
        // successful
        sb->data = p;
        sb->length = size;
    }

    sb->data[sb->used] = item;
    ++sb->used;
    sb->data[sb->used] = '\0';
    return EXIT_SUCCESS;
}

/*
    sb_concat adds the string str to the end of the string held in sb. 
    Assume that str is a null-terminated C string. 
*/
int sb_concatk(strbuf_t *sb, char *str){
    if((sb->used + strlen(str)) >= sb->length){
        size_t size = ((sb->used + strlen(str) + 1) > 2 * sb->length) ? (sb->used + strlen(str) + 1) : 2*sb->length;
        char *p = realloc(sb->data, sizeof(char)* size);
        if(!p) {
            perror("helperR: sb_concatk - realloc failure!");
            exit(EXIT_FAILURE);
        }
        // successful
        sb->data = p;
        sb->length = size;
    }
    
    int count = 0;
    while(count < strlen(str)){
        sb->data[sb->used+count] = *(str+count);
        ++count;
    }
    sb->data[sb->used+count] = '\0';
    sb->used += count;
    return EXIT_SUCCESS;
}

int sb_insertk(strbuf_t *sb, int index, char item){
    if(index <= sb->used){
        ++sb->used;
        if(sb->used == sb->length){
            size_t size = 2*sb->length;
            char *p = realloc(sb->data, sizeof(char)* size);
            if(!p) {
                perror("helperR: sb_insertk - realloc failure!");
                exit(EXIT_FAILURE);
            }
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
        return EXIT_SUCCESS;
    }
    else{ // we know that index > used
        if((index+1) >= (2*sb->length)){
            size_t size = index+2;
            char *p = realloc(sb->data, sizeof(char)* size);
            if(!p) {
                perror("helperR: sb_insertk - realloc failure!");
                exit(EXIT_FAILURE);
            }
            // successful
            sb->data = p;
            sb->length = size;
            sb->data[index] = item;
            sb->data[index+1] = '\0';
            sb->used = index+1;
            return EXIT_SUCCESS;
        } // we know that index+1 is less than 2*sb->length
        // so it will fit between used and length
        // need to check if enough length to cover, if not double
        else{
            if(index+1 >= sb->length){
                size_t size = 2*sb->length;
                char *p = realloc(sb->data, sizeof(char)* size);
                if(!p) {
                    perror("helperR: sb_insertk - realloc failure!");
                    exit(EXIT_FAILURE);
                }
                // successful
                sb->data = p;
                sb->length = size;
                sb->data[index] = item;
                sb->data[index+1] = '\0';
                sb->used = index+1;
                return EXIT_SUCCESS;
            }
            else{
                // index+1 is > used but less than length
                sb->data[index] = item;
                sb->data[index+1] = '\0';
                sb->used = index+1;
                return EXIT_SUCCESS;
            }
        }
    }
}
