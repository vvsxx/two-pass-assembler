#include "header.h"

/*
 * General purpose functions
 */

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
    }
    return file;
}

/*
 * Safely allocates memory of the given size using malloc.
 * A memory allocation error is considered critical and in this case the program terminates completely.
 */
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
    /* empty value */
    if (*token == '\0')
        return 0;

    /* first character must be digit ro + or - */
    if (!isdigit(*token) && *token != '+' && *token != '-')
        return 0;

    /* move to the next character in case of + or - */
    if (*token == '+' || *token == '-')
        token++;

    /* check that all characters are digits */
    while (*token != '\0') {
        if (!isdigit(*token))
            return 0;
        token++;
    }
    return 1;
}

/* calls functions to write .ent, .ext, & .ob files */
int writeFiles(list *symbols, mem_img *img, char *filename) {
    int isCorrect = SUCCESS;
    isCorrect = createEntFile(symbols, filename);
    if (isCorrect != SUCCESS)
        return isCorrect;
    isCorrect = writeObjFile(img, filename);
    return isCorrect;
}

/* writes .obj file */
int writeObjFile(mem_img *img, char *filename) {
    FILE *objFile;
    char *newName;
    int IC;
    word *tmp;
    newName = safeMalloc(strlen(filename) + 4); /* +.ob + \0 */
    strcpy(newName, filename);
    strcat(newName, ".ob\0");
    objFile = openFile(newName, "w");
    if (objFile == NULL) /* can't open object file */
        return INCORRECT;
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
    return  SUCCESS;
}

/* write .ent & .ext file */
int createEntFile(list *labels, char *fileName) {
    int isCorrect = SUCCESS, i;
    list *head;
    int nameSize = strlen(fileName) + 5;
    char *ent_fileName = safeMalloc(nameSize * sizeof (int)); /* .ent + '\0' */
    char *ext_fileName = safeMalloc(nameSize * sizeof (int)); /* .ext + '\0' */
    FILE *ent = NULL, *ext = NULL;
    strcpy(ent_fileName, fileName);
    strcpy(ext_fileName, fileName);
    strcat(ent_fileName, ".ent\0");
    strcat(ext_fileName, ".ext\0");
    head = labels;
    while (head != NULL) {
        if (head->isExternal) {
            if (ext == NULL) {
                ext = openFile(ext_fileName, "w");
                if (ext == NULL) /* can't open externals file */
                    return INCORRECT;
            }
            for (i = 0; i < head->addresses_size; ++i) {
                fprintf(ext, "%s\t%.4d\n", head->name, head->addresses[i]);
            }
        }
        if (head->isEntry) {
            if (strcmp(head->type, "entry") != 0) {
                if (ent == NULL) {
                    ent = openFile(ent_fileName, "w");
                    if (ent == NULL) /* can't open entries file */
                        return INCORRECT;
                }
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
    free(ent_fileName);
    free(ext_fileName);
    return isCorrect;
}


int syntaxCheck(char *line, op_table *opcodes) {
    int opcode, i;
    int errorCode = SUCCESS;
    char *p, *token, *operator = NULL, *label = NULL, *operands = NULL;
    int bufferSize = strlen(line) + 1;
    char *buffer = safeMalloc(bufferSize * sizeof (char));
    SentenceType type;

    if (line[0] == '\0') { /* skip empty line */
        free(buffer);
        return SUCCESS;
    }
    if (line[strlen(line) - 1] == '\n')
        line[strlen(line) - 1] = '\0';

    if (line[strlen(line)-1] == ',') {
        free(buffer);
        return ILLEGAL_COMMA;
    }
    strcpy(buffer, line);
    token = strtok(buffer, " \t");
    type = getSentence(opcodes, token);
    if (type == LABEL) { /* skip label */
        token = strtok(NULL, " \t");
        if (token != NULL){
            type = getSentence(opcodes, token);
        } else {
            errorCode = EMPTY_LABEL;
        }
    }
    if (type == ENTRY || type == EXTERN || type == DEFINE || type == DATA || type == STRING) {
        errorCode =  SUCCESS; /* skip this case */
    } else if (type == INSTRUCTION) {
        operator = safeMalloc((strlen(token)+1) * sizeof(char));
        strcpy(operator, token);
        p = strstr(line,operator);
        p = deleteWhiteSpaces(&p[strlen(operator)]);
        operands = safeMalloc(((strlen(p)+1) * sizeof(char)));
        strcpy(operands,  p);
        opcode = getOpcode(opcodes, operator);
        if (opcode < 0) { /* error case */
            if (operator[strlen(operator) - 1] == ',')
                errorCode = ILLEGAL_COMMA;
            else
                errorCode = opcode;
        }
        if (opcodes->operands_needed[opcode] == 0) { /* operator with no operands */
            token = strtok(NULL, " ,\t");
            if (token != NULL)
                errorCode = EXTRANEOUS_TEXT;
        } else if (opcodes->operands_needed[opcode] == 1) { /* operator with 1 operand */
            token = strtok(operands, ",");
            if (token == NULL)
                errorCode = MISSING_OPERAND;
            if (operands[0] == ',')
                errorCode = ILLEGAL_COMMA;
            token = strtok(NULL, ",");
            if (token != NULL)
                errorCode = EXTRANEOUS_TEXT;
        } else if (opcodes->operands_needed[opcode] == 2) { /* operator with two operands */
            if (operands[0] == ',')
                errorCode = ILLEGAL_COMMA;
            strcpy(buffer, operands);
            token = strtok(operands, ","); /* get src */
            if (token == NULL)
                errorCode = MISSING_OPERAND;
            if (token != NULL) {
                if (token[0] == ',' || token[strlen(token) - 1] == ',')
                    errorCode = ILLEGAL_COMMA;
            }
            token = strtok(NULL, ","); /* get dst */
            if (token == NULL) {
                if (strchr(operands, ' ') != NULL) {
                    errorCode = MISSING_COMMA;
                } else {
                    errorCode = MISSING_OPERAND;
                }
            }
            if (token != NULL) {
                if (token[0] == ',' || token[strlen(token) - 1] == ',')
                    errorCode = ILLEGAL_COMMA;
            }
            token = strtok(buffer, " ,\t");
            for (i = 1; token != NULL; i++) {
                token = strtok(NULL, " ,\t");
                if (i > opcodes->operands_needed[opcode])
                    errorCode = EXTRANEOUS_TEXT;
            }
        }
    } else if (errorCode == SUCCESS){
        errorCode = UNKNOWN_OPERATOR;
    }

    if (operator != NULL)
        free(operator);
    if (operands != NULL)
        free(operands);
    free(buffer);
    return errorCode;
}

/*
 * Checks if the provided name matches any saved word in the opcode table.
 * Receives the name to be checked and pointer to the opcode table containing saved words.
 * Returns an integer indicating the result of the check:
 *     - ILLEGAL_LABEL_NAME if the name matches a saved word in the opcode table.
 *     - SUCCESS if the name does not match any saved word.
 */
int isSavedWord(char *name, op_table *opcodes) {
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
int isLegalName(char *name, op_table *opcodes) {
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