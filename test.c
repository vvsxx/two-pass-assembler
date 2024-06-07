#include "header.h"

void compareFilesLineByLine() {
    const char file1[] = "encrypted_test";
    const char file2[] = "valid_input.ob";
    FILE *fp1 = fopen(file1, "r");
    FILE *fp2 = fopen(file2, "r");

    if (fp1 == NULL || fp2 == NULL) {
        fprintf(stderr, "Error opening files.\n");
        if (fp1) fclose(fp1);
        if (fp2) fclose(fp2);
        return;
    }

    char test[LINE_LENGTH];
    char result[LINE_LENGTH];
    char cleanRes[LINE_LENGTH];
    int lineNum = 1;
    fgets(result, LINE_LENGTH, fp2); /* skip first line */
    while (fgets(test, LINE_LENGTH, fp1) && fgets(result, LINE_LENGTH, fp2)) {
        memset(cleanRes, '\0', LINE_LENGTH);
        memcpy(cleanRes, &result[5], 7);
        if (test[strlen(test)-1] == '\n')
            test[strlen(test)-1] = '\0';
        if (test[strlen(test)-1] == '\r')
            test[strlen(test)-1] = '\0';

        printf("%s --- %s ", test, cleanRes);

        if (strcmp(test, cleanRes) == 0){
            printf("SUCCESS\n");
        } else {
            printf("ERROR\n");
        }
    }

    fclose(fp1);
    fclose(fp2);
}