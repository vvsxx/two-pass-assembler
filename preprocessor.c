#include "header.h"

/* functions accessible only from this file */
void writeFile(FILE *file, FILE *outputFile, macros_list *macros_h);
void buildTables(FILE *file, FILE *outputFile);


void preProcessor(char *filename){
    FILE *inputFile, *outputFile;
    char inputFileName[strlen(filename) + 4]; /* ".as/.am" + '\0' */
    char outputFileName[strlen(filename) + 4];
    /* prepare input/output files */
    strcpy(inputFileName,filename);
    strcat(inputFileName, ".as");
    inputFile = openFile(inputFileName, "r");
    strcpy(outputFileName, filename);
    strcat(outputFileName, ".am");
    outputFile = openFile(outputFileName, "w");
    buildTables(inputFile, outputFile);
    fclose(inputFile);
    fclose(outputFile);
}

void buildTables(FILE *file, FILE *outputFile) {
    macros_list *macros = NULL, *macros_h = NULL;
    char line[LINE_LENGTH];
    char buffer[LINE_LENGTH];
    char *token;
    int newLine, i, isMacro = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
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
                }
                if (macros_h == NULL)
                    macros_h = macros;
                macros->name = strDuplicate(token);
                macros->data = (char **) safeMalloc(sizeof(char *));
                macros->data[i] = (char *) safeMalloc(LINE_LENGTH * sizeof(char));
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
    writeFile(file, outputFile, macros_h);
    freeList(macros_h, MACROS_LIST);
}

void writeFile(FILE *file, FILE *outputFile, macros_list *macros_h) {
    char line[LINE_LENGTH], buffer[LINE_LENGTH], *token; /* line processing */
    char *data;
    macros_list *macros = NULL;
    int i = 0, macroDef = 0;
    rewind(file);
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        strcpy(buffer, line);
        token = strtok(line, " ,\t");
        if (token == NULL)
            continue;
        if (strcmp(token, "mcr") == 0)
            macroDef = 1;
        if (strcmp(token, "endmcr") == 0) {
            macroDef = 0;
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
            continue;
        }

        if (!macroDef)
            fprintf(outputFile, "%s\n", buffer);
    }
}

