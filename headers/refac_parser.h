#ifndef REFAC_PARSER_H
#define REFAC_PARSER_H

#include <stdlib.h>
#include "refac_lexer.h"
#include "ast_node.h"

typedef struct Parser {
    Lexer* lexer;
    Token* currentToken;
    Token* nextToken;
    ASTNode* ast;
    bool has_error;
} Parser;

Parser* init_parser(Lexer* lexer);
ClassNode* parse_class(Parser* parser);
ClassVarDecNode* parse_class_var_dec(Parser* parser);
SubroutineDecNode* parse_subroutine_dec(Parser* parser);
ParameterListNode* parse_parameter_list(Parser* parser);
SubroutineBodyNode* parse_subroutine_body(Parser* parser);
VarDecNode* parse_var_dec(Parser* parser);
StatementsNode* parse_statements(Parser* parser);
StatementNode* parse_statement(Parser* parser);
LetStatementNode* parse_let_statement(Parser* parser);
IfStatementNode* parse_if_statement(Parser* parser);
WhileStatementNode* parse_while_statement(Parser* parser);
DoStatementNode* parse_do_statement(Parser* parser);
ReturnStatementNode* parse_return_statement(Parser* parser);
SubroutineCallNode* parse_subroutine_call(Parser* parser);
ExpressionNode* parse_expression(Parser* parser);
TermNode* parse_term(Parser* parser);

void* safer_malloc(size_t size);
void safer_free(void** ptr);
void destroy_ast(ASTNode* ast);
void destroy_class(ClassNode* class);
void destroy_class_var_dec(ClassVarDecNode* class_var_dec);
void destroy_subroutine_dec(SubroutineDecNode* subroutine_dec);
void destroy_parameter_list(ParameterListNode* parameter_list);
void destroy_subroutine_body(SubroutineBodyNode* subroutine_body);
void destroy_var_dec(VarDecNode* var_dec);
void destroy_statements(StatementsNode* statements);
void destroy_statement(StatementNode* statement);
void destroy_let_statement(LetStatementNode* let_statement);
void destroy_if_statement(IfStatementNode* if_statement);
void destroy_while_statement(WhileStatementNode* while_statement);
void destroy_do_statement(DoStatementNode* do_statement);
void destroy_return_statement(ReturnStatementNode* return_statement);
void destroy_subroutine_call(SubroutineCallNode* subroutine_call);
void destroy_expression(ExpressionNode* expression);
void destroy_term(TermNode* term);

void destroy_parser(Parser* parser);


#endif // REFAC_PARSER_H

