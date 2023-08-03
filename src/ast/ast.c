#include "ast.h"
#include "symbol.h"

void ast_node_accept(ASTNode* node, ASTVisitor* visitor) {
    if(visitor->visit_ast_node) {
        visitor->visit_ast_node(node);
    }
}

void visit_ast_node(ASTVisitor* visitor, ASTNode* node) {
    switch (node->nodeType) {
        case NODE_CLASS:
            visit_class_node(visitor, node);
            break;
        case NODE_CLASS_VAR_DEC:
            visit_class_var_dec_node(visitor, node);
            break;
        case NODE_SUBROUTINE_DEC:
            visit_subroutine_dec_node(visitor, node);
            break;
        case NODE_PARAMETER_LIST:
            visit_parameter_list_node(visitor, node);
            break;
        case NODE_SUBROUTINE_BODY:
            visit_subroutine_body_node(visitor, node);
            break;
        case NODE_VAR_DEC:
            visit_var_dec_node(visitor, node);
            break;
        case NODE_STATEMENTS:
            visit_statements_node(visitor, node);
            break;
        case NODE_STATEMENT:
            visit_statement_node(visitor, node);
            break;
        case NODE_LET_STATEMENT:
            visit_let_statement_node(visitor, node);
            break;
        case NODE_IF_STATEMENT:
            visit_if_statement_node(visitor, node);
            break;
        case NODE_WHILE_STATEMENT:
            visit_while_statement_node(visitor, node);
            break;
        case NODE_DO_STATEMENT:
            visit_do_statement_node(visitor, node);
            break;
        case NODE_RETURN_STATEMENT:
            visit_return_statement_node(visitor, node);
            break;
        case NODE_SUBROUTINE_CALL:
            visit_subroutine_call_node(visitor, node);
            break;
        case NODE_EXPRESSION:
            visit_expression_node(visitor, node);
            break;
        case NODE_TERM:
            visit_term_node(visitor, node);
            break;
        case NODE_OPERATION:
            visit_operation(visitor, node);
            break;
        default:
            log_error(ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__, "Unknown node type: %d\n", node->nodeType);
            exit(EXIT_FAILURE);
    }
}

void visit_class_node(ASTVisitor* visitor, ASTNode* node) {
    SymbolTable* table = create_table(SCOPE_CLASS, visitor->currentTable);
    visitor->currentTable = table;

    symbol_table_add(table, node->data.classDec->className, node->data.classDec->className, KIND_CLASS);

    // visit classVarDecs
    for (int i = 0; i < vector_size(node->data.classDec->classVarDecs); i++) {
        ASTNode* classVarDecNode =  (ASTNode*) vector_get(node->data.classDec->classVarDecs, i);
        ast_node_accept(classVarDecNode, visitor);
    }

    // Visit subroutine declarations
    for (int i = 0; i < vector_size(node->data.classDec->subroutineDecs); i++) {
        ASTNode* subroutineDecNode = (ASTNode*) vector_get(node->data.classDec->subroutineDecs, i);
        ast_node_accept(subroutineDecNode, visitor);
    }

    visitor->currentTable = visitor->currentTable->parent;
}

void visit_class_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.classVarDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.classVarDec->varNames, i);
        switch (node->data.classVarDec->classVarModifier) {
            case STATIC:
                symbol_table_add(visitor->currentTable, varName, node->data.classVarDec->varType, KIND_STATIC);
                break;
            case FIELD:
                symbol_table_add(visitor->currentTable, varName, node->data.classVarDec->varType, KIND_FIELD);
                break;
            default:
                log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__, "Invalid class var modifier");
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void visit_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {

    SymbolTable* table;

    switch(node->data.subroutineDec->subroutineType) {
        case CONSTRUCTOR:
            table = create_table(SCOPE_CONSTRUCTOR, visitor->currentTable);
            symbol_table_add(table, node->data.subroutineDec->subroutineName, 
                node->data.subroutineDec->returnType, KIND_CONSTRUCTOR);
            break;
        case METHOD:
            table = create_table(SCOPE_METHOD, visitor->currentTable);
            symbol_table_add(table, node->data.subroutineDec->subroutineName, 
                node->data.subroutineDec->returnType, KIND_METHOD);
            break;
        case FUNCTION:
            table = create_table(SCOPE_FUNCTION, visitor->currentTable);
            symbol_table_add(table, node->data.subroutineDec->subroutineName, 
                node->data.subroutineDec->returnType, KIND_FUNCTION);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_SUBROUTINE, __FILE__, __LINE__, "Invalid subroutine type");
            exit(EXIT_FAILURE);
            break;
    }

    visitor->currentTable = table;
    ast_node_accept(node->data.subroutineDec->parameters, visitor);
    ast_node_accept(node->data.subroutineDec->body, visitor);
    visitor->currentTable = visitor->currentTable->parent;
}

