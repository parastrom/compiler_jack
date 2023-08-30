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
Parser* init_parser(TokenQueue* queue, vector line_offsets, Arena* arena) {
    Parser* parser = arena_alloc(arena, sizeof(Parser));
    
    if(!parser) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for the parser", __func__);
        return NULL;
    }

    if (queue == NULL) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : NULL pointer instead of valid queue pointer", __func__);
        free(parser);
        return NULL;
    }
    parser->line_offsets = line_offsets;
    parser->arena = arena;
    parser->queue = queue;
    parser->currentToken = NULL;
    parser->has_error = false;

    return parser;
}

void destroy_parser(Parser* parser) {

    /*
     *  parser->arena will live longer than the parser
     *  parser->currentToken - pointer into a element from the queue // does not need to be manually freed
     *  parser itself arena allocated dies when arena is destroyed
     */
     vector_destroy(parser->line_offsets);
}

void expect_and_consume(Parser* parser, TokenType expected) {
    if (parser->currentToken->type != expected) {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(expected),
                              token_type_to_string(parser->currentToken->type)
                              );
        parser->has_error = true;
    } else {
        queue_pop(parser->queue, &parser->currentToken);
    }
}

ASTNode* parse_class(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_CLASS, parser->arena);
    log_message(LOG_LEVEL_DEBUG, ERROR_NONE, "Parsing class\n");

    queue_pop(parser->queue, &parser->currentToken);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, parser->currentToken->line);
    expect_and_consume(parser, TOKEN_TYPE_CLASS);

    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.classDec->className = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
            "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
            token_type_to_string(parser->currentToken->type)
        );
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

    log_message(LOG_LEVEL_INFO, ERROR_NONE,
                "Current token : %s\n", token_to_string(parser->currentToken));
    log_message(LOG_LEVEL_INFO, ERROR_NONE, "idx : [%d], queue size : [%d]\n",
                parser->queue->idx, vector_size(parser->queue->list));

    if (parser->currentToken->type != TOKEN_TYPE_CLOSE_BRACE) {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line - 2),
            "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_CLOSE_BRACE),
            token_type_to_string(parser->currentToken->type)
        );
    }

    return node;
}

ASTNode* parse_class_var_dec(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_CLASS_VAR_DEC, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing class variable declaration. Current Token : %s, Line : %d\n",
                token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    // Parse the class variable modifier
    if (parser->currentToken->type == TOKEN_TYPE_STATIC) {
        node->data.classVarDec->classVarModifier = STATIC;
        queue_pop(parser->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FIELD) {
        node->data.classVarDec->classVarModifier = FIELD;
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected category > '%s', instead received > '%s'",
                              token_category_to_string(TOKEN_CATEGORY_CLASS_VAR),
                              token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }


    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.classVarDec->varType = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected category > '%s', instead received > '%s'",
                              token_category_to_string(TOKEN_CATEGORY_TYPE),
                              token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_push(node->data.classVarDec->varNames, arena_strdup(parser->arena,parser->currentToken->lx));
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
            "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
            token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }


    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        queue_pop(parser->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.classVarDec->varNames, arena_strdup(parser->arena,parser->currentToken->lx));
            queue_pop(parser->queue, &parser->currentToken);
        } else {
            log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
                token_type_to_string(parser->currentToken->type)
            );
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_subroutine_dec(Parser* parser) {
    
    ASTNode* node = init_ast_node(NODE_SUBROUTINE_DEC, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);
    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing subroutine declaration. Current Token : %s, Line : %d\n",
                token_type_to_string(parser->currentToken->type), parser->currentToken->line);
    
    // Parse the subroutine type
    if (parser->currentToken->type == TOKEN_TYPE_CONSTRUCTOR) {
        node->data.subroutineDec->subroutineType = CONSTRUCTOR;
        queue_pop(parser->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FUNCTION) {
        node->data.subroutineDec->subroutineType = FUNCTION;
        queue_pop(parser->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_METHOD) {
        node->data.subroutineDec->subroutineType = METHOD;
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected category > '%s', instead received > '%s'",
                              token_category_to_string(TOKEN_CATEGORY_SUBROUTINE_DEC),
                              token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }

    // Parse the return type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.subroutineDec->returnType = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    }else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected category > '%s', instead received > '%s'",
                              token_category_to_string(TOKEN_CATEGORY_TYPE),
                              token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }
    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.subroutineDec->subroutineName = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
            "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
            token_type_to_string(parser->currentToken->type)
        );
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
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing parameter list. Current Token : %s, Line : %d\n",
                    token_type_to_string(parser->currentToken->type), parser->currentToken->line);
   
    // Parse the first parameter
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {

        vector_push(node->data.parameterList->parameterTypes, arena_strdup(parser->arena,parser->currentToken->lx));
        queue_pop(parser->queue, &parser->currentToken);

         if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.parameterList->parameterNames, arena_strdup(parser->arena,parser->currentToken->lx));
            queue_pop(parser->queue, &parser->currentToken);
        } else {
             log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
                token_type_to_string(parser->currentToken->type)
             );
            parser->has_error = true;
        }

        // Parse any additional parameters
        while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
            queue_pop(parser->queue, &parser->currentToken);
            if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
                vector_push(node->data.parameterList->parameterTypes, arena_strdup(parser->arena,parser->currentToken->lx));
                queue_pop(parser->queue, &parser->currentToken);
            } else {
                log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                                      (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                                      "['%s'] : Expected category type > '%s', instead received > '%s'",
                                      token_category_to_string(TOKEN_CATEGORY_TYPE),
                                      token_type_to_string(parser->currentToken->type)
                );
                parser->has_error = true;
            }
            if (parser->currentToken->type == TOKEN_TYPE_ID) {
                vector_push(node->data.parameterList->parameterNames, arena_strdup(parser->arena,parser->currentToken->lx));
                queue_pop(parser->queue, &parser->currentToken);
            } else {
                log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                    (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                    "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
                     token_type_to_string(parser->currentToken->type)
                );
                parser->has_error = true;
            }

        }
    }

    return node;
}

