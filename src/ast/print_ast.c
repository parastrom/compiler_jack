#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "print_ast.h"
#include <unistd.h>

#define BUFFER_SIZE 1024

char* buffer;
int bufferPos = 0;

void writeToFile(FILE* file, const char* format, ...) {
    char formattedStr[BUFFER_SIZE];

    // Initialize the variable argument list
    va_list args;
    va_start(args, format);

    // Format the string
    vsnprintf(formattedStr, sizeof(formattedStr), format, args);

    // Clean up the variable argument list
    va_end(args);

    int len = strlen(formattedStr);
    if (bufferPos + len >= BUFFER_SIZE) {
        fwrite(buffer, sizeof(char), bufferPos, file);
        bufferPos = 0;
    }
    memcpy(buffer + bufferPos, formattedStr, len);
    bufferPos += len;
}


void flushBuffer(FILE* file) {
    if (bufferPos > 0) {
        fwrite(buffer, sizeof(char), bufferPos, file);
        bufferPos = 0;
    }
}

void printSpaces(FILE* file, int depth) {
    for (int i = 0; i < depth; ++i) {
        writeToFile(file, "│  ");
    }
}


void printTermNode(FILE* file, struct TermNode* termNode, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "TermNode\n");
    printSpaces(file, depth);
    switch (termNode->termType) {
    case INTEGER_CONSTANT:
        writeToFile(file, "├─ Integer Constant: %d\n", termNode->data.intValue);
        break;
    case STRING_CONSTANT:
        writeToFile(file, "├─ String Constant: %s\n", termNode->data.stringValue);
        break;
    case KEYWORD_CONSTANT:
        writeToFile(file, "├─ Keyword Constant: %s\n", termNode->data.keywordValue);
        break;
    case VAR_TERM:
        writeToFile(file, "├─ VarTerm:\n");
        printVarTerm(file, &termNode->data.varTerm, depth+1);
        break;
    case ARRAY_ACCESS:
        writeToFile(file, "├─ Array Access:\n");
        writeToFile(file, "│  ├─ Array Name: %s\n", termNode->data.arrayAccess.arrayName);
        writeToFile(file, "│  └─ Index Expression:\n");
        printExpressionNode(file, termNode->data.arrayAccess.index, depth+2);
        break;
    case SUBROUTINE_CALL:
        writeToFile(file, "├─ Subroutine Call: ");
        printSubroutineCallNode(file, termNode->data.subroutineCall, depth+1);
        break;
    case EXPRESSION:
        writeToFile(file, "├─ Expression: ");
        printExpressionNode(file, termNode->data.expression, depth+1);
        break;
    case UNARY_OP:
        writeToFile(file, "├─ Unary Operation:\n");
        writeToFile(file, "│  ├─ Unary Operator: %c\n", termNode->data.unaryOp.unaryOp);
        writeToFile(file, "│  └─ Term:\n");
        printTermNode(file, termNode->data.unaryOp.term, depth+2);
        break;
    default:
        writeToFile(file, "├─ No Term\n");
        break;
    }
}



void printVarTerm(FILE* file, struct VarTerm* varTerm, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "VarTerm\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ className: %s\n", varTerm->className ? varTerm->className : "NULL");

    printSpaces(file, depth+1);
    writeToFile(file, "└─ varName: %s\n", varTerm->varName);
}


void printOperation(FILE* file, struct Operation* operation, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "Operation\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ op: %c\n", operation->op);

    printSpaces(file, depth+1);
    writeToFile(file, "└─ term:\n");
    printTermNode(file, operation->term, depth+2);
}


void printExpressionNode(FILE* file, struct ExpressionNode* node, int depth) {
    writeToFile(file, "ExpressionNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ term:\n");
    printTermNode(file, node->term, depth+2);

    printSpaces(file, depth+1);
    writeToFile(file, "└─ operations:\n");
    if (node->operations != NULL) {
        for (int i = 0; i < vector_size(node->operations); i++) {
            Operation* operation = (Operation*)vector_get(node->operations, i);
            printOperation(file, operation, depth+2);
        }
    } else {
        printSpaces(file, depth+2);
        writeToFile(file, "NULL\n");
    }
}


void printSubroutineCallNode(FILE* file, struct SubroutineCallNode* node, int depth) {
    writeToFile(file, "SubroutineCallNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ caller: %s\n", node->caller ? node->caller : "NULL");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ subroutineName: %s\n", node->subroutineName);

    printSpaces(file, depth+1);
    writeToFile(file, "└─ arguments: \n");
    if (node->arguments != NULL) {
        for (int i = 0; i < vector_size(node->arguments); i++) {
            ExpressionNode* argument = (ExpressionNode*)vector_get(node->arguments, i);
            printExpressionNode(file, argument, depth+2);
        }
    } else {
        printSpaces(file, depth+2);
        writeToFile(file, "NULL\n");
    }
}


