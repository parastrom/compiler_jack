#include "refac_parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


/**
 * @brief Initializes a parser.
 * 
 * @param lexer 
 * @return Parser* 
 */
Parser* init_parser(Stack* stack, Arena* arena) {
    Parser* parser = malloc(sizeof(Parser));
    
    if(!parser) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for Parser");
        return NULL;
    }

    if (stack == NULL) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__, "Lexer is NULL");
        free(parser);
        return NULL;
    }

    parser->arena = arena;
    parser->stack = stack;
    parser->currentToken = NULL;
    parser->has_error = false;

    return parser;
}

void expect_and_consume(Parser* parser, TokenType expected) {
    if (parser->currentToken->type != expected) {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", token_type_to_string(expected),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    } else {
       stack_pop(parser->stack, (void**) &parser->currentToken);
    }
}

ASTNode* parse_class(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_CLASS, parser->arena);
    log_message(LOG_LEVEL_INFO, "Parsing class\n");

    stack_pop(parser->stack, (void**) &parser->currentToken);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;
    expect_and_consume(parser, TOKEN_TYPE_CLASS);

    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.classDec->className = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_CLASS_VAR | TOKEN_CATEGORY_SUBROUTINE_DEC)) {
        if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_CLASS_VAR)) {
            ASTNode* classVarDec = parse_class_var_dec(parser);
            vector_push(node->data.classDec->classVarDecs, classVarDec);
        } else if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_SUBROUTINE_DEC)) {
            ASTNode* subroutineDec = parse_subroutine_dec(parser);
            vector_push(node->data.classDec->subroutineDecs, subroutineDec);
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    return node;
}

ASTNode* parse_class_var_dec(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_CLASS_VAR_DEC, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing class variable declaration. Current Token : %s, Line : %d\n",
                token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    // Parse the class variable modifier
    if (parser->currentToken->type == TOKEN_TYPE_STATIC) {
        node->data.classVarDec->classVarModifier = STATIC;
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FIELD) {
        node->data.classVarDec->classVarModifier = FIELD;
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s or %s, got %s", token_type_to_string(TOKEN_TYPE_STATIC),
                  token_type_to_string(TOKEN_TYPE_FIELD), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }


    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.classVarDec->varType = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", category_to_string(TOKEN_CATEGORY_TYPE),
                    token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_push(node->data.classVarDec->varNames, strdup(parser->currentToken->lx));
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }


    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        stack_pop(parser->stack, (void**) &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.classVarDec->varNames, strdup(parser->currentToken->lx));
            stack_pop(parser->stack, (void**) &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                      "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_subroutine_dec(Parser* parser) {
    
    ASTNode* node = init_ast_node(NODE_SUBROUTINE_DEC, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;
    log_message(LOG_LEVEL_INFO, "Parsing subroutine declaration. Current Token : %s, Line : %d\n",
                token_type_to_string(parser->currentToken->type), parser->currentToken->line);
    
    // Parse the subroutine type
    if (parser->currentToken->type == TOKEN_TYPE_CONSTRUCTOR) {
        node->data.subroutineDec->subroutineType = CONSTRUCTOR;
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FUNCTION) {
        node->data.subroutineDec->subroutineType = FUNCTION;
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_METHOD) {
        node->data.subroutineDec->subroutineType = METHOD;
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                    "Expected token category, got %s", category_to_string(TOKEN_CATEGORY_SUBROUTINE_DEC),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the return type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.subroutineDec->returnType = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    }else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", category_to_string(TOKEN_CATEGORY_TYPE),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }
    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.subroutineDec->subroutineName = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }
    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);
    // Parse the parameter list
    node->data.subroutineDec->parameters = parse_parameter_list(parser);
    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    // Parse the subroutine body
    node->data.subroutineDec->body = parse_subroutine_body(parser);
    return node;
}

ASTNode* parse_parameter_list(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_PARAMETER_LIST, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing parameter list. Current Token : %s, Line : %d\n",
                    token_type_to_string(parser->currentToken->type), parser->currentToken->line);
   
    // Parse the first parameter
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {

        vector_push(node->data.parameterList->parameterNames, strdup(parser->currentToken->lx));
        stack_pop(parser->stack, (void**) &parser->currentToken);

         if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.parameterList->parameterNames, strdup(parser->currentToken->lx));
            stack_pop(parser->stack, (void**) &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                      "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }

        // Parse any additional parameters
        while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
            stack_pop(parser->stack, (void**) &parser->currentToken);
            if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
                vector_push(node->data.parameterList->parameterTypes, strdup(parser->currentToken->lx));
                stack_pop(parser->stack, (void**) &parser->currentToken);
            } else {
            
                log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                          "Expected token %s, got %s", category_to_string(TOKEN_CATEGORY_TYPE),
                                token_type_to_string(parser->currentToken->type));
                parser->has_error = true;
            }
            if (parser->currentToken->type == TOKEN_TYPE_ID) {
                vector_push(node->data.parameterList->parameterNames, strdup(parser->currentToken->lx));
                stack_pop(parser->stack, (void**) &parser->currentToken);
            } else {
                log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                          "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                                token_type_to_string(parser->currentToken->type));
                parser->has_error = true;
            }

        }
    }

    return node;
}

