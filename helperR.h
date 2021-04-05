struct node{
    char* value;
    struct node* next;
}; typedef struct node Node;


/* The only item we can dequeue from a Directory Queue is a directory
    Parameter dequeuedItem refers to the name of the directory dequeued.
    Parameter files refers to the arraylist of files
    Parameter dirs refers to the arraylist of directories.
    Parameter fileSuffix such as ".txt" need for comparison of directory entries that are files.
    Return: we populate files and dirs by having a pointer to them, nothing returned.
*/
void directoryFunction_r(char* dequeuedItem, Node* files, Node* dirs, char* fileSuffix);

/*  Generate file path relative to current working directory for deqeued item 
    Implicitly reentrant
*/
char* generateFilePath(char* directoryName, char* currPath);

/*  endsWithSuffix(".txt", "coronavirus.txt") returns 1.
    If fileName doesn't end with fileSuffix, returns 0
*/
int endsWithSuffix(char* fileSuffix, char* fileName);