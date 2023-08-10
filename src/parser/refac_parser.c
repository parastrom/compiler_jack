#include "refac_parser.h"
#include <string.h>
#include <signal.h>


/**
 * @brief Initializes a parser.
 * 
 * @param lexer 
 * @return Parser* 
 */
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

ASTNode* create_ast_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)safer_malloc(sizeof(ASTNode));
    node->nodeType = type;

    switch (type) {
        case NODE_PROGRAM:
            node->data.program = safer_malloc(sizeof(ProgramNode));
            node->data.program->classes = vector_create();
            break;
        case NODE_CLASS:
            node->data.classDec = safer_malloc(sizeof(ClassNode));
            node->data.classDec->className = NULL;
            node->data.classDec->classVarDecs = vector_create();
            node->data.classDec->subroutineDecs = vector_create();
            break;
        case NODE_CLASS_VAR_DEC:
            node->data.classVarDec = safer_malloc(sizeof(ClassVarDecNode));
            node->data.classVarDec->classVarModifier = CVAR_NONE;
            node->data.classVarDec->varType = NULL;
            node->data.classVarDec->varNames = vector_create();
            break;
        case NODE_SUBROUTINE_DEC:
            node->data.subroutineDec = safer_malloc(sizeof(SubroutineDecNode));
            node->data.subroutineDec->subroutineType = SUB_NONE;
            node->data.subroutineDec->returnType = NULL;
            node->data.subroutineDec->subroutineName = NULL;
            node->data.subroutineDec->parameters = NULL;
            node->data.subroutineDec->body = NULL;
            break;
        case NODE_PARAMETER_LIST:
            node->data.parameterList = safer_malloc(sizeof(ParameterListNode));
            node->data.parameterList->parameterTypes = vector_create();
            node->data.parameterList->parameterNames = vector_create();
            break;
        case NODE_SUBROUTINE_BODY:
            node->data.subroutineBody = safer_malloc(sizeof(SubroutineBodyNode));
            node->data.subroutineBody->varDecs = vector_create();
            node->data.subroutineBody->statements = NULL;
            break;
        case NODE_VAR_DEC:
            node->data.varDec = safer_malloc(sizeof(VarDecNode));
            node->data.varDec->varType = NULL;
            node->data.varDec->varNames = vector_create();
            break;
        case NODE_STATEMENTS:
            node->data.statements = safer_malloc(sizeof(StatementsNode));
            node->data.statements->statements = vector_create();
            break;
        case NODE_STATEMENT:
            node->data.statement = safer_malloc(sizeof(StatementNode));
            node->data.statement->statementType = STMT_NONE;
            node->data.statement->data.letStatement = NULL;
            node->data.statement->data.ifStatement = NULL;
            node->data.statement->data.whileStatement = NULL;
            node->data.statement->data.doStatement = NULL;
            node->data.statement->data.returnStatement = NULL;
            break;
        case NODE_LET_STATEMENT:
            node->data.letStatement = safer_malloc(sizeof(LetStatementNode));
            node->data.letStatement->varName = NULL;
            node->data.letStatement->indexExpression = NULL;
            node->data.letStatement->rightExpression = NULL;
            break;
        case NODE_IF_STATEMENT:
            node->data.ifStatement = safer_malloc(sizeof(IfStatementNode));
            node->data.ifStatement->condition = NULL;
            node->data.ifStatement->ifBranch = NULL;
            node->data.ifStatement->elseBranch = NULL;
            break;
        case NODE_WHILE_STATEMENT:
            node->data.whileStatement = safer_malloc(sizeof(WhileStatementNode));
            node->data.whileStatement->condition = NULL;
            node->data.whileStatement->body = NULL;
            break;
        case NODE_DO_STATEMENT:
            node->data.doStatement = safer_malloc(sizeof(DoStatementNode));
            node->data.doStatement->subroutineCall = NULL;
            break;
        case NODE_RETURN_STATEMENT:
            node->data.returnStatement = safer_malloc(sizeof(ReturnStatementNode));
            node->data.returnStatement->expression = NULL;
            break;
        case NODE_SUBROUTINE_CALL:
            node->data.subroutineCall = safer_malloc(sizeof(SubroutineCallNode));
            node->data.subroutineCall->caller = NULL;
            node->data.subroutineCall->subroutineName = NULL;
            node->data.subroutineCall->arguments = vector_create();
            break;
        case NODE_EXPRESSION:
            node->data.expression = safer_malloc(sizeof(ExpressionNode));
            node->data.expression->term = NULL;
            node->data.expression->operations = vector_create();
            break;
        case NODE_TERM:
            node->data.term = safer_malloc(sizeof(TermNode));
            node->data.term->termType = TRM_NONE;
            break;
        case NODE_OPERATION:
            node->data.operation = safer_malloc(sizeof(Operation));
            node->data.operation->op = 0;
            node->data.operation->term = NULL;
            break;
        case NODE_VAR_TERM:
            node->data.varTerm = safer_malloc(sizeof(VarTerm));
            node->data.varTerm->className = NULL;
            node->data.varTerm->varName = NULL;
            break;
        default:
            log_error(ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__, "Unknown node type: %d\n", type);
            exit(EXIT_FAILURE);
    }

    return node;
}