ASTNode* parse_subroutine_body(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_SUBROUTINE_BODY, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing subroutine body. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    // Parse any variable declarations
    while (parser->currentToken->type == TOKEN_TYPE_VAR) {
        ASTNode* varDec = parse_var_dec(parser);
        vector_push(node->data.subroutineBody->varDecs, varDec);
    }

    // Parse the statements
    node->data.subroutineBody->statements = parse_statements(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    return node;
}

ASTNode* parse_var_dec(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_VAR_DEC, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing variable declaration. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_VAR);

    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.varDec->varType = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", category_to_string(TOKEN_CATEGORY_TYPE),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_push(node->data.varDec->varNames, strdup(parser->currentToken->lx));
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        stack_pop(parser->stack, (void**) &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.varDec->varNames, strdup(parser->currentToken->lx));
            stack_pop(parser->stack, (void**) &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                      "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                            token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_statements(Parser* parser) {
    
    ASTNode* node = init_ast_node(NODE_STATEMENTS, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing statements. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    // Parse any statements
    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_STATEMENT)) {
        ASTNode* statement = parse_statement(parser);
        vector_push(node->data.statements->statements, statement);
    }

    return node;
}

ASTNode* parse_statement(Parser* parser) {


    ASTNode* node = init_ast_node(NODE_STATEMENT, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing statement. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    if (parser->currentToken->type == TOKEN_TYPE_LET) {
        node->data.statement->statementType = LET;
        node->data.statement->data.letStatement = parse_let_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_DO) {
        node->data.statement->statementType = DO;
        node->data.statement->data.doStatement = parse_do_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_IF) {
        node->data.statement->statementType = IF;
        node->data.statement->data.ifStatement = parse_if_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_WHILE) {
        node->data.statement->statementType = WHILE;
        node->data.statement->data.whileStatement = parse_while_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_RETURN) {
        node->data.statement->statementType = RETURN;
        node->data.statement->data.returnStatement = parse_return_statement(parser);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token category %s, got %s", category_to_string(TOKEN_CATEGORY_STATEMENT),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    return node;
}

ASTNode* parse_let_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_LET_STATEMENT, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    log_message(LOG_LEVEL_INFO, "Parsing let statement. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_LET);

    // Parse the variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.letStatement->varName = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                        token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the expression
    if (parser->currentToken->type == TOKEN_TYPE_OPEN_BRACKET) {
        stack_pop(parser->stack, (void**) &parser->currentToken);
        node->data.letStatement->indexExpression = parse_expression(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACKET);
    }

    expect_and_consume(parser, TOKEN_TYPE_EQUAL);
    node->data.letStatement->rightExpression = parse_expression(parser);
    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_if_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_IF_STATEMENT, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    expect_and_consume(parser, TOKEN_TYPE_IF);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);
    node->data.ifStatement->condition = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    node->data.ifStatement->ifBranch = parse_statements(parser);
    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    if (parser->currentToken->type == TOKEN_TYPE_ELSE) {
        stack_pop(parser->stack, (void**) &parser->currentToken);
        expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);
        node->data.ifStatement->elseBranch = parse_statements(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);
    }

    return node;
}

ASTNode* parse_while_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_WHILE_STATEMENT, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    expect_and_consume(parser, TOKEN_TYPE_WHILE);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);

    node->data.whileStatement->condition = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    node->data.whileStatement->body = parse_statements(parser);
    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    return node;
}

ASTNode* parse_do_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_DO_STATEMENT, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    expect_and_consume(parser, TOKEN_TYPE_DO);
    node->data.doStatement->subroutineCall = parse_subroutine_call(parser);
    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_return_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_RETURN_STATEMENT, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    expect_and_consume(parser, TOKEN_TYPE_RETURN);
    if (parser->currentToken->type != TOKEN_TYPE_SEMICOLON) {
        node->data.returnStatement->expression = parse_expression(parser);
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);
    return node;
}

