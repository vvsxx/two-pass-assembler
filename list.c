#include "header.h"


/* free memory */
void freeSymList(struct list *p){
    void *tmp;
    while (p != NULL) {
        tmp = p->next;
        free(p->name);
        free(p->type);
        free(p);
        p = tmp;
    }
}

void freeWordList(struct word *head) {
    while (head != NULL) {
        word *temp = head;
        head = head->next;
        if (temp->binary != NULL)
            free(temp->binary);
        if (temp->secure4 != NULL)
            free(temp->secure4);

        free(temp);
    }
}

void freeMacList(struct macros_list *head) {
    while (head != NULL) {
        struct macros_list *temp = head;
        head = head->next;
        free(temp->name);
        for (int i = 0; i < temp->lines; ++i) {
            free(temp->data[i]);
        }
        free(temp->data);
        free(temp);
    }
}

void freeList(void *node, ListType type) {
    void *tmp;
    struct list *sym_p;
    struct word *word_p;
    struct macros_list *mac_p;
    switch (type) {
        case SYMBOL_LIST:
            sym_p = node;
            freeSymList(sym_p);
            break;
        case MACROS_LIST:
            mac_p = node;
            freeMacList(mac_p);
            break;
        case WORD_LIST:
            word_p = node;
            freeWordList(word_p);
            break;
        default:
            printf("Unknown type\n");
            break;
    }
}

list * createSymbol(struct list *list, char *token, char *line, SentenceType type){
    if (type == DEFINE) {
        line = strstr(line, ".define");
        line += strlen(".define");
        token = strtok(line, "="); /* УБРАТЬ ЦИФРЫ */
    }
    if (list == NULL) {
        list = safeMalloc(sizeof(struct list));
    } else if (list->name != NULL){
        list->next = (struct list*) safeMalloc(sizeof(struct list));
        list = list->next;
    }
    list->next = NULL;
    token = deleteWhiteSpaces(token);

    list->name = strDuplicate(token);

    if (type == DEFINE) {
        token = strtok(NULL, "=");
        list->value = atoi(token);
        list->type = strDuplicate("mdefine");
    }
    list->isEntry = 0;
    list->isExternal = 0;
    if (type == ENTRY){
        list->type = strDuplicate("entry");
        list->isEntry = 1;
        list->ARE = ARE_RELOCATABLE;
    } else if (type == EXTERN){
        list->type = strDuplicate("extern");
        list->isExternal = 1;
        list->ARE = ARE_EXTERNAL;
        list->value = 0;
    } else if (type == DEFINE){
        list->ARE = ARE_ABSOLUTE;
    } else if (type == INSTRUCTION){
        list->ARE = ARE_RELOCATABLE;
    }
        return list;
}

/* searches for list element, returns pointer if element found */
struct list *getElementByName(struct list *listHead, char *string) {
    struct list *current = listHead;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, string) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}


struct macros_list *getMacroByName(struct macros_list *listHead, char *string) {
    struct macros_list *current = listHead;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, string) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}




word * createWordNode(struct word *node, int addr){
    word *tmp = node == NULL ? node : node->next;
    tmp = (struct word *) safeMalloc(sizeof(struct word));
    tmp->next = NULL;
    tmp->binary = (int*)safeMalloc(WORD_L * sizeof(int));
    tmp->secure4 = safeMalloc(8 * sizeof (int));
    resetBits(tmp->binary, WORD_L);
    tmp->address = addr;
    if (node != NULL)
        node->next = tmp;
    return tmp;
}