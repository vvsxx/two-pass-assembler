#include "header.h"

/* the function is used to catch errors when opening a file */
FILE *openFile(char *fileName, char *mode) {
    FILE *file;
    file = fopen(fileName, mode);
    if (file == NULL) {
        if (mode[0] == 'w')
            fprintf(stderr, "unable to create file %s\n", fileName);
        else if (errno == EACCES)
            fprintf(stderr, "Access error\n");
        else if (errno == ENOENT)
            fprintf(stderr, "File \"%s\" not found\n", fileName);
        else
            fprintf(stderr, "Error opening file %s\n", fileName);

        exit(EXIT_FAILURE);
    }
    return file;
}

/* memory allocation with error handling */
void *safeMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stdout, "CRITICAL ERROR: Allocating memory failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}


/* converts a decimal value to binary and stores it in "array_size" bits in the "binary" array  */
void decimalToBinary(int decimal, int *binary, int array_size) {
    int i, tmp = decimal;
    resetBits(binary, array_size);
    i = 0;
    /* positive value case */
    while (tmp && i < array_size) {
        binary[i] = tmp % 2;
        i++;
        tmp /= 2;
    }
    /* negative value case */
    if (decimal < 0) {
        /* invert all digits */
        for (i = 0; i < array_size; i++) {
            binary[i] = binary[i] == 0 ? 1 : 0;
        }
        i = 0;
        /* add 001 */
        while (binary[i] == 1 && i < array_size) {
            binary[i] = 0;
            i--;
        }
        binary[i] = 1;
    }
}

/* converts the binary value to a base 4 encrypted value and writes it to the "result" array */
void binaryToEncrypted4(const int *binary, char *result) {
    int i, j, res = 6;
    int base4[4][2] = {{0, 0},
                       {0, 1},
                       {1, 0},
                       {1, 1}};

    int secure[] = {'*', '#', '%', '!'};

    result[7] = '\0';
    for (i = 0; i < WORD_L; i += 2) {
        for (j = 0; j < 4; ++j) {
            if (base4[j][0] == binary[i + 1] && base4[j][1] == binary[i]) {
                result[res] = (char) secure[j];
                res--;
                break;
            }
        }
    }
}

/* sets array values to 0 */
void resetBits(int *arr, int size) {
    int i;
    for (i = 0; i < size; ++i)
        arr[i] = 0;
}


/* differs from the standard strdup() in that it uses safeMalloc */
char *strDuplicate(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = safeMalloc(len);
    strcpy(dst, src);
    return dst;
}

/* deletes white spaces and removes newline character */
char *deleteWhiteSpaces(char *token) {
    if (token[strlen(token) - 1] == '\n')
        token[strlen(token) - 1] = '\0';

    while (token[0] == ' ' || token[0] == '\t')
        token++;
    while (token[strlen(token) - 1] == ' ' || token[strlen(token) - 1] == '\t')
        token[strlen(token) - 1] = '\0';
    return token;
}

/* returns non zero value in case if token is number */
int isNumber(const char *token) {
    if (token[0] == '-' || token[0] == '+')
        return isdigit(token[1]);
    return isdigit(token[0]);
}

/* encrypts binary value to encrypted base 4 value */
void cryptWords(word *wrd) {
    word *tmp = wrd;
    while (tmp != NULL) {
        binaryToEncrypted4(tmp->binary, tmp->secure4);
        tmp = tmp->next;
    }
}

/* saves the memory address where the symbol that was declared as external was used
 * for each new address allocates additional space in the symbol.addresses array */
void addAddress(int **arr, int *size, int address) {
    int newSize = (*size) + 1;
    int *tmp = realloc(*arr, newSize * sizeof(int));
    if (tmp == NULL) {
        fprintf(stdout, "CRITICAL ERROR: Allocating memory failed\n");
        exit(EXIT_FAILURE);
    }
    *arr = tmp;
    (*arr)[newSize - 1] = address;
    *size = newSize;
}

