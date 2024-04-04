#include "header.h"

FILE * openFile(char *fileName, char *mode){
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
            fprintf(stderr, "Error opening file\n");

        exit(EXIT_FAILURE);
    }
    return file;
}

/* memory allocation with error handling */
void *safeMalloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Allocating memory error\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}



void dec_to_bin(int decimal, int *binary, int array_size){
    int i, tmp = decimal;

    for (i = 0; i < array_size; ++i)
        binary[i] = 0;

    i = 0;
    /* convert to binary as is */
    while (tmp && i < array_size){
        binary[i] = tmp % 2;
        i++;
        tmp /= 2;
    }
    if(decimal < 0){
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

void bin_to_4secure(char *binary, char secure4){
    int i,j,res = 7;
    int base4[4][2] = {{0,0},
                       {0,1},
                       {1,0},
                       {1,1}};
    int bin[] = {0,0,0,0,1,1,1,0,0,0,1,0,1,0};
    int result[8] = {0,0,0,0,0,0,0,0};
    char secure[] = {'*','#','%','!'};


    if (bin[0] == 1) { /* is negative */
        /* code for negative case*/
    } else if (bin[0] == 0){
        for (i = WORD_L-1; i > 0; i -= 2) {
            for (j = 0; j < 4; ++j) {
                if(base4[j][0] == bin[i-1] && base4[j][1] == bin[i]) {
                    result[res] = j;
                    res--;
                    break;
                }
            }
        }
    } else {
        /* ERROR */
    }
}

/* sets array values to 0 */
void resetBits(int *arr, int size){
    int i;
    for (i = 0; i < size; ++i)
        arr[i] = 0;
}


/* differs from the standard strdup one in that it uses safeMalloc */
char *strDuplicate(const char *src) {
    size_t len = strlen(src) + 1;
    char *dst = safeMalloc(len);
    strcpy(dst, src);
    return dst;
}

/* deletes white spaces and removes newline character */
char *deleteWhiteSpaces(char *token){
    if (token[strlen(token) - 1] == '\n')
        token[strlen(token) - 1] = '\0';

    while (token[0] == ' ')
        token++;
    while(token[strlen(token)-1] == ' ')
        token[strlen(token)-1] = '\0';
    return token;
}

/* returns non zero value in case if token is number */
int isNumber(char *token){
    if(token[0] == '-' || token[0] == '+')
        return isdigit(token[1]);
    return isdigit(token[0]);
}
