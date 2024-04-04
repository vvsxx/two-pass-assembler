#include "header.h"


void freeWordList(struct word *head) {
    while (head != NULL) {
        word *temp = head;
        head = head->next;
        if (temp->binary != NULL)
            free(temp->binary);
//        if (temp->secure4 != NULL)
//            free(temp->secure4);

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

void freeList(void *node, DataType type) {
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

list * createSymbol(struct list *list, char *token, char *line, char *type){
    struct list *tmp;
    if (strcmp(type, "mdefine") == 0) {
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
    list->type = strDuplicate(type);
    if (strcmp(type, "mdefine") == 0) {
        token = strtok(NULL, "=");
        list->value = atoi(token);
    }

    return list;
}

/* searches for element, returns pointer if element found */
struct list *search_by_name(struct list *listHead, char *string) {
    struct list *current = listHead;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, string) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

struct macros_list *search_macros_by_name(struct macros_list *listHead, char *string) {
    struct macros_list *current = listHead;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, string) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}


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

word * createWordNode(struct word *node, int addr){
    word *tmp = node == NULL ? node : node->next;
    tmp = (struct word *) safeMalloc(sizeof(struct word));
    tmp->next = NULL;
    tmp->binary = (int*)safeMalloc(WORD_L * sizeof(int));
    resetBits(tmp->binary, WORD_L);
    tmp->address = addr;
    if (node != NULL)
        node->next = tmp;
    return tmp;
}