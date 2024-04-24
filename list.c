#include "header.h"

/*
 * linked list processing functions
 */

/* Frees the memory allocated for a linked list of symbols */
void freeSymList(struct list *p) {
    struct list *tmp;
    while (p != NULL) {
        tmp = p->next;
        if (p->name != NULL) {
            free(p->name);
        }
        if (p->type != NULL) {
            free(p->type);
        }
        if (p->addresses != NULL) {
            free(p->addresses);
        }

        free(p);

        p = tmp;
    }
}

/* Frees the memory allocated for a linked list of words */
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
/* Frees the memory allocated for a linked list of macros */
void freeMacList(struct macros_list *head) {
    int i;
    struct macros_list *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp->name);
        for (i = 0; i <= temp->lines; ++i) {
            free(temp->data[i]);
        }
        free(temp->data);
        free(temp);
    }
}

/*
 * Calls to specific function to free memory allocated for a specific type of linked list.
 *   node: Pointer to the head of the linked list.
 *   type: Type of the linked list (SYMBOL_LIST, MACROS_LIST, WORD_LIST).
 */
void freeList(void *node, ListType type) {
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

/*
 * Creates a new symbol node and adds it to the symbol list.
 *   list: Pointer to the head of the symbol list.
 *   token: Name of the symbol.
 *   line: Line containing the symbol definition.
 *   type: Type of the symbol (DEFINE, ENTRY, EXTERN, INSTRUCTION).
 *  Returns pointer to the new node.
 */
list * createSymbol(struct list *list, char *token, char *line, SentenceType type, int lineNum){
    if (type == DEFINE) {
        line = strstr(line, ".define");
        line += strlen(".define");
        token = strtok(line, "=");
    }
    if (list == NULL) {
        list = safeMalloc(sizeof(struct list));
    } else if (list->name != NULL){
        list->next = (struct list*) safeMalloc(sizeof(struct list));
        list = list->next;
    }
    list->type = NULL;
    list->addresses = NULL;
    list->next = NULL;
    list->value = 0;
    token = deleteWhiteSpaces(token);

    list->name = strDuplicate(token);
    list->isEntry = 0;
    list->isExternal = 0;

    if (type == DEFINE) {
        if ((token = strtok(NULL, "=")) == NULL)
            return NULL;
        token = deleteWhiteSpaces(token);
        if (!isNumber(token))
            printError(NOT_AN_INTEGER, lineNum);
        list->value = atoi(token);
        list->type = strDuplicate("mdefine");
        list->ARE = ARE_ABSOLUTE;
    } else if (type == ENTRY){
        list->type = strDuplicate("entry");
        list->isEntry = 1;
        list->ARE = ARE_RELOCATABLE;
    } else if (type == EXTERN){
        list->type = strDuplicate("extern");
        list->isExternal = 1;
        list->ARE = ARE_EXTERNAL;
        list->value = 0;
    } else if (type == INSTRUCTION){
        list->ARE = ARE_RELOCATABLE;
    }
    list->addresses = NULL;
    list->addresses_size = 0;
        return list;
}

/*
 * Retrieves a node from a linked list by its name.
 *   listHead: Pointer to the head of the linked list.
 *   string: Name to search for.
 * Returns pointer to the node with the matching name, or NULL if not found.
 */
struct list *getElementByName(struct list *listHead, char *string) {
    struct list *current = listHead;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, string) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}

/*
 * Retrieves a macro node from a linked list by its name.
 *   listHead: Pointer to the head of the linked list.
 *   string: Name to search for.
 * Returns pointer to the macro node with the matching name, or NULL if not found.
 */
struct macros_list *getMacroByName(struct macros_list *listHead, char *string) {
    struct macros_list *current = listHead;
    while (current != NULL) {
        if (current->name != NULL && strcmp(current->name, string) == 0)
            return current;
        current = current->next;
    }
    return NULL;
}



/*
 * Creates a new word node and adds it to the linked list of words.
 *   node: Pointer to the previous word node.
 *   addr: Decimal address.
 *  Returns pointer to the new node.
 */
word * createWordNode(struct word *node, int addr){
    word *tmp;
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