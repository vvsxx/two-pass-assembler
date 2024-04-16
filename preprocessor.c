#include "header.h"

/* functions accessible only from this file */
void writeFile(FILE *inputFile, FILE *outputFile, macros_list *macros_h);
macros_list *buildTable(FILE *input);


int preProcessor(char *filename){
    FILE *inputFile, *outputFile;
    macros_list *macros_h = NULL;
    char ch;
    char inputFileName[strlen(filename) + 4]; /* ".as/.am" + '\0' */
    char outputFileName[strlen(filename) + 4];
    /* prepare input/output files */
    strcpy(inputFileName,filename);
    strcat(inputFileName, ".as\0");
    inputFile = openFile(inputFileName, "r");
    while ((ch = fgetc(inputFile)) != EOF && (ch == ' ' || ch == '\t')); /* skip white spaces */
    if (ch == EOF)
        return EMPTY_FILE;
    rewind(inputFile);
    strcpy(outputFileName, filename);
    strcat(outputFileName, ".am\0");
    macros_h = buildTable(inputFile);
    outputFile = openFile(outputFileName, "w");
    rewind(inputFile);
    writeFile(inputFile, outputFile, macros_h);
    freeList(macros_h, MACROS_LIST);
    fclose(outputFile);
    fclose(inputFile);
    return 1;
}

/* function creates macro table */
macros_list *buildTable(FILE *input) {
    macros_list *macros = NULL, *macros_h = NULL;
    char line[LINE_LENGTH];
    char buffer[LINE_LENGTH];
    char *token;
    int newLine, i, isMacro = 0;
    while (fgets(line, sizeof(line), input) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        strcpy(buffer, line);
        token = strtok(line, " ,\t");
        while (token != NULL) {
            if (strcmp(token,"mcr") == 0) { /* macro definition found */
                isMacro = 1;
                i = 0;
                token = strtok(NULL, " \t"); /* macro name */
                if (macros != NULL) { /* list already exist */
                    macros->next = safeMalloc(sizeof(macros_list));
                    macros = macros->next;
                } else { /* list is empty  */
                    macros = safeMalloc(sizeof(macros_list));
                    macros_h = macros;
                }

                macros->name = strDuplicate(token);
                macros->data = (char **) safeMalloc(sizeof(char *));
                macros->lines = i+1;
                token = strtok(NULL, " ,\t");
                i--;
            } else if (isMacro && strcmp(token, "endmcr")) { /* inside macro definition */
                if (newLine)
                    macros->data[i] = safeMalloc(LINE_LENGTH * sizeof(char));
                newLine = 0;
                strcat(macros->data[i], " ");
                strcat(macros->data[i], token);
            } else if (strcmp(token, "endmcr") == 0) { /* end of macro definition found */
                isMacro = 0;
                i = 0;
            }
            token = strtok(NULL, " \t");
        }
        newLine = 1;
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
            while (data != NULL && i <= macros->lines){
                fprintf(outputFile, "\t%s\n", data);
                i++;
                data = macros->data[i];
            }
        } else if (!macroDec)
            fprintf(outputFile, "%s\n", buffer);
    }
}

