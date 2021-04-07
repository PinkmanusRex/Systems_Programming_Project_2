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

/* The only item we can dequeue from a Directory Queue is a directory
    Parameter dequeuedItem refers to the name of the directory dequeued.
    Parameter files refers to the arraylist of files
    Parameter dirs refers to the arraylist of directories.
    Parameter fileSuffix such as ".txt" need for comparison of directory entries that are files.
    Return: we populate files and dirs by having a pointer to them, nothing returned.
*/
int directoryFunction_r(char* dequeuedItem, Node* files, Node* dirs, char* fileSuffix){
    DIR* dirp = opendir(dequeuedItem);
    // Check if we have perms to read it.
    if(dirp == NULL){
        perror(dequeuedItem);
        return EXIT_FAILURE;
    }

    errno = 0;

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
        
        statReturn = stat(currentPath, &entryStat); // Variety of reasons -> man 2 stat
        if(statReturn == -1) {
            perror(currentPath);
            free(currentPath);
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
                continue;
            }
            close(inputFD);
            
            if(endsWithSuffix(fileSuffix, currentPath) == 0){
                free(currentPath);
                continue;
            }
            
            // If we can open it and has matching suffix then add it to files
            Node* temp = (Node *)malloc(sizeof(Node));
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
                continue;
            }
            closedir(temporaryCheckForPerms);
            

            // Add currentPath to dir linkedlist.
            Node* temp = (Node *)malloc(sizeof(Node));
            temp->value = dirs->value;
            temp->next = dirs->next;
            dirs->value = currentPath;
            dirs->next = temp;
        }
    }
    closedir(dirp);
    free(dequeuedItem);
    return EXIT_SUCCESS;
}

// 1 if true, 0 if false
int endsWithSuffix(char* fileSuffix, char* fileName){
    int suffixLen = strlen(fileSuffix);
    int fileNameLen = strlen(fileName);
    if(suffixLen > fileNameLen){
        return 0;
    }
    for(int i=0; i<suffixLen; ++i){
        if(fileSuffix[suffixLen-i-1] != fileName[fileNameLen-i-1]){
            return 0;
        }
    }

    return 1;
}

/*  Generate file path relative to current working directory for deqeued item 
    Implicitly reentrant
*/
char* generateFilePath(char* directoryName, char* currPath)
{
    strbuf_t path;
    sb_initk(&path, 10);
    sb_concatk(&path, directoryName);
    sb_appendk(&path, '/');
    sb_concatk(&path, currPath);

    char* returnString = malloc(sizeof(char)* (path.used+1));
    strcpy(returnString, path.data);
    
    sb_destroyk(&path);
    return returnString;
}

/* Used for generateFilepath */

int sb_resetk(strbuf_t *sb){
    sb->used = 0;
    sb->data[sb->used] = '\0';
    return 0;
}

int sb_initk(strbuf_t *sb, size_t length)
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

int sb_removek(strbuf_t *sb, char* item){
    // if nothing to remove, can't return anything
    if(sb->used == 0)
        return 1;
    
    --sb->used;
    if (item)
        *item = sb->data[sb->used];

    sb->data[sb->used] = '\0';
    return 0;
}

void sb_destroyk(strbuf_t *sb)
{
    free(sb->data);
}

int sb_appendk(strbuf_t *sb, char item){
    if((sb->used+1) == sb->length){
        size_t size = sb->length * 2;
        char *p = realloc(sb->data, sizeof(char)* size);
        if(!p) return 1;
        // successful
        sb->data = p;
        sb->length = size;
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
int sb_concatk(strbuf_t *sb, char *str){
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

int sb_insertk(strbuf_t *sb, int index, char item){
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