/**
 * @brief Visits a parameter list node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_parameter_list_node(ASTVisitor* visitor, ASTNode* node) {
    for(int i = 0; i < vector_size(node->data.parameterList->parameterTypes); i++) {
        const char* parameterType = (char*) vector_get(node->data.parameterList->parameterTypes, i);
        const char* parameterName = (char*) vector_get(node->data.parameterList->parameterNames, i);
        symbol_table_add(visitor->currentTable, parameterName, parameterType, KIND_ARG);
    }
}

/**
 * @brief Visits a subroutine body node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_subroutine_body_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.subroutineBody->varDecs); i++) {
        ASTNode* varDecNode = vector_get(node->data.subroutineBody->varDecs, i);
        ast_node_accept(varDecNode, visitor);
    }

    ast_node_accept(node->data.subroutineBody->statements, visitor);
}

/**
 * @brief Visits a var dec node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.varDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.varDec->varNames, i);
        symbol_table_add(visitor->currentTable, varName, node->data.varDec->varType, KIND_VAR);
    }
}

/**
 * @brief Visits a statements node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_statements_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.statements->statements); i++) {
        ASTNode* statementNode = vector_get(node->data.statements->statements, i);
        ast_node_accept(statementNode, visitor);
    }
}

/**headers
 * @brief Visits a statement node.
 * Visits the statement based on its type.
 * 
 * @param visitor 
 * @param node 
 */
void visit_statement_node(ASTVisitor* visitor, ASTNode* node) {
    switch(node->data.statement->statementType) {
        case LET:
            ast_node_accept(node->data.statement->data.letStatement, visitor);
            break;
        case IF:
            ast_node_accept(node->data.statement->data.ifStatement, visitor);
            break;
        case WHILE:
            ast_node_accept(node->data.statement->data.whileStatement, visitor);
            break;
        case DO:
            ast_node_accept(node->data.statement->data.doStatement, visitor);
            break;
        case RETURN:
            ast_node_accept(node->data.statement->data.returnStatement, visitor);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_STATEMENT, __FILE__, __LINE__, "Invalid statement type");
            exit(EXIT_FAILURE);
            break;
    }
}

/**
 * @brief Visits a let statement node.
 * Visits the index expression and the right expression.
 * 
 * @param visitor 
 * @param node 
 */
void visit_let_statement_node(ASTVisitor* visitor, ASTNode* node) {

    const Symbol* symbol = symbol_table_lookup(visitor->currentTable, node->data.letStatement->varName);
    if (!symbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, "Variable %s not declared", node->data.letStatement->varName);
        exit(EXIT_FAILURE);
    }

    if (node->data.letStatement->indexExpression) {
        ast_node_accept(node->data.letStatement->indexExpression, visitor);
    }
    ast_node_accept(node->data.letStatement->rightExpression, visitor);
}


/**
 * @brief Visits an if statement node.
 * Visits the condition expression, the if branch, and the else branch (if it exists).
 * 
 * @param visitor 
 * @param node 
 */
void visit_if_statement_node(ASTVisitor* visitor, ASTNode* node) {
    ast_node_accept(node->data.ifStatement->condition, visitor);

    ast_node_accept(node->data.ifStatement->ifBranch, visitor);

    if (node->data.ifStatement->elseBranch) {
        ast_node_accept(node->data.ifStatement->elseBranch, visitor);
    }
}