/* calls functions to write .ent, .ext, & .obj files */
void writeFiles(list *symbols, mem_img *img, char *filename) {
    createEntFile(symbols, filename);
    writeObjFile(img, filename);
}

/* writes .obj file */
void writeObjFile(mem_img *img, char *filename) {
    FILE *objFile;
    char *newName;
    int IC;
    word *tmp;
    newName = safeMalloc(sizeof(filename) + 4); /* +.ob + \0 */
    strcpy(newName, filename);
    strcat(newName, ".ob\0");
    objFile = openFile(newName, "w");
    IC = img->IC - FIRST_ADDRESS + 1; /* +1 because addressing starts from 0 */
    fprintf(objFile, "%d %d\n", IC, img->DC);
    tmp = img->code_h;
    while (tmp != NULL) {
        fprintf(objFile, "%.4d %s\n", tmp->address, tmp->secure4);
        tmp = tmp->next;
    }
    tmp = img->data_h;
    while (tmp != NULL) {
        fprintf(objFile, "%.4d %s\n", tmp->address, tmp->secure4);
        tmp = tmp->next;
    }
    free(newName);
}

/* write .ent & .ext file */
int createEntFile(list *labels, char *fileName) {
    int isCorrect = 1, i;
    list *head;
    char ent_fileName[strlen(fileName) + 5]; /* .ent + '\0' */
    char ext_fileName[strlen(fileName) + 5]; /* .ext + '\0' */
    FILE *ent = NULL, *ext = NULL;
    strcpy(ent_fileName, fileName);
    strcpy(ext_fileName, fileName);
    strcat(ent_fileName, ".ent\0");
    strcat(ext_fileName, ".ext\0");
    head = labels;
    while (head != NULL) {
        if (head->isExternal) {
            if (ext == NULL)
                ext = openFile(ext_fileName, "w");
            for (i = 0; i < head->addresses_size; ++i) {
                fprintf(ext, "%s\t%.4d\n", head->name, head->addresses[i]);
            }
        }
        if (head->isEntry) {
            if (strcmp(head->type, "entry") != 0) {
                if (ent == NULL)
                    ent = openFile(ent_fileName, "w");
                fprintf(ent, "%s\t%.4d\n", head->name, head->value);
            } else {
                printError(UNDEFINED_ENTRY, 0);
                isCorrect = 0;
            }
        }
        head = head->next;
    }

    if (ent != NULL)
        fclose(ent);
    if (ext != NULL)
        fclose(ext);
    return isCorrect;
}

