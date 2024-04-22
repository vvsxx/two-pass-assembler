#include "header.h"

/* functions accessible only from this file */
void writeFile(FILE *inputFile, FILE *outputFile, macros_list *macros_h);
macros_list *buildTable(FILE *input, opcode_table *opcodes);

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
int preProcessor(char *filename, opcode_table *opcodes) {
    FILE *inputFile, *outputFile;
    macros_list *macros_h = NULL;
    char ch;
    char inputFileName[strlen(filename) + 4]; /* ".as/.am" + '\0' */
    char outputFileName[strlen(filename) + 4];
    strcpy(inputFileName, filename);
    strcat(inputFileName, ".as\0"); /* create input file name */
    inputFile = openFile(inputFileName, "r"); /* open input file for read */
    while ((ch = fgetc(inputFile)) != EOF && (ch == ' ' || ch == '\t')); /* skip white spaces */
    if (ch == EOF) /* if file is empty, stop here */
        return EMPTY_FILE;

    strcpy(outputFileName, filename);
    strcat(outputFileName, ".am\0"); /* create output file name */
    outputFile = openFile(outputFileName, "w"); /* open output file for write */
    macros_h = buildTable(inputFile, opcodes); /* build macros table */
    rewind(inputFile);
    writeFile(inputFile, outputFile, macros_h);
    freeList(macros_h, MACROS_LIST);
    fclose(outputFile);
    fclose(inputFile);
    return SUCCESS;
}

/* function creates macro table */
macros_list *buildTable(FILE *input, opcode_table *opcodes) {
    macros_list *macros = NULL, *macros_h = NULL;
    char line[LINE_LENGTH];
    char buffer[LINE_LENGTH];
    char *token;
    int isCorrect = SUCCESS;
    int i, lineNum = 0, isMacro = 0;
    while (fgets(line, sizeof(line), input) != NULL) {
        lineNum++;
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
                isCorrect = INCORRECT;
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
            if ((isCorrect = isSavedWord(token, opcodes)) != SUCCESS) {
                isCorrect = INCORRECT;
                printError(ILLEGAL_MACRO_NAME, lineNum);
            }
            macros->name = strDuplicate(token);
            macros->data = (char **) safeMalloc(sizeof(char *));
            macros->lines = i + 1;
            i--;
        } else if (isMacro && strcmp(token, "endmcr") != 0) { /* inside macro declaration */
            macros->data[i] = safeMalloc(LINE_LENGTH * sizeof(char));
            strcat(macros->data[i], buffer);
        } else if (strcmp(token, "endmcr") == 0) { /* end of macro declaration found */
            isMacro = 0;
            i = 0;
            continue;
        }
        i++;
    }
    return macros_h;
}


void writeFile(FILE *inputFile, FILE *outputFile, macros_list *macros_h) {
    char line[LINE_LENGTH], buffer[LINE_LENGTH], *token; /* line processing */
    char *data;
    macros_list *macros = NULL;
    int i, macroDec = 0;
    while (fgets(line, sizeof(line), inputFile) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        strcpy(buffer, line);
        token = strtok(line, " \t");
        if (token == NULL) /* empty line case */
            continue;
        if (strcmp(token, "mcr") == 0)
            macroDec = 1;
        if (strcmp(token, "endmcr") == 0) {
            macroDec = 0;
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
    }
}