/**
 * @brief Initializes a program node.
 * 
 * @return ProgramNode* 
 */
ASTNode* init_program() {
    ASTNode* node = create_ast_node(NODE_PROGRAM);

    return node;
}

/**
 * @brief Parses a class.
 * 
 * @param parser 
 * @return ClassNode 
 */
ASTNode* parse_class(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_CLASS);

    log_message(LOG_LEVEL_INFO, "Parsing class\n");

    ringbuffer_pop(parser->lexer->queue, &parser->currentToken);

    expect_and_consume(parser, TOKEN_TYPE_CLASS);

    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.classDec->className = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
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

/**
 * @brief Parses a class variable declaration.
 * 
 * @param parser 
 * @return ClassVarDecNode 
 */
ASTNode* parse_class_var_dec(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_CLASS_VAR_DEC);

    log_message(LOG_LEVEL_INFO, "Parsing class variable declaration. Current Token : %s, Line : %d\n", token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    // Parse the class variable modifier
    if (parser->currentToken->type == TOKEN_TYPE_STATIC) {
        node->data.classVarDec->classVarModifier = STATIC;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FIELD) {
        node->data.classVarDec->classVarModifier = FIELD;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s or %s, got %s", token_type_to_string(TOKEN_TYPE_STATIC), token_type_to_string(TOKEN_TYPE_FIELD), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }


    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.classVarDec->varType = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_push(node->data.classVarDec->varNames, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }


    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.classVarDec->varNames, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

/**
 * @brief Parses a subroutine declaration.
 * 
 * @param parser 
 * @return SubroutineDecNode 
 */
ASTNode* parse_subroutine_dec(Parser* parser) {
    
    ASTNode* node = create_ast_node(NODE_SUBROUTINE_DEC);

    log_message(LOG_LEVEL_INFO, "Parsing subroutine declaration. Current Token : %s, Line : %d\n", token_type_to_string(parser->currentToken->type), parser->currentToken->line);
    
    // Parse the subroutine type
    if (parser->currentToken->type == TOKEN_TYPE_CONSTRUCTOR) {
        node->data.subroutineDec->subroutineType = CONSTRUCTOR;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_FUNCTION) {
        node->data.subroutineDec->subroutineType = FUNCTION;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (parser->currentToken->type == TOKEN_TYPE_METHOD) {
        node->data.subroutineDec->subroutineType = METHOD;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, %s or %s, got %s", token_type_to_string(TOKEN_TYPE_CONSTRUCTOR), token_type_to_string(TOKEN_TYPE_FUNCTION), token_type_to_string(TOKEN_TYPE_METHOD), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the return type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.subroutineDec->returnType = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    }else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s or %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(TOKEN_TYPE_VOID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }
    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.subroutineDec->subroutineName = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
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

/**
 * @brief Parses a parameter list.
 * 
 * @param parser 
 * @return ParameterListNode 
 */

ASTNode* parse_parameter_list(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_PARAMETER_LIST);

    log_message(LOG_LEVEL_INFO, "Parsing parameter list. Current Token : %s, Line : %d\n", token_type_to_string(parser->currentToken->type), parser->currentToken->line);
   
    // Parse the first parameter
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {

        vector_push(node->data.parameterList->parameterNames, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);

         if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.parameterList->parameterNames, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }

        // Parse any additional parameters
        while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
            if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
                vector_push(node->data.parameterList->parameterTypes, strdup(parser->currentToken->lx));
                ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
            } else {
            
                log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
                parser->has_error = true;
            }
            if (parser->currentToken->type == TOKEN_TYPE_ID) {
                vector_push(node->data.parameterList->parameterNames, strdup(parser->currentToken->lx));
                ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
            } else {
                log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
                parser->has_error = true;
            }

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
ASTNode* parse_subroutine_body(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_SUBROUTINE_BODY);

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

/**
 * @brief Parses a variable declaration.
 * 
 * @param parser 
 * @return VarDecNode 
 */
ASTNode* parse_var_dec(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_VAR_DEC);

    log_message(LOG_LEVEL_INFO, "Parsing variable declaration. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_VAR);

    // Parse the variable type
    if (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_TYPE)) {
        node->data.varDec->varType = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_CATEGORY_TYPE), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the first variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        vector_push(node->data.varDec->varNames, strdup(parser->currentToken->lx));
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse any additional variable names
    while (parser->currentToken->type == TOKEN_TYPE_COMMA) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            vector_push(node->data.varDec->varNames, strdup(parser->currentToken->lx));
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
            parser->has_error = true;
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

/**
 * @brief Parses a statement.
 * 
 * @param parser 
 * @return StatementNode 
 */

ASTNode* parse_statements(Parser* parser) {
    
    ASTNode* node = create_ast_node(NODE_STATEMENTS);

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

    ASTNode* node = create_ast_node(NODE_STATEMENT);

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
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, %s, %s, %s or %s, got %s", 
            token_type_to_string(TOKEN_TYPE_LET), token_type_to_string(TOKEN_TYPE_DO), token_type_to_string(TOKEN_TYPE_IF), 
            token_type_to_string(TOKEN_TYPE_WHILE), token_type_to_string(TOKEN_TYPE_RETURN), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    return node;
}

ASTNode* parse_let_statement(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_LET_STATEMENT);

    log_message(LOG_LEVEL_INFO, "Parsing let statement. Current Token : %s, Line : %d\n", 
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);

    expect_and_consume(parser, TOKEN_TYPE_LET);

    // Parse the variable name
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.letStatement->varName = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", 
            token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
    }

    // Parse the expression
    if (parser->currentToken->type == TOKEN_TYPE_OPEN_BRACKET) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        node->data.letStatement->indexExpression = parse_expression(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACKET);
    }

    expect_and_consume(parser, TOKEN_TYPE_EQUAL);
    node->data.letStatement->rightExpression = parse_expression(parser);
    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}

ASTNode* parse_if_statement(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_IF_STATEMENT);

    expect_and_consume(parser, TOKEN_TYPE_IF);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_PAREN);
    node->data.ifStatement->condition = parse_expression(parser);

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);

    node->data.ifStatement->ifBranch = parse_statements(parser);
    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);

    if (parser->currentToken->type == TOKEN_TYPE_ELSE) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);
        node->data.ifStatement->elseBranch = parse_statements(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);
    }

    return node;
}

