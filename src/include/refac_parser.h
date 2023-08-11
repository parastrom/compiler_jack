#ifndef REFAC_PARSER_H
#define REFAC_PARSER_H

#include <stdlib.h>
#include "refac_lexer.h"
#include "ast.h"
#include "token.h"

typedef struct Parser {
    Lexer* lexer;
    Token* currentToken;
    Token* nextToken;
    ASTNode* ast;
    bool has_error;
    Arena* arena;
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

void destroy_parser(Parser* parser);


#endif // REFAC_PARSER_H

