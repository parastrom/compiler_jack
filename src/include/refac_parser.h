#ifndef REFAC_PARSER_H
#define REFAC_PARSER_H

#include <stdlib.h>
#include "refac_lexer.h"
#include "ast.h"

typedef struct Parser {
    Lexer* lexer;
    Token* currentToken;
    Token* nextToken;
    ASTNode* ast;
    bool has_error;
} Parser;

Parser* init_parser(Lexer* lexer);
ASTNode* init_program();
ASTNode* parse_class(Parser* parser);
ASTNode* parse_class_var_dec(Parser* parser);
ASTNode* parse_subroutine_dec(Parser* parser);
ASTNode* parse_parameter_list(Parser* parser);
ASTNode* parse_subroutine_body(Parser* parser);
ASTNode* parse_var_dec(Parser* parser);
ASTNode* parse_statements(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_let_statement(Parser* parser);
ASTNode* parse_if_statement(Parser* parser);
ASTNode* parse_while_statement(Parser* parser);
ASTNode* parse_do_statement(Parser* parser);
ASTNode* parse_return_statement(Parser* parser);
ASTNode* parse_subroutine_call(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_term(Parser* parser);

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