ASTNode* parse_while_statement(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_WHILE_STATEMENT);

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

    ASTNode* node = create_ast_node(NODE_DO_STATEMENT);

    expect_and_consume(parser, TOKEN_TYPE_DO);
    node->data.doStatement->subroutineCall = parse_subroutine_call(parser);
    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);

    return node;
}


ASTNode* parse_return_statement(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_RETURN_STATEMENT);

    expect_and_consume(parser, TOKEN_TYPE_RETURN);
    if (parser->currentToken->type != TOKEN_TYPE_SEMICOLON) {
        node->data.returnStatement->expression = parse_expression(parser);
    }

    expect_and_consume(parser, TOKEN_TYPE_SEMICOLON);
    return node;
}

ASTNode* parse_subroutine_call(Parser *parser) {

    ASTNode* node = create_ast_node(NODE_SUBROUTINE_CALL);

    // Parse the caller
    if (parser->currentToken->type == TOKEN_TYPE_ID) {
        node->data.subroutineCall->caller = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    }

    // Parse the subroutine name
    if (parser->currentToken->type == TOKEN_TYPE_PERIOD) {
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        if (parser->currentToken->type == TOKEN_TYPE_ID) {
            node->data.subroutineCall->subroutineName = strdup(parser->currentToken->lx);
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        } else {
            log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
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
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        }
    }

    expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    return node;
}

