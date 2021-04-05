struct node{
    char* value;
    struct node* next;
}; typedef struct node Node;

Node* insertNode(Node *head, char* val)
{
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->value = val;
    temp->next = head;
    return temp;
}

void directoryFunction_r(char* dequeuedItem, Node** files, Node** dirs, char* fileSuffix);
char* generateFilePath(char* directoryName, char* currPath);
int endsWithSuffix(char* fileName, char* fileSuffix);