int syntaxCheck(char *line, opcode_table *opcodes, int lineNum) {
    int opcode;
    char *lastChar, *p, *token;
    char buffer[strlen(line) + 1];
    char operands_line[strlen(line) + 1];
    char *tmp = buffer;
    SentenceType type;
    strcpy(buffer, line);
    lastChar = &tmp[strlen(tmp) - 1];
    line = deleteWhiteSpaces(line);
    if (line[0] == ';') /* skip comment line */
        return SUCCESS;
    if ((*lastChar) == ',')
        return EXTRANEOUS_TEXT;
    if (strlen(tmp) > LINE_LENGTH)
        return TOO_LONG_LINE;
    tmp = buffer;
    token = strtok(buffer, " \t");
    type = getSentence(opcodes, token, lineNum);
    if (token[strlen(tmp) - 1] == ',') {
        return ILLEGAL_COMMA;
    }
    if (type == LABEL) {
        p = &buffer[strlen(token) + 1]; /* move pointer to end of label declaration */
        p = deleteWhiteSpaces(p);
        token = strtok(NULL, " \t");
        if (token[0] == ',')
            return ILLEGAL_COMMA;
        type = getSentence(opcodes, token, lineNum);
        p = &p[strlen(token) + 1]; /* move pointer to end operator name */
    } else {
        p = &buffer[strlen(token) + 1]; /* move pointer to end operator name */
    }
    if (type == ENTRY || type == EXTERN || type == DEFINE || type == DATA || type == STRING) {
        tmp += strlen(token) + 1;
        if (tmp[0] == ',')
            return ILLEGAL_COMMA;
    } else if (type == INSTRUCTION) {
        opcode = getOpcode(opcodes, token);
        if (opcode < 0) /* error case */
            return opcode;
        if (opcodes->max_ops[opcode] == 0) { /* operator with no operands */
            token = strtok(NULL, " \t");
            if (token != NULL)
                return EXTRANEOUS_TEXT;
        } else if (opcodes->max_ops[opcode] == 1) { /* operator with 1 operand */
            token = strtok(NULL, " \t");
            if (token == NULL)
                return MISSING_OPERAND;
            if (token[0] == ',')
                return ILLEGAL_COMMA;
            token = strtok(NULL, " ,\t");
            if (token != NULL)
                return EXTRANEOUS_TEXT;
        } else if (opcodes->max_ops[opcode] == 2) { /* operator with two operands */
            strcpy(operands_line, p);
            token = strtok(NULL, " \t");
            if (token == NULL)
                return MISSING_OPERAND;
            if (token[0] == ',')
                return ILLEGAL_COMMA;
            p = &token[strlen(token) + 1]; /* pointer to last operand */
            p = deleteWhiteSpaces(p);
            if (token[strlen(token) - 1] == ',' && token[strlen(token) - 2] == ',')
                return MULTIPLE_CONS_COMMAS;
            if (token[strlen(token) - 1] != ',' && p[0] != ',') {
                token = strtok(operands_line, ",");
                p = &token[strlen(token)]+1;
                if (p[0] == ',')
                    return MULTIPLE_CONS_COMMAS;
                token = strtok(NULL, ",");
                if (token == NULL)
                    return MISSING_COMMA;
            }
            if (token[strlen(token) - 1] == ',' && p[0] == ',')
                return MULTIPLE_CONS_COMMAS;
            if (p[0] == ',')
                p++;
            p = deleteWhiteSpaces(p);
            if (p[0] == ',')
                return MULTIPLE_CONS_COMMAS;
            token = strtok(operands_line, " ,\t");
            token = strtok(NULL, " ,\t");
            token = strtok(NULL, " ,\t");
            if (token != NULL)
                return EXTRANEOUS_TEXT;
        }
    } else {
        return UNKNOWN_OPERATOR;
    }
    return SUCCESS;
}

/*
 * Checks if the provided name matches any saved word in the opcode table.
 * Receives the name to be checked and pointer to the opcode table containing saved words.
 * Returns an integer indicating the result of the check:
 *     - ILLEGAL_LABEL_NAME if the name matches a saved word in the opcode table.
 *     - SUCCESS if the name does not match any saved word.
 */
int isSavedWord(char *name, opcode_table *opcodes) {
    int i;
    for (i = 0; i < MAX_OPERATORS; ++i) {
        if (strcmp(name, opcodes->name[i]) == 0)
            return ILLEGAL_LABEL_NAME;
    }
    for (i = 0; i < MAX_REGISTERS; ++i) {
        if (strcmp(name, opcodes->registerNames[i]) == 0)
            return ILLEGAL_LABEL_NAME;
    }
    return SUCCESS;
}

/*
 * Checks if the provided name is a legal label name, uses isSavedWord function to check that name is not a saved word.
 * Receives the name to be checked and pointer to the opcode table containing saved words.
 * Returns SUCCESS or error code in case that name is illegal;
 */
int isLegalName(char *name, opcode_table *opcodes) {
    int i;
    if (strlen(name) > LABEL_LENGTH)
        return TOO_LONG_NAME;
    if (!isalpha(name[0]))
        return ILLEGAL_LABEL_NAME;
    for (i = 1; i < strlen(name); ++i) {
        if (!isalpha(name[i]) && !isdigit(name[i]))
            return ILLEGAL_LABEL_NAME;
    }
    return isSavedWord(name, opcodes);
}