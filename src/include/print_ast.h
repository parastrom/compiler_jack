#ifndef PRINT_AST_H
#define PRINT_AST_H

#include "ast.h"

void flushBuffer(FILE* file);
void writeToFile(FILE* file, const char* format, ...);
void printSpaces(FILE* file, int depth);
void printTermNode(FILE* file, struct TermNode* termNode, int depth);
void printVarTerm(FILE* file, struct VarTerm* varTerm, int depth);
void printOperation(FILE* file, struct Operation* operation, int depth);
void printExpressionNode(FILE* file, struct ExpressionNode* node, int depth);
void printSubroutineCallNode(FILE* file, struct SubroutineCallNode* node, int depth);
void printStatementNode(FILE* file, struct StatementNode* node, int depth);
void printIfStatementNode(FILE* file, struct IfStatementNode* node, int depth);
void printWhileStatementNode(FILE* file, struct WhileStatementNode* node, int depth);
void printReturnStatementNode(FILE* file, struct ReturnStatementNode* node, int depth);
void printLetStatementNode(FILE* file, struct LetStatementNode* node, int depth);
void printDoStatementNode(FILE* file, struct DoStatementNode* node, int depth);
void printStatementsNode(FILE* file, struct StatementsNode* node, int depth);
void printSubroutineBodyNode(FILE* file, struct SubroutineBodyNode* node, int depth);
void printParameterListNode(FILE* file, struct ParameterListNode* node, int depth);
void printSubroutineDecNode(FILE* file, struct SubroutineDecNode* node, int depth);
void printClassVarDecNode(FILE* file, struct ClassVarDecNode* node, int depth);
void printClassNode(FILE* file, struct ClassNode* node, int depth);
void print_class(ClassNode* node);

#endif // PRINT_AST_H