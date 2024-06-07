#include "header.h"

/* contains static data to fill opcode table in start of program */
void getOpTable(op_table *table, int max_registers) {
    int i;
    char name[MAX_OPERATORS][OPCODE_LENGTH] = {"mov", "cmp", "add", "sub",
                                               "not", "clr", "lea", "inc",
                                               "dec", "jmp", "bne", "red",
                                               "prn", "jsr", "rts", "hlt"};

    int operands_needed[MAX_OPERATORS] = {2, 2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0};
    int alwd_src[MAX_OPERATORS][MAX_MODES] = {{1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1},
                                              {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0},
                                              {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
                                              {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    int alwd_dst[MAX_OPERATORS][MAX_MODES] = {{0, 1, 1, 1}, {1, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1},
                                              {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1},
                                              {0, 1, 1, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 1, 1},
                                              {1, 1, 1, 1}, {0, 1, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}};

    for (i = 0; i < max_registers; ++i) {
        table->registerNames[i][0] = 'r';
        table->registerNames[i][1] = '0' + i;
    }

    memcpy(table->name, name, sizeof(name));
    memcpy(table->operands_needed, operands_needed, sizeof(operands_needed));
    memcpy(table->allowed_src, alwd_src, sizeof(alwd_src));
    memcpy(table->allowed_dst, alwd_dst, sizeof(alwd_dst));
}
