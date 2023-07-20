#include "headers/refac_parser.h"
#include <string.h>

Parser* init_parser(Lexer* lexer) {
    Parser* parser = malloc(sizeof(Parser));
    
    if(!parser) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for Parser");
        return NULL;
    }

    if (lexer == NULL) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__, "Lexer is NULL");
        free(parser);
        return NULL;
    }

    parser->lexer = lexer;
    parser->currentToken = NULL;
    parser->nextToken = NULL;
    parser->has_error = false;

    return parser;
}

void expect_and_consume(Parser* parser, TokenType expected) {
    if (parser->currentToken->type != expected) {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(expected), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    } else {
       ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    }
}

ClassNode* parse_class(Parser* parser) {
    
    ClassNode* node = malloc(sizeof(ClassNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for ClassNode");
    }

    node->className = NULL;
    node->classVarDecs = vector_create();
    node->subroutineDecs = vector_create();

    ringbuffer_pop(parser->lexer->queue, &parser->currentToken);

    expect_and_consume(parser, TOKEN_TYPE_CLASS);

    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->className = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_CLASS_VAR | TOKEN_CATEGORY_SUBROUTINE_DEC)) {
        if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_CLASS_VAR)) {
            ClassVarDecNode* classVarDec = parse_class_var_dec(parser);
            vector_add(&node->classVarDecs, classVarDec);
        } else if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_SUBROUTINE_DEC)) {
            SubroutineDecNode* subroutineDec = parse_subroutine_dec(parser);
            vector_add(&node->subroutineDecs, subroutineDec);
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    return node;
}

/**
 * @brief Parses a class variable declaration.
 * 
 * @param parser 
 * @return ClassVarDecNode 
 */