void printDoStatementNode(FILE* file, struct DoStatementNode* node, int depth) {
    writeToFile(file, "DoStatementNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "└─ subroutineCall: ");
    printSubroutineCallNode(file, node->subroutineCall, depth+2);
}


void printReturnStatementNode(FILE* file, struct ReturnStatementNode* node, int depth) {
    writeToFile(file, "ReturnStatementNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "└─ expression: ");
    if (node->expression != NULL) {
        printExpressionNode(file, node->expression, depth+2);
    } else {
        writeToFile(file, "NULL\n");
    }
}


void printWhileStatementNode(FILE* file, struct WhileStatementNode* node, int depth) {
    writeToFile(file, "WhileStatementNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ condition: ");
    printExpressionNode(file, node->condition, depth+2);

    printSpaces(file, depth+1);
    writeToFile(file, "└─ body: ");
    printStatementsNode(file, node->body, depth+2);
}


void printIfStatementNode(FILE* file, struct IfStatementNode* node, int depth) {
    writeToFile(file, "IfStatementNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ condition: ");
    printExpressionNode(file, node->condition, depth+2);

    printSpaces(file, depth+1);
    writeToFile(file, "├─ ifBranch: \n");
    printStatementsNode(file, node->ifBranch, depth+2);

    printSpaces(file, depth+1);
    writeToFile(file, "└─ elseBranch: \n");
    if (node->elseBranch != NULL) {
        printStatementsNode(file, node->elseBranch, depth+2);
    } else {
        writeToFile(file, "NULL\n");
    }
}


void printLetStatementNode(FILE* file, struct LetStatementNode* node, int depth) {
    writeToFile(file, "LetStatementNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ varName: ");
    writeToFile(file, node->varName);
    writeToFile(file, "\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ indexExpression: ");
    if (node->indexExpression != NULL) {
        printExpressionNode(file, node->indexExpression, depth+2);
    } else {
        writeToFile(file, "NULL\n");
    }

    printSpaces(file, depth+1);
    writeToFile(file, "└─ rightExpression: ");
    printExpressionNode(file, node->rightExpression, depth+2);
}


void printStatementNode(FILE* file, struct StatementNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "StatementNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ Type: ");
    switch (node->statementType) {
        case LET:
            writeToFile(file, "LET");
            break;
        case IF:
            writeToFile(file, "IF");
            break;
        case WHILE:
            writeToFile(file, "WHILE");
            break;
        case DO:
            writeToFile(file, "DO");
            break;
        case RETURN:
            writeToFile(file, "RETURN");
            break;
        default:
            writeToFile(file, "NONE");
            break;
    }
    writeToFile(file, "\n");

    printSpaces(file, depth+1);
    writeToFile(file, "└─ Data: ");
    switch (node->statementType) {
        case LET:
            printLetStatementNode(file, node->data.letStatement, depth+2);
            break;
        case IF:
            printIfStatementNode(file, node->data.ifStatement, depth+2);
            break;
        case WHILE:
            printWhileStatementNode(file, node->data.whileStatement, depth+2);
            break;
        case DO:
            printDoStatementNode(file, node->data.doStatement, depth+2);
            break;
        case RETURN:
            printReturnStatementNode(file, node->data.returnStatement, depth+2);
            break;
        default:
            break;
    }
}


void printVarDecNode(FILE* file, struct VarDecNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "VarDecNode\n");

    printSpaces(file, depth+1);
    writeToFile(file, "├─ Type: ");
    writeToFile(file, node->varType);
    writeToFile(file, "\n");

    int size = vector_size(node->varNames);
    for (int i = 0; i < size; ++i) {
        char* name = (char*) vector_get(node->varNames, i);
        printSpaces(file, depth+1);
        writeToFile(file, "└─ Name: ");
        writeToFile(file, name);
        writeToFile(file, "\n");
    }
}

void printStatementsNode(FILE* file, struct StatementsNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "StatementsNode\n");

    int size = vector_size(node->statements);
    for (int i = 0; i < size; ++i) {
        StatementNode* statement = (StatementNode*) vector_get(node->statements, i);
        printStatementNode(file, statement, depth+1);
    }
}