ASTNode* parse_subroutine_body(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_SUBROUTINE_BODY, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing subroutine body. Current Token : %s, Line : %d\n",
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
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing variable declaration. Current Token : %s, Line : %d\n",
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_VAR);

    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.varDec->varType = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected category > '%s', instead received > '%s'",
                              token_category_to_string(TOKEN_CATEGORY_TYPE),
                              token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_push(node->data.varDec->varNames, arena_strdup(parser->arena,parser->currentToken->lx));
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
            "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
            token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }

    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        queue_pop(parser->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.varDec->varNames, arena_strdup(parser->arena,parser->currentToken->lx));
            queue_pop(parser->queue, &parser->currentToken);
        } else {
            log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
                token_type_to_string(parser->currentToken->type)
            );
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_statements(Parser* parser) {
    
    ASTNode* node = init_ast_node(NODE_STATEMENTS, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing statements. Current Token : %s, Line : %d\n",
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
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing statement. Current Token : %s, Line : %d\n",
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
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                              (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                              "['%s'] : Expected category > '%s', instead received > '%s'",
                              token_category_to_string(TOKEN_CATEGORY_STATEMENT),
                              token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }

    return node;
}

ASTNode* parse_let_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_LET_STATEMENT, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing let statement. Current Token : %s, Line : %d\n",
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_LET);

    // Parse the variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.letStatement->varName = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
            "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
            token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
    }

    // Parse the expression
    if (parser->currentToken->type == TOKEN_TYPE_OPEN_BRACKET) {
        queue_pop(parser->queue, &parser->currentToken);
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
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    expect_and_consume(parser, TOKEN_TYPE_IF);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);
    node->data.ifStatement->condition = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    node->data.ifStatement->ifBranch = parse_statements(parser);
    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    if (parser->currentToken->type == TOKEN_TYPE_ELSE) {
        queue_pop(parser->queue, &parser->currentToken);
        expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);
        node->data.ifStatement->elseBranch = parse_statements(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);
    }

    return node;
}

ASTNode* parse_while_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_WHILE_STATEMENT, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

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
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    expect_and_consume(parser, TOKEN_TYPE_DO);
    node->data.doStatement->subroutineCall = parse_subroutine_call(parser);
    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_return_statement(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_RETURN_STATEMENT, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    expect_and_consume(parser, TOKEN_TYPE_RETURN);
    if (parser->currentToken->type != TOKEN_TYPE_SEMICOLON) {
        node->data.returnStatement->expression = parse_expression(parser);
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);
    return node;
}

ASTNode* parse_subroutine_call(Parser *parser) {

    ASTNode* node = init_ast_node(NODE_SUBROUTINE_CALL, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    // Parse the caller
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.subroutineCall->caller = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    }

    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_PERIOD) {
        queue_pop(parser->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            node->data.subroutineCall->subroutineName = arena_strdup(parser->arena,parser->currentToken->lx);
            queue_pop(parser->queue, &parser->currentToken);
        } else {
            log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
                token_type_to_string(parser->currentToken->type)
            );
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
            queue_pop(parser->queue, &parser->currentToken);
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    return node;
}

ASTNode *parse_expression(Parser *parser) {

    ASTNode* node = init_ast_node(NODE_EXPRESSION, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    node->data.expression->term = parse_term(parser);

    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_UNARY | TOKEN_CATEGORY_ARITH
        | TOKEN_CATEGORY_BOOLEAN | TOKEN_CATEGORY_RELATIONAL)) {

        ASTNode* op = init_ast_node(NODE_OPERATION, parser->arena);
    
        op->data.operation->op = parser->currentToken->lx[0]; // Assuming lx is the string representation of the token
        queue_pop(parser->queue, &parser->currentToken);
        op->data.operation->term = parse_term(parser);

        vector_push(node->data.expression->operations, op);
    }

    return node;
}