ClassVarDecNode* parse_class_var_dec(Parser* parser) {

    ClassVarDecNode* node = malloc(sizeof(ClassVarDecNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for ClassVarDecNode");
    }

    node->varType = NULL;
    node->varNames = vector_create();

    // Parse the class variable modifier
    if (parser->currentToken->type == TOKEN_TYPE_STATIC) {
        node->classVarModifier = STATIC;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FIELD) {
        node->classVarModifier = FIELD;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s or %s, got %s", token_type_to_string(TOKEN_TYPE_STATIC), token_type_to_string(TOKEN_TYPE_FIELD), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }


    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->varType = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_add(&node->varNames, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }


    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_add(&node->varNames, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);
}

/**
 * @brief Parses a subroutine declaration.
 * 
 * @param parser 
 * @return SubroutineDecNode 
 */
SubroutineDecNode* parse_subroutine_dec(Parser* parser) {
    
        SubroutineDecNode* node = malloc(sizeof(SubroutineDecNode));
        if(!node) {
            log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for SubroutineDecNode");
        }
    
        node->subroutineType = NULL;
        node->returnType = NULL;
        node->subroutineName = NULL;
        node->parameters = NULL;
        node->body = NULL;
    
        // Parse the subroutine type
        if (parser->currentToken->type == TOKEN_TYPE_CONSTRUCTOR) {
            node->subroutineType = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else if (parser->currentToken->type == TOKEN_TYPE_FUNCTION) {
            node->subroutineType = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else if (parser->currentToken->type == TOKEN_TYPE_METHOD) {
            node->subroutineType = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, %s or %s, got %s", token_type_to_string(TOKEN_TYPE_CONSTRUCTOR), token_type_to_string(TOKEN_TYPE_FUNCTION), token_type_to_string(TOKEN_TYPE_METHOD), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    
        // Parse the return type
        if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
            node->returnType = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else if (parser->currentToken->type == TOKEN_TYPE_VOID) {
            node->returnType = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s or %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(TOKEN_TYPE_VOID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }

        // Parse the subroutine name
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            node->subroutineName = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }

        expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);
        // Parse the parameter list
        node->parameters = parse_parameter_list(parser);

        expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
        // Parse the subroutine body
        node->body = parse_subroutine_body(parser);

        return node;
}

/**
 * @brief Parses a parameter list.
 * 
 * @param parser 
 * @return ParameterListNode 
 */

ParameterListNode* parse_parameter_list(Parser* parser) {
    ParameterListNode* node = malloc(sizeof(ParameterListNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for ParameterListNode");
    }

    node->parameterTypes = vector_create();
    node->parameterNames = vector_create();

    // Parse the first parameter
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        vector_add(&node->parameterTypes, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_add(&node->parameterNames, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse any additional parameters
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
            vector_add(&node->parameterTypes, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }

        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_add(&node->parameterNames, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
        
    }

    return node;
}

/**
 * @brief Parses a subroutine body.
 * 
 * @param parser 
 * @return SubroutineBodyNode 
 */
SubroutineBodyNode* parse_subroutine_body(Parser* parser) {
    SubroutineBodyNode* node = malloc(sizeof(SubroutineBodyNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for SubroutineBodyNode");
    }

    node->varDecs = vector_create();
    node->statements = NULL;

    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    // Parse any variable declarations
    while (parser->currentToken->type == TOKEN_TYPE_VAR) {
        VarDecNode* varDec = parse_var_dec(parser);
        vector_add(&node->varDecs, varDec);
    }

    // Parse the statements
    node->statements = parse_statements(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    return node;
}

/**
 * @brief Parses a variable declaration.
 * 
 * @param parser 
 * @return VarDecNode 
 */
VarDecNode* parse_var_dec(Parser* parser) {

    VarDecNode* node = malloc(sizeof(VarDecNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for VarDecNode");
    }

    node->varType = NULL;
    node->varNames = vector_create();

    expect_and_consume(parser, TOKEN_TYPE_VAR);

    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->varType = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_add(&node->varNames, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_add(&node->varNames, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

StatementsNode* parse_statements(Parser* parser) {
    StatementsNode* node = malloc(sizeof(StatementsNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for StatementsNode");
    }

    node->statements = vector_create();

    // Parse any statements
    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_STATEMENT)) {
        StatementNode* statement = parse_statement(parser);
        vector_add(&node->statements, statement);
    }

    return node;
}

StatementNode* parse_statement(Parser* parser) {

    StatementNode* node = malloc(sizeof(StatementNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for StatementNode");
    }

    // Initialize the union
    node->data.letStatement = NULL;
    node->data.ifStatement = NULL;
    node->data.whileStatement = NULL;
    node->data.doStatement = NULL;
    node->data.returnStatement = NULL;

    if (parser->currentToken->type == TOKEN_TYPE_LET) {
        node->statementType = LET;
        node->data.letStatement = parse_let_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_DO) {
        node->statementType = DO;
        node->data.doStatement = parse_do_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_IF) {
        node->statementType = IF;
        node->data.ifStatement = parse_if_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_WHILE) {
        node->statementType = WHILE;
        node->data.whileStatement = parse_while_statement(parser);
    } else if (parser->currentToken->type == TOKEN_TYPE_RETURN) {
        node->statementType = RETURN;
        node->data.returnStatement = parse_return_statement(parser);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, %s, %s, %s or %s, got %s", token_type_to_string(TOKEN_TYPE_LET), token_type_to_string(TOKEN_TYPE_DO), token_type_to_string(TOKEN_TYPE_IF), token_type_to_string(TOKEN_TYPE_WHILE), token_type_to_string(TOKEN_TYPE_RETURN), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    return node;
}

LetStatementNode* parse_let_statement(Parser* parser) {
    LetStatementNode* node = malloc(sizeof(LetStatementNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for LetStatementNode");
    }

    node->varName = NULL;
    node->indexExpression = NULL;
    node->rightExpression = NULL;

    expect_and_consume(parser, TOKEN_TYPE_LET);

    // Parse the variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->varName = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the expression
    if (parser->currentToken->type == TOKEN_TYPE_OPEN_BRACKET) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        node->indexExpression = parse_expression(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACKET);
    }

    expect_and_consume(parser, TOKEN_TYPE_EQUAL);

    node->rightExpression = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

IfStatementNode* parse_if_statement(Parser* parser) {
    IfStatementNode* node = malloc(sizeof(IfStatementNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for IfStatementNode");
    }

    node->condition = NULL;
    node->ifBranch = NULL;
    node->elseBranch = NULL;

    expect_and_consume(parser, TOKEN_TYPE_IF);

    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);

    node->condition = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);

    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    node->ifBranch = parse_statements(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    if (parser->currentToken->type == TOKEN_TYPE_ELSE) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);

        expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

        node->elseBranch = parse_statements(parser);

        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);
    }

    return node;
}

WhileStatementNode* parse_while_statement(Parser* parser) {
    WhileStatementNode* node = malloc(sizeof(WhileStatementNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for WhileStatementNode");
    }

    node->condition = NULL;
    node->body = NULL;

    expect_and_consume(parser, TOKEN_TYPE_WHILE);

    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);

    node->condition = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);

    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    node->body = parse_statements(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    return node;
}

DoStatementNode* parse_do_statement(Parser* parser) {
    DoStatementNode* node = malloc(sizeof(DoStatementNode));
    
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for DoStatementNode");
    }

    node->caller = NULL;
    node->subroutineName = NULL;
    node->arguments = vector_create();

    expect_and_consume(parser, TOKEN_TYPE_DO);

    // Parse the caller
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->caller = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    }

    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_PERIOD) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            node->subroutineName = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    } else {
            // If there is no period after the ID, it is a subroutine call without a caller
            node->subroutineName = node->caller;
            node->caller = NULL;
    }

    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);

    // Parse the expression list
    while (parser->currentToken->type != TOKEN_TYPE_CLOSE_PAREN) {
        ExpressionNode* expression = parse_expression(parser);
        vector_add(&node->arguments, expression);

        // If the next token is a comma, consume it
        if (parser->currentToken->type == TOKEN_TYPE_COMMA) {
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;

}


ReturnStatementNode* parse_return_statement(Parser* parser) {
    ReturnStatementNode* node = malloc(sizeof(ReturnStatementNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for ReturnStatementNode");
    }

    node->expression = NULL;

    expect_and_consume(parser, TOKEN_TYPE_RETURN);

    if (parser->currentToken->type != TOKEN_TYPE_SEMICOLON) {
        node->expression = parse_expression(parser);
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ExpressionNode* parse_expression(Parser* parser) {
    ExpressionNode* node = malloc(sizeof(ExpressionNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for ExpressionNode");
    }

    node->term = NULL;

    node->term = parse_term(parser);

    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_UNARY | TOKEN_CATEGORY_ARITH 
    | TOKEN_CATEGORY_BOOLEAN | TOKEN_CATEGORY_RELATIONAL)) {
        Operation* op = malloc(sizeof(Operation));
        if (!op) {
            log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for Operation");
        }

        op->op = parser->currentToken->lx[0]; // Assuming lx is the string representation of the token
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);

        op->term = parse_term(parser);

        vector_add(&node->operations, op);
    }

    return node;
}

TermNode* parse_term(Parser* parser) {
    TermNode* node = malloc(sizeof(TermNode));
    if(!node) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for TermNode");
    }

    TokenType type = parser->currentToken->type;

    if (type == TOKEN_TYPE_NUM) {
        // Parse an integer constant
        node->termType = INTEGER_CONSTANT;
        node->data.intValue = atoi(parser->currentToken->lx); // Assuming lx contains the string representation of the number
    } else if (type == TOKEN_TYPE_STRING) {
        // Parse a string constant
        node->termType = STRING_CONSTANT;
        node->data.stringValue = strdup(parser->currentToken->lx);
    } else if (type == TOKEN_TYPE_TRUE || type == TOKEN_TYPE_FALSE || type == TOKEN_TYPE_NULL || type == TOKEN_TYPE_THIS) {
        // Parse a keyword constant
        node->termType = KEYWORD_CONSTANT;
        node->data.keywordValue = strdup(parser->currentToken->lx);
    } else if (type == TOKEN_TYPE_ID) {
    // The token is an identifier. It can be a varName, an array access, or a subroutine call.
    // We need to peek the next token to differentiate between these possibilities.
    Token* nextToken = ringbuffer_peek(parser->lexer->queue);

    if (nextToken->type == TOKEN_TYPE_OPEN_BRACKET) {
        // It's an array access
        node->termType = ARRAY_ACCESS;
        node->data.arrayAccess.arrayName = strdup(parser->currentToken->lx);
        // Consume the '['
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        // Parse the expression inside the brackets
        node->data.arrayAccess.index = parse_expression(parser);
        // Expect and consume the ']'
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACKET);
    } else if (nextToken->type == TOKEN_TYPE_OPEN_PAREN || nextToken->type == TOKEN_TYPE_PERIOD) {
        // It's a subroutine call
        node->termType = SUBROUTINE_CALL;
        // Assuming parse_subroutine_call takes care of consuming the tokens and returns a filled struct
        node->data.subroutineCall = parse_subroutine_call(parser);
    } else if (type == TOKEN_TYPE_ID) {
        // It's just a varName or a className.varName
        node->termType = VAR_TERM;
        node->data.varTerm.varName = strdup(parser->currentToken->lx);
        // Consume the varName
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        nextToken = ringbuffer_peek(parser->lexer->queue);
        if (nextToken->type == TOKEN_TYPE_PERIOD) {
            // It's a className.varName
            node->data.varTerm.className = node->data.varTerm.varName;
            // Consume the '.'
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
            // Parse the varName
            if (parser->currentToken->type == TOKEN_TYPE_ID) {
                node->data.varTerm.varName = strdup(parser->currentToken->lx);
            } else {
                log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
                parser->has_error = true;
            }
        } else {
            // It's just a varName
            node->data.varTerm.className = NULL;
        }
    } else if (type == TOKEN_TYPE_OPEN_PAREN) {
        // It's an expression inside parentheses
        node->termType = EXPRESSION;
        // Consume the '('
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        // Parse the expression inside the parentheses
        node->data.expression = parse_expression(parser);
        // Expect and consume the ')'
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    } else if (type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_TILDE) {
        // It's a unary operation
        node->termType = UNARY_OP;
        node->data.unaryOp.unaryOp = parser->currentToken->lx[0];
        // Consume the unary operator
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        // Parse the term
        node->data.unaryOp.term = parse_term(parser);

    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Unexpected token in term: %s", token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Move to the next token once we're done parsing the current one
    ringbuffer_pop(parser->lexer->queue, &parser->currentToken);

    return node;
}