ASTNode* parse_subroutine_call(Parser *parser) {

    ASTNode* node = init_ast_node(NODE_SUBROUTINE_CALL, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    // Parse the caller
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.subroutineCall->caller = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    }

    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_PERIOD) {
        stack_pop(parser->stack, (void**) &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            node->data.subroutineCall->subroutineName = strdup(parser->currentToken->lx);
            stack_pop(parser->stack, (void**) &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                      "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                            token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    } else {
        // If there is no period after the ID, it is a subroutine call without a caller
        node->data.subroutineCall->subroutineName = node->data.subroutineCall->caller;
        node->data.subroutineCall->caller = NULL;
    }

    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);

    // Parse the expression list
    while (parser->currentToken->type != TOKEN_TYPE_CLOSE_PAREN) {
        ASTNode* expression = parse_expression(parser);
        vector_push(node->data.subroutineCall->arguments, expression);

        if (parser->currentToken->type == TOKEN_TYPE_COMMA) {
            stack_pop(parser->stack, (void**) &parser->currentToken);
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    return node;
}

ASTNode *parse_expression(Parser *parser) {

    ASTNode* node = init_ast_node(NODE_EXPRESSION, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;
    node->data.expression->term = parse_term(parser);

    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_UNARY | TOKEN_CATEGORY_ARITH
        | TOKEN_CATEGORY_BOOLEAN | TOKEN_CATEGORY_RELATIONAL)) {

        ASTNode* op = init_ast_node(NODE_OPERATION, parser->arena);
    
        op->data.operation->op = parser->currentToken->lx[0]; // Assuming lx is the string representation of the token
        stack_pop(parser->stack, (void**) &parser->currentToken);
        op->data.operation->term = parse_term(parser);

        vector_push(node->data.expression->operations, op);
    }

    return node;
}

ASTNode* parse_term(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_TERM, parser->arena);
    node->filename = parser->currentToken->filename;
    node->line = parser->currentToken->line;

    TokenType type = parser->currentToken->type;

    log_message(LOG_LEVEL_INFO, "Parsing term. Current Token : %s\n",
                token_to_string(parser->currentToken));

    if (type == TOKEN_TYPE_NUM) {
        // Parse an integer constant
        node->data.term->termType = INTEGER_CONSTANT;
        node->data.term->data.intValue = atoi(parser->currentToken->lx); // lx contains the string representation of the number
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else if (type == TOKEN_TYPE_STRING) {
        // Parse a string constant
        node->data.term->termType = STRING_CONSTANT;
        node->data.term->data.stringValue = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else if (type == TOKEN_TYPE_TRUE || type == TOKEN_TYPE_FALSE || type == TOKEN_TYPE_NULL || type == TOKEN_TYPE_THIS) {
        // Parse a keyword constant
        node->data.term->termType = KEYWORD_CONSTANT;
        node->data.term->data.keywordValue = strdup(parser->currentToken->lx);
        stack_pop(parser->stack, (void**) &parser->currentToken);
    } else if (type == TOKEN_TYPE_ID) {
        Token* nextToken = (Token*) stack_peek(parser->stack);

        if (nextToken->type == TOKEN_TYPE_OPEN_BRACKET) {
            // It's an array access
            node->data.term->termType = ARRAY_ACCESS;
            node->data.term->data.arrayAccess.arrayName = strdup(parser->currentToken->lx);

            expect_and_consume(parser, TOKEN_TYPE_ID);
            expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACKET);
            // Parse the expression inside the brackets
            node->data.term->data.arrayAccess.index = parse_expression(parser);
            // Expect and consume the ']'
            expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACKET);
        } else if (nextToken->type == TOKEN_TYPE_OPEN_PAREN || nextToken->type == TOKEN_TYPE_PERIOD) {
            node->data.term->termType = SUBROUTINE_CALL;
            node->data.term->data.subroutineCall = parse_subroutine_call(parser);
        } else {
            // It's just a varName or a className.varName
            node->data.term->termType = VAR_TERM;
            node->data.term->data.varTerm.varName = strdup(parser->currentToken->lx);
            stack_pop(parser->stack, (void**) &parser->currentToken);
            nextToken =  (Token*) stack_peek(parser->stack);
            if (nextToken->type == TOKEN_TYPE_PERIOD) {
                // It's a className.varName
                node->data.term->data.varTerm.className = node->data.term->data.varTerm.varName;
                stack_pop(parser->stack, (void**) &parser->currentToken);
                if (parser->currentToken->type == TOKEN_TYPE_ID) {
                    node->data.term->data.varTerm.varName = strdup(parser->currentToken->lx);
                } else {
                    log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID),
                                    token_type_to_string(parser->currentToken->type));
                    parser->has_error = true;
                }
            } else {
                // It's just a varName.
                node->data.term->data.varTerm.className = NULL;
            }
        } 
    } else if (type == TOKEN_TYPE_OPEN_PAREN) {
        // It's an expression inside parentheses
        node->data.term->termType = EXPRESSION;
        stack_pop(parser->stack, (void**) &parser->currentToken);
        node->data.term->data.expression = parse_expression(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    } else if (type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_TILDE) {
        // It's a unary operation
        node->data.term->termType = UNARY_OP;
        node->data.term->data.unaryOp.unaryOp = parser->currentToken->lx[0];
        stack_pop(parser->stack, (void**) &parser->currentToken);
        node->data.term->data.unaryOp.term = parse_term(parser);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                  "Unexpected token in term: %s", token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
        node->data.term->termType = TRM_NONE;
        stack_pop(parser->stack, (void**) &parser->currentToken);
        return NULL;
    }
    return node;
}