ASTNode *parse_expression(Parser *parser) {

    ASTNode* node = create_ast_node(NODE_EXPRESSION);
    node->data.expression->term = parse_term(parser);

    while (is_token_category(parser->currentToken->type, TOKEN_CATEGORY_UNARY | TOKEN_CATEGORY_ARITH 
        | TOKEN_CATEGORY_BOOLEAN | TOKEN_CATEGORY_RELATIONAL)) {

        ASTNode* op = create_ast_node(NODE_OPERATION);
    
        op->data.operation->op = parser->currentToken->lx[0]; // Assuming lx is the string representation of the token
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        op->data.operation->term = parse_term(parser);

        vector_push(node->data.expression->operations, op);
    }

    return node;
}

ASTNode* parse_term(Parser* parser) {

    ASTNode* node = create_ast_node(NODE_TERM);

    TokenType type = parser->currentToken->type;

    log_message(LOG_LEVEL_INFO, "Parsing term. Current Token : %s\n", token_to_string(parser->currentToken));

    if (type == TOKEN_TYPE_NUM) {
        // Parse an integer constant
        node->data.term->termType = INTEGER_CONSTANT;
        node->data.term->data.intValue = atoi(parser->currentToken->lx); // lx contains the string representation of the number
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (type == TOKEN_TYPE_STRING) {
        // Parse a string constant
        node->data.term->termType = STRING_CONSTANT;
        node->data.term->data.stringValue = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (type == TOKEN_TYPE_TRUE || type == TOKEN_TYPE_FALSE || type == TOKEN_TYPE_NULL || type == TOKEN_TYPE_THIS) {
        // Parse a keyword constant
        node->data.term->termType = KEYWORD_CONSTANT;
        node->data.term->data.keywordValue = strdup(parser->currentToken->lx);
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
    } else if (type == TOKEN_TYPE_ID) {
        Token* nextToken = ringbuffer_peek(parser->lexer->queue);

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
            ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
            nextToken = ringbuffer_peek(parser->lexer->queue);
            if (nextToken->type == TOKEN_TYPE_PERIOD) {
                // It's a className.varName
                node->data.term->data.varTerm.className = node->data.term->data.varTerm.varName;
                ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
                if (parser->currentToken->type == TOKEN_TYPE_ID) {
                    node->data.term->data.varTerm.varName = strdup(parser->currentToken->lx);
                } else {
                    log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Expected token %s, got %s", token_type_to_string(TOKEN_TYPE_ID), token_type_to_string(parser->currentToken->type));
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
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        node->data.term->data.expression = parse_expression(parser);
        expect_and_consume(parser, TOKEN_TYPE_CLOSE_PAREN);
    } else if (type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_TILDE) {
        // It's a unary operation
        node->data.term->termType = UNARY_OP;
        node->data.term->data.unaryOp.unaryOp = parser->currentToken->lx[0];
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        node->data.term->data.unaryOp.term = parse_term(parser);
    } else {
        log_error(ERROR_PARSER_UNEXPECTED_TOKEN, __FILE__, __LINE__, "Unexpected token in term: %s", token_type_to_string(parser->currentToken->type));
        parser->has_error = true;
        node->data.term->termType = TRM_NONE;
        ringbuffer_pop(parser->lexer->queue, &parser->currentToken);
        return NULL;
    }
    return node;
}