ASTNode* parse_term(Parser* parser) {

    ASTNode* node = init_ast_node(NODE_TERM, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    TokenType type = parser->currentToken->type;

    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing term. Current Token : %s\n",
                token_to_string(parser->currentToken));

    if (type == TOKEN_TYPE_NUM) {
        // A integer constant
        node->data.term->termType = INTEGER_CONSTANT;
        node->data.term->data.intValue = atoi(parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else if (type == TOKEN_TYPE_STRING) {
        // A string constant
        node->data.term->termType = STRING_CONSTANT;
        node->data.term->data.stringValue = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else if (type == TOKEN_TYPE_TRUE || type == TOKEN_TYPE_FALSE || type == TOKEN_TYPE_NULL || type == TOKEN_TYPE_THIS) {
        // A keyword constant
        node->data.term->termType = KEYWORD_CONSTANT;
        node->data.term->data.keywordValue = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    } else if (type == TOKEN_TYPE_ID) {
        Token* nextToken = (Token*) queue_peek(parser->queue);

        if (nextToken->type == TOKEN_TYPE_OPEN_BRACKET) {
            // It's an array access
            node->data.term->termType = ARRAY_ACCESS;
            node->data.term->data.arrayAccess.arrayName = arena_strdup(parser->arena,parser->currentToken->lx);

            expect_and_consume(parser, TOKEN_TYPE_ID);
            expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACKET);
            node->data.term->data.arrayAccess.index = parse_expression(parser);
            expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACKET);
        } else if (nextToken->type == TOKEN_TYPE_PERIOD) {
            Token* secondPeek = (Token*) queue_peek_offset(parser->queue, 1);

            if (secondPeek->type == TOKEN_TYPE_ID) {
                 Token* thirdPeek = (Token*) queue_peek_offset(parser->queue, 2);
                 if (thirdPeek->type == TOKEN_TYPE_OPEN_PAREN) {
                     node->data.term->termType = SUBROUTINE_CALL;
                     node->data.term->data.subroutineCall = parse_subroutine_call(parser);
                 } else {
                     node->data.term->termType = VAR_TERM;
                     node->data.term->data.varTerm = parse_var_term(parser);
                 }
            } else {
                log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                    (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                    "['%s'] : Unexpected token after period > '%s'",
                    token_type_to_string(secondPeek->type)
                );
                parser->has_error = true;
            }
        } else {
            node->data.term->termType = VAR_TERM;
            node->data.term->data.varTerm  = parse_var_term(parser);
        }
    } else if (type == TOKEN_TYPE_OPEN_PAREN) {
        // It's an expression inside parentheses
        node->data.term->termType = EXPRESSION;
        queue_pop(parser->queue, &parser->currentToken);
        node->data.term->data.expression = parse_expression(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    } else if (type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_TILDE) {
        // It's a unary operation
        node->data.term->termType = UNARY_OP;
        node->data.term->data.unaryOp.unaryOp = parser->currentToken->lx[0];
        queue_pop(parser->queue, &parser->currentToken);
        node->data.term->data.unaryOp.term = parse_term(parser);
    } else {
        log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
            (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
            "['%s'] : Unexpected token in term > '%s'", token_type_to_string(parser->currentToken->type)
        );
        parser->has_error = true;
        node->data.term->termType = TRM_NONE;
        queue_pop(parser->queue, &parser->currentToken);
        return NULL;
    }
    return node;
}


ASTNode* parse_var_term(Parser* parser) {
    ASTNode* node = init_ast_node(NODE_VAR_TERM, parser->arena);
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);
    node->line = parser->currentToken->line;
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);

    char* possibleClassName;
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        possibleClassName = arena_strdup(parser->arena,parser->currentToken->lx);
        queue_pop(parser->queue, &parser->currentToken);
    }

    if (parser->currentToken->type == TOKEN_TYPE_PERIOD) {
        node->data.varTerm->className = possibleClassName;
        queue_pop(parser->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            node->data.varTerm->varName = arena_strdup(parser->arena,parser->currentToken->lx);
            queue_pop(parser->queue, &parser->currentToken);
        } else {
            log_error_with_offset(ERROR_PHASE_PARSER, ERROR_PARSER_UNEXPECTED_TOKEN, parser->currentToken->filename, parser->currentToken->line,
                (size_t) vector_get(parser->line_offsets, parser->currentToken->line),
                "['%s'] : Expected token > '%s', instead received > '%s'", token_type_to_string(TOKEN_TYPE_ID),
                token_type_to_string(parser->currentToken->type)
            );
            parser->has_error = true;
        }
    } else {
        node->data.varTerm->varName = possibleClassName;
    }

    return node;
}