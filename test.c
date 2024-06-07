#include "header.h"


void test(mem_img *img){
    int i, counter, int_val, j, error;
    char line[LINE_LENGTH], *addr, *val, tmp[2];
    char encrypted[10];
    FILE *testfile, *encfile;
    word *wrd_tmp = img->code_h;
    testfile = openFile("testfile", "r");
    encfile = openFile("encrypted_test", "r");
    memset(encrypted, '\0', sizeof (encrypted));
    /*  TESTS */
    while (wrd_tmp != NULL){
        error = 0;
        counter = wrd_tmp->address;
        fgets(encrypted, sizeof(encrypted), encfile);
        encrypted[strcspn(encrypted, "\r")] = '\0';
        if (fgets(line, sizeof(line), testfile) != NULL){
            addr = strtok(line, " \t");
            val = strtok(NULL, " \t");
        } else {
            printf("НЕХВАТАЕТ СТРОК\n");
        }
        if (counter != atoi(addr))
            error = 1;
        if (strcmp(encrypted, wrd_tmp->secure4) != 0)
            error = 1;
        printf("%d\t\t%s\t\t", counter, wrd_tmp->secure4);
        j = 0;
        error = 0;
        for (i = sizeof(short) * 8 - 1; i >= 0; --i, j++) {
            int bit = (wrd_tmp->binary >> i) & 1;
            tmp[0] = val[j];
            tmp[1] = '\0';
            int_val = atoi( tmp);
            printf("%d ", bit);
            if (int_val != bit)
                error = 1;
        }
        if (error == 0)
            printf(" OK\n");
        else
            printf(" error\n");
        wrd_tmp = wrd_tmp->next;
    }
    wrd_tmp = img->data_h;
    while (wrd_tmp != NULL){
        counter = wrd_tmp->address;
        if (fgets(line, sizeof(line), testfile) != NULL){
            addr = strtok(line, " \t");
            val = strtok(NULL, " \t");
        } else {
            printf("НЕХВАТАЕТ СТРОК\n");
        }
        if (counter != atoi(addr))
            error = 1;
        printf("%d\t\t%s\t\t", counter, wrd_tmp->secure4);
        error = 0;
        j = 0;
        for (i = sizeof(short) * 8 - 1; i >= 0; --i, j++) {
            int bit = (wrd_tmp->binary >> i) & 1;
            tmp[0] = val[j];
            tmp[1] = '\0';
            int_val = atoi( tmp);
            printf("%d ", bit);
            if (int_val != bit)
                error = 1;
        }
        if (error == 0)
            printf(" OK\n");
        else
            printf(" error\n");
        wrd_tmp = wrd_tmp->next;
    }
    fclose(testfile);
}