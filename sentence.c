#include "header.h"

/* Sentence processing functions */

/*
 * Determines the type of operand based on the provided token.
 * Receives the token representing the operand.
 * Returns the type of operand (OperandType).
 */
OperandType getOpType(char *token){
    token = deleteWhiteSpaces(token);
    if (token[0] == 'r' && isdigit(token[1])){ /* REGISTER, */
        if (!IS_REGISTER(atoi(&token[1])))
            return REG_DOES_NOT_EXIST;
        else
            return REGISTER;
    } else if (token[0] == '#') { /* DIRECT */
        if (isdigit(token[1]) || isalpha(token[1])) /* is number or symbol */
            return IMMEDIATE_OP;
        else if (isNumber(&token[1])) /* is number */
            return IMMEDIATE_OP;
        else
            return UNKNOWN_OPERAND;
    } else if (isalpha(token[0])) { /* label or array index,*/
        if (strchr(token, '['))
            return ARRAY_INDEX;
        else
            return LABEL_OP;
    } else if (isNumber(token))
        return NUMBER_OP;
    return UNKNOWN_OPERAND;
}

/*
 * Retrieves the value of the operand based on its type and representation.
 * Receives the operand string and the pointer to the list of symbols table (labels and constants).
 * Returns the value of the operand if it's recognized and valid, or UNKNOWN_OPERATOR if it's unrecognized.
 */
int getOpValue (char *op, list *symbols){
    list *symbol;
    OperandType type = getOpType(op);
    op = deleteWhiteSpaces(op); /* delete white spaces */
    if (type == REGISTER){ /* register */
        op++; /* skip 'r' */
        return atoi(op);
    } else if (type == IMMEDIATE_OP){ /* immediate mode number */
        op++; /* skip '#' */
        if (op[0] == '-' || op[0] == '+')
            return atoi(op);
    } else if (type == LABEL_OP) { /* label address or constant */
        if ((symbol = getElementByName(symbols, op)) != NULL){
            return symbol->value;
            }
        else
            return UNKNOWN_OPERAND;
    } else if (type == ARRAY_INDEX){ /* array index  */
        op = strchr(op, '[');
        op[strlen(op)-1] = '\0'; /* delete square brackets */
        op++;
        if (isNumber(op))
            return atoi(op);
        if (isalpha(op[0])) {
            if ((symbol = getElementByName(symbols, op)) != NULL)
                return symbol->value;
            else
                return UNKNOWN_OPERAND;
        }
    } else if (type == NUMBER_OP)
        return atoi(op);

    return UNKNOWN_OPERATOR;
}

SentenceType getSentence(opcode_table *opcodes, char *token, int lineNum){
    SentenceType type;
    char *tmp = safeMalloc(strlen(token)+1);
    strcpy(tmp, token);
    tmp = deleteWhiteSpaces(tmp);
    if (tmp[strlen(tmp)-1] == ',')
        tmp[strlen(tmp)-1] = '\0';
    if (tmp[0] == ';') /* comment */
        type = COMMENT;
    else if (tmp [0] == '\0') /* empty */
        type =  EMPTY;
    else if (tmp[strlen(tmp)-1] == ':')
        type = LABEL;
    else if (strcmp(tmp,".define") == 0)
        type = DEFINE;
    else if (getOpcode(opcodes, tmp) != UNKNOWN_OPERATOR) /* instruction */
        type = INSTRUCTION;
    else if (tmp [0] == '.') { /* directive sentence */
        if (strcmp(tmp, ".extern") == 0) /* .extern */
            type = EXTERN;
        else if (strcmp(tmp, ".entry") == 0) /* .entry */
            type =  ENTRY;
        else if (strcmp(tmp, ".data") == 0)/* .data */
            type =  DATA;
        else if (strcmp(tmp, ".string") == 0)/* .string */
            type =  STRING;
        else
            type = UNKNOWN_OPERATOR;
    } else {
        type = UNKNOWN_OPERATOR;
    }
    return type;
}

int getAddressingMode (char *operand){
    OperandType type;
    operand = deleteWhiteSpaces(operand);
    type = getOpType(operand);
    /* check for empty operator */
    if (strchr(operand,' ') != NULL) {
        return MISSING_COMMA;
    }
    /* get mode */
    if (operand[0] == '\0')
        return UNKNOWN_OPERATOR;
    if (type == REG_DOES_NOT_EXIST)
        return REG_DOES_NOT_EXIST;
    if (type == REGISTER) { /* register */
        return REGISTER_MODE;
    } else if (type == IMMEDIATE_OP){ /* immediate */
        return IMMEDIATE;
    } else if (type == LABEL_OP){ /* direct */
        return DIRECT_MODE;
    } else if (type == ARRAY_INDEX) { /* indexed */
        return INDEXED_MODE;
    }
    return UNKNOWN_OPERAND;
}

/* returns opcode value if exist or UNKNOWN_OPERATOR if doesn't */
int getOpcode(opcode_table *opcodes, char *token){
    int i;
    if (token[strlen(token)-1] == ',')
        return ILLEGAL_COMMA;
    for (i = 0; i < MAX_OPERATORS; ++i) {
        if (strcmp(token, opcodes->name[i]) == 0)
            return i;
    }
    return UNKNOWN_OPERATOR;
}