/**
 * @brief Visits a while statement node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_while_statement_node(ASTVisitor* visitor, ASTNode* node) {
    // Visit the condition expression
    ast_node_accept(node->data.whileStatement->condition, visitor);

    // Visit the body
    ast_node_accept(node->data.whileStatement->body, visitor);
}

/**
 * @brief Visits a do statement node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_do_statement_node(ASTVisitor* visitor, ASTNode* node) {
    ast_node_accept(node->data.doStatement->subroutineCall, visitor);
}


/**
 * @brief Visits a return statement node.
 * If there is a return expression, visit it.
 * 
 * @param visitor 
 * @param node 
 */
void visit_return_statement_node(ASTVisitor* visitor, ASTNode* node) {
    if (node->data.returnStatement->expression) {
        ast_node_accept(node->data.returnStatement->expression, visitor);
    }
}

/**
 * @brief Visits a subroutine call node.
 * Visits each argument.
 * 
 * @param visitor 
 * @param node 
 */
void visit_subroutine_call_node(ASTVisitor* visitor, ASTNode* node) {
    
    const Symbol* symbol = symbol_table_lookup(visitor->currentTable, node->data.subroutineCall->subroutineName);
    if (!symbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, 
        "Undefined subroutine: %s", node->data.subroutineCall->subroutineName);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < vector_size(node->data.subroutineCall->arguments); i++) {
        ASTNode* argument = vector_get(node->data.subroutineCall->arguments, i);
        ast_node_accept(argument, visitor);
    }
}


/**
 * @brief Visits an expression node.
 * Visits the first term, then each operation.
 * 
 * @param visitor 
 * @param node 
 */
void visit_expression_node(ASTVisitor* visitor, ASTNode* node) {
    // Visit the first term
    ast_node_accept(node->data.expression->term, visitor);

    // Visit each operation
    for (int i = 0; i < vector_size(node->data.expression->operations); i++) {
        ASTNode* operation = vector_get(node->data.expression->operations, i);
        ast_node_accept(operation, visitor);
    }
}


/**
 * @brief Visits a term node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_operation(ASTVisitor* visitor, ASTNode* node) {
    // Visit the term of the operation
    ast_node_accept(node->data.operation->term, visitor);
}


/**
 * @brief Visits a term node.
 * Visits the term based on its type.
 * 
 * @param visitor 
 * @param node 
 */
void visit_term_node(ASTVisitor* visitor, ASTNode* node) {
    switch(node->data.term->termType) {
        case INTEGER_CONSTANT:
            ;
            break;
        case STRING_CONSTANT:
            ;
            break;
        case KEYWORD_CONSTANT:
            ;
            break;
        case VAR_TERM:
            {
                // Look up the variable in the symbol table
                const Symbol* symbol = symbol_table_lookup(visitor->currentTable, 
                    node->data.term->data.varTerm.varName);
                if (!symbol) {
                    log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, 
                        "Undefined variable: %s", node->data.term->data.varTerm.varName);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case ARRAY_ACCESS:
            {
                // Look up the array in the symbol table
                const Symbol* symbol = symbol_table_lookup(visitor->currentTable, 
                    node->data.term->data.arrayAccess.arrayName);
                if (!symbol) {
                    log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, 
                        "Undefined array: %s", node->data.term->data.arrayAccess.arrayName);
                    exit(EXIT_FAILURE);
                }

                ast_node_accept(node->data.term->data.arrayAccess.index, visitor);
            }
            break;
        case SUBROUTINE_CALL:
            visit_subroutine_call_node(visitor, node->data.term->data.subroutineCall);
            break;
        case EXPRESSION:
            visit_expression_node(visitor, node->data.term->data.expression);
            break;
        case UNARY_OP:
            visit_term_node(visitor, node->data.term->data.unaryOp.term);
            break;
        default:
            log_error(ERROR_INVALID_INPUT, __FILE__, __LINE__, "Invalid term type");
            exit(EXIT_FAILURE);
            break;
    }
}