void printParameterListNode(FILE* file, struct ParameterListNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "ParameterListNode\n");

    int size = vector_size(node->parameterTypes);
    for (int i = 0; i < size; ++i) {
        char* type = (char*) vector_get(node->parameterTypes, i);
        char* name = (char*) vector_get(node->parameterNames, i);
        printSpaces(file, depth+1);
        writeToFile(file, "├─ Type: ");
        writeToFile(file, type);
        writeToFile(file, "\n");
        printSpaces(file, depth+1);
        writeToFile(file, "└─ Name: ");
        writeToFile(file, name);
        writeToFile(file, "\n");
    }
}

void printSubroutineBodyNode(FILE* file, struct SubroutineBodyNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "SubroutineBodyNode\n");

    int size = vector_size(node->varDecs);
    for (int i = 0; i < size; ++i) {
        VarDecNode* varDec = (VarDecNode*) vector_get(node->varDecs, i);
        printVarDecNode(file, varDec, depth+1);
    }
    printStatementsNode(file, node->statements, depth+1);
}


void printClassVarDecNode(FILE* file, struct ClassVarDecNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "├─ classVarModifier: ");
    switch (node->classVarModifier) {
        case CVAR_NONE: writeToFile(file, "CVAR_NONE"); break;
        case STATIC: writeToFile(file, "STATIC"); break;
        case FIELD: writeToFile(file, "FIELD"); break;
    }
    writeToFile(file, "\n");
    printSpaces(file, depth);
    writeToFile(file, "├─ varType: ");
    writeToFile(file, node->varType);
    writeToFile(file, "\n");
    for (int i = 0; i < vector_size(node->varNames); i++) {
        printSpaces(file, depth);
        writeToFile(file, "├─ varName: ");
        writeToFile(file, (char*)vector_get(node->varNames, i));
        writeToFile(file, "\n");
    }
}

void printSubroutineDecNode(FILE* file, struct SubroutineDecNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "├─ subroutineType: ");
    switch (node->subroutineType) {
        case SUB_NONE: writeToFile(file, "SUB_NONE"); break;
        case CONSTRUCTOR: writeToFile(file, "CONSTRUCTOR"); break;
        case FUNCTION: writeToFile(file, "FUNCTION"); break;
        case METHOD: writeToFile(file, "METHOD"); break;
    }
    writeToFile(file, "\n");
    
    printSpaces(file, depth);
    writeToFile(file, "├─ returnType: ");
    writeToFile(file, node->returnType);
    writeToFile(file, "\n");
    
    printSpaces(file, depth);
    writeToFile(file, "├─ subroutineName: ");
    writeToFile(file, node->subroutineName);
    writeToFile(file, "\n");
    
    // Print the parameters if present
    if (node->parameters != NULL) {
        printSpaces(file, depth);
        writeToFile(file, "├─ Parameters\n");
        printParameterListNode(file, node->parameters, depth + 1);
    }
    
    // Print the body if present
    if (node->body != NULL) {
        printSpaces(file, depth);
        writeToFile(file, "├─ SubroutineBody\n");
        printSubroutineBodyNode(file, node->body, depth + 1);
    }
    
    // Continue for other fields and child nodes...
}

void printClassNode(FILE* file, struct ClassNode* node, int depth) {
    printSpaces(file, depth);
    writeToFile(file, "├─ className: ");
    writeToFile(file, node->className);
    writeToFile(file, "\n");
    
    for (int i = 0; i < vector_size(node->classVarDecs); i++) {
        printSpaces(file, depth);
        writeToFile(file, "├─ ClassVarDecNode\n");
        printClassVarDecNode(file, (struct ClassVarDecNode*)vector_get(node->classVarDecs, i), depth + 1);
    }
    
    for (int i = 0; i < vector_size(node->subroutineDecs); i++) {
        printSpaces(file, depth);
        writeToFile(file, "├─ SubroutineDecNode\n");
        printSubroutineDecNode(file, (struct SubroutineDecNode*)vector_get(node->subroutineDecs, i), depth + 1);
    }
}

void printProgramNode(FILE* file, struct ProgramNode* node, int depth) {
    writeToFile(file, "ProgramNode\n");
    for (int i = 0; i < vector_size(node->classes); i++) {
        printSpaces(file, depth);
        writeToFile(file, "├─ ClassNode\n");
        printClassNode(file, (struct ClassNode*)vector_get(node->classes, i), depth + 1);
    }
}

void print_class(ClassNode* node) {
    buffer = safer_malloc(BUFFER_SIZE * sizeof(char));
    FILE* file = fopen("ast.txt", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    } else {
        perror("getcwd() error");
    }
    printClassNode(file, node, 0);
    flushBuffer(file);
    fclose(file);
    free(buffer);
}
