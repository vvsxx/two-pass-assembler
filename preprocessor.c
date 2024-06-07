#include "header.h"

/* functions accessible only from this file */
char* removeSpaces(char *line);
void writeFile(FILE *inputFile, FILE *outputFile, macros_list *macros_h);
macros_list *buildTable(FILE *input, op_table *opcodes);
static int isCorrect; /* accessible only from this file, uses to detect logical problems in multiple functions */
/*
 * Pre-processes the source file by deploying macros and creating a ".am" file.
 *
 * Parameters:
 *   filename: The name of the source file to be pre-processed.
 *   opcodes: Pointer to the opcode table containing binary values and addressing modes.
 *
 * Returns:
 *   SUCCESS if pre-processing is successful, EMPTY_FILE if the file is empty,
 *   or an error code if an error occurs during pre-processing.
 */
int preProcessor(char *filename, op_table *opcodes) {
    FILE *inputFile = NULL, *outputFile = NULL;
    macros_list *macros_h = NULL;
    char ch;
    int nameSize = strlen(filename) + 4; /* ".as/.am" + '\0' */
    char *inputFileName = safeMalloc(nameSize * sizeof (int));
    char *outputFileName = safeMalloc(nameSize * sizeof (int));
    isCorrect = SUCCESS;
    strcpy(inputFileName, filename);
    strcat(inputFileName, ".as\0"); /* create input file name */
    inputFile = openFile(inputFileName, "r"); /* open input file for read */
    if (inputFile == NULL){ /* file not found */
        free(inputFileName);
        free(outputFileName);
        return FILE_NOT_FOUND;
    }
    while ((ch = fgetc(inputFile)) != EOF && (ch == ' ' || ch == '\t')); /* skip white spaces */
    if (ch == EOF) /* if file is empty, stop here */
        isCorrect = EMPTY_FILE;

    strcpy(outputFileName, filename);
    strcat(outputFileName, ".am\0"); /* create output file name */
    if (isCorrect != EMPTY_FILE) {
        outputFile = openFile(outputFileName, "w"); /* open output file for write */
        if (outputFile == NULL) /* can't open output file */
            isCorrect = ERROR_OPENING_FILE;
        macros_h = buildTable(inputFile, opcodes); /* build macros table */
        rewind(inputFile);
        writeFile(inputFile, outputFile, macros_h);
    }
    freeList(macros_h, MACROS_LIST);
    if (outputFile != NULL) {
        fclose(outputFile);
    }
    fclose(inputFile);
    free(inputFileName);
    free(outputFileName);
    return isCorrect;
}

/* creates table that contains all data defined between each macro declaration */
macros_list *buildTable(FILE *input, op_table *opcodes) {
    macros_list *macros = NULL, *macros_h = NULL;
    char line[LINE_LENGTH];
    char *tmp = NULL, *buffer;
    char *token;
    int i, c, lineNum = 0, isMacro = 0;
    buffer = safeMalloc((LINE_LENGTH + 1) * sizeof(char));
    while (fgets(line, LINE_LENGTH, input) != NULL) {
        lineNum++;
        /* if line is longer than LINE_LENGTH characters */
        if (strlen(line) == LINE_LENGTH-1 && line[LINE_LENGTH - 1] != '\n') {
            printError(TOO_LONG_LINE, lineNum); /* print alert */
            while ((c = fgetc(input)) != '\n' && c != EOF); /* skip extra characters */
        }
        line[strcspn(line, "\n")] = '\0';
        strcpy(buffer, line);
        token = strtok(line, " \t");
        if (token == NULL) /* empty line */
            continue;
        if (strcmp(token, "mcr") == 0) { /* macro definition found */
            isMacro = 1;
            i = 0;
            token = strtok(NULL, " \t"); /* macro name */
            if (token == NULL) {
                isCorrect = ILLEGAL_MACRO_NAME;
                printError(ILLEGAL_MACRO_NAME, lineNum);
                continue;
            }
            if (macros != NULL) { /* list already exist */
                macros->next = safeMalloc(sizeof(macros_list));
                macros = macros->next;
            } else { /* list is empty  */
                macros = safeMalloc(sizeof(macros_list));
                macros_h = macros;
            }
            macros->next = NULL;
            if ((isCorrect = isSavedWord(token, opcodes)) != SUCCESS) {
                isCorrect = ILLEGAL_MACRO_NAME;
                printError(ILLEGAL_MACRO_NAME, lineNum);
            }
            macros->name = strDuplicate(token);
            macros->data = (char **) safeMalloc(sizeof(char *));
            macros->lines = i + 1;
            i--;
        } else if (isMacro && strcmp(token, "endmcr") != 0) { /* inside macro declaration */
            macros->data[i] = safeMalloc((LINE_LENGTH + 1) * sizeof(char));
            tmp = removeSpaces(buffer);
            strcpy(macros->data[i], tmp);
            free(tmp);
        } else if (strcmp(token, "endmcr") == 0) { /* end of macro declaration found */
            isMacro = 0;
            i = 0;
            continue;
        }
        i++;

    }
    free(buffer);
    return macros_h;
}

/* Writes the processed assembly code to the .am file */
void writeFile(FILE *inputFile, FILE *outputFile, macros_list *macros_h) {
    char line[LINE_LENGTH], buffer[LINE_LENGTH], *token; /* line processing */
    char *data;
    char *formatedLine = NULL;
    macros_list *macros = NULL;
    int i, c, macroDec = 0;
    SentenceType sentence;
    while (fgets(line, LINE_LENGTH, inputFile) != NULL) {
        /* if line is longer than LINE_LENGTH characters */
        if (strlen(line) == LINE_LENGTH-1 && line[LINE_LENGTH - 1] != '\n')
            while ((c = fgetc(inputFile)) != '\n' && c != EOF); /* skip extra characters */
        formatedLine = removeSpaces(line);
        strcpy(line, formatedLine);
        line[strcspn(line, "\n")] = '\0';
        strcpy(buffer, line);
        token = strtok(line, " \t");
        if (token == NULL || token[0] == ';') { /* empty line or comment case */
            free(formatedLine);
            continue;
        }
        if (strcmp(token, "mcr") == 0)
            macroDec = 1;
        if (strcmp(token, "endmcr") == 0) {
            macroDec = 0;
            free(formatedLine);
            continue;
        }
        if ((macros = getMacroByName(macros_h, token)) != NULL) { /* marco name found */
            i = 0;
            data = macros->data[i];
            while (data != NULL && i <= macros->lines) {
                fprintf(outputFile, "%s\n", data);
                i++;
                data = macros->data[i];
            }
        } else if (!macroDec)
            fprintf(outputFile, "%s\n", buffer);

        free(formatedLine);
    }
}


char* removeSpaces(char *line){
    char *token = NULL;
    char *p = line;
    char *newLine = safeMalloc(LINE_LENGTH);
    newLine[0] = '\0';
    while ((token = strtok(p, ",")) != NULL && (strcmp(newLine, token) != 0)){
        p = NULL;
        token = deleteWhiteSpaces(token);
        if (strlen(newLine) > 0)
            strcat(newLine,",");
        strcat(newLine,token);
    }
    return newLine;
}