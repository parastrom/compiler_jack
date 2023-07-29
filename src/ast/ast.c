#include "ast.h"
#include "symbol.h"

void class_node_accept(ClassNode* node, ASTVisitor* visitor) {
    if(visitor->visit_class_node) {
        visitor->visit_class_node(node);
    }
}

void class_var_dec_node_accept(ClassVarDecNode* node, ASTVisitor* visitor) {
    if(visitor->visit_class_var_dec_node) {
        visitor->visit_class_var_dec_node(node);
    }
}

void subroutine_dec_node_accept(SubroutineDecNode* node, ASTVisitor* visitor) {
    if(visitor->visit_subroutine_dec_node) {
        visitor->visit_subroutine_dec_node(node);
    }
}

void parameter_list_node_accept(ParameterListNode* node, ASTVisitor* visitor) {
    if(visitor->visit_parameter_list_node) {
        visitor->visit_parameter_list_node(node);
    }
}

void subroutine_body_node_accept(SubroutineBodyNode* node, ASTVisitor* visitor) {
    if(visitor->visit_subroutine_body_node) {
        visitor->visit_subroutine_body_node(node);
    }
}

void var_dec_node_accept(VarDecNode* node, ASTVisitor* visitor) {
    if(visitor->visit_var_dec_node) {
        visitor->visit_var_dec_node(node);
    }
}

void statements_node_accept(StatementsNode* node, ASTVisitor* visitor) {
    if(visitor->visit_statements_node) {
        visitor->visit_statements_node(node);
    }
}

void statement_node_accept(StatementNode* node, ASTVisitor* visitor) {
    if(visitor->visit_statement_node) {
        visitor->visit_statement_node(node);
    }
}

void let_statement_node_accept(LetStatementNode* node, ASTVisitor* visitor) {
    if(visitor->visit_let_statement_node) {
        visitor->visit_let_statement_node(node);
    }
}

void if_statement_node_accept(IfStatementNode* node, ASTVisitor* visitor) {
    if(visitor->visit_if_statement_node) {
        visitor->visit_if_statement_node(node);
    }
}

void while_statement_node_accept(WhileStatementNode* node, ASTVisitor* visitor) {
    if(visitor->visit_while_statement_node) {
        visitor->visit_while_statement_node(node);
    }
}

void do_statement_node_accept(DoStatementNode* node, ASTVisitor* visitor) {
    if(visitor->visit_do_statement_node) {
        visitor->visit_do_statement_node(node);
    }
}

void return_statement_node_accept(ReturnStatementNode* node, ASTVisitor* visitor) {
    if(visitor->visit_return_statement_node) {
        visitor->visit_return_statement_node(node);
    }
}

void subroutine_call_node_accept(SubroutineCallNode* node, ASTVisitor* visitor) {
    if(visitor->visit_subroutine_call_node) {
        visitor->visit_subroutine_call_node(node);
    }
}

void expression_node_accept(ExpressionNode* node, ASTVisitor* visitor) {
    if(visitor->visit_expression_node) {
        visitor->visit_expression_node(node);
    }
}

void operation_accept(Operation* node, ASTVisitor* visitor) {
    if(visitor->visit_operation) {
        visitor->visit_operation(node);
    }
}

void term_node_accept(TermNode* node, ASTVisitor* visitor) {
    if (visitor->visit_term_node) {
        visitor->visit_term_node(node);
    }
}

void visit_class_node(ASTVisitor* visitor, ClassNode* node) {
    SymbolTable* table = create_table(SCOPE_CLASS, visitor->currentTable);
    visitor->currentTable = table;

    symbol_table_add(table, node->className, node->className, KIND_CLASS);

    // visit classVarDecs
    for (int i = 0; i < vector_size(node->classVarDecs); i++) {
        ClassVarDecNode* classVarDecNode =  (ClassVarDecNode*) vector_get(node->classVarDecs, i);
        class_var_dec_node_accept(classVarDecNode, visitor);
    }

    // Visit subroutine declarations
    for (int i = 0; i < vector_size(node->subroutineDecs); i++) {
        SubroutineDecNode* subroutineDecNode = (SubroutineDecNode*) vector_get(node->subroutineDecs, i);
        subroutine_dec_node_accept(subroutineDecNode, visitor);
    }

    visitor->currentTable = visitor->currentTable->parent;
}

void visit_class_var_dec_node(ASTVisitor* visitor, ClassVarDecNode* node) {
    for (int i = 0; i < vector_size(node->varNames); i++) {
        char* varName = (char*) vector_get(node->varNames, i);
        switch (node->classVarModifier) {
            case STATIC:
                symbol_table_add(visitor->currentTable, varName, node->varType, KIND_STATIC);
                break;
            case FIELD:
                symbol_table_add(visitor->currentTable, varName, node->varType, KIND_FIELD);
                break;
            default:
                log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__, "Invalid class var modifier");
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void visit_subroutine_dec_node(ASTVisitor* visitor, SubroutineDecNode* node) {
    
    SymbolTable* table;

    switch(node->subroutineType) {
        case CONSTRUCTOR:
            table = create_table(SCOPE_CONSTRUCTOR, visitor->currentTable);
            symbol_table_add(table, node->subroutineName, node->returnType, KIND_CONSTRUCTOR);
            break;
        case METHOD:
            table = create_table(SCOPE_METHOD, visitor->currentTable);
            symbol_table_add(table, node->subroutineName, node->returnType, KIND_METHOD);
            break;
        case FUNCTION:
            table = create_table(SCOPE_FUNCTION, visitor->currentTable);
            symbol_table_add(table, node->subroutineName, node->returnType, KIND_FUNCTION);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_SUBROUTINE, __FILE__, __LINE__, "Invalid subroutine type");
            exit(EXIT_FAILURE);
            break;
    }

    visitor->currentTable = table;
   
    parameter_list_node_accept(node->parameters, visitor);

    subroutine_body_node_accept(node->body, visitor);

    visitor->currentTable = visitor->currentTable->parent;
}


/**
 * @brief Visits a parameter list node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_parameter_list_node(ASTVisitor* visitor, ParameterListNode* node) {
    for (int i = 0; i < vector_size(node->parameterTypes); i++) {
        char* parameterType = (char*) vector_get(node->parameterTypes, i);
        char* parameterName = (char*) vector_get(node->parameterNames, i);
        symbol_table_add(visitor->currentTable, parameterName, parameterType, KIND_ARG);
    }
}

/**
 * @brief Visits a subroutine body node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_subroutine_body_node(ASTVisitor* visitor, SubroutineBodyNode* node) {
    for (int i = 0; i < vector_size(node->varDecs); i++) {
        VarDecNode* varDecNode = (VarDecNode*) vector_get(node->varDecs, i);
        var_dec_node_accept(varDecNode, visitor);
    }

    statements_node_accept(node->statements, visitor);
}

/**
 * @brief Visits a var dec node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_var_dec_node(ASTVisitor* visitor, VarDecNode* node) {
    for (int i = 0; i < vector_size(node->varNames); i++) {
        char* varName = (char*) vector_get(node->varNames, i);
        symbol_table_add(visitor->currentTable, varName, node->varType, KIND_VAR);
    }
}

/**
 * @brief Visits a statements node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_statements_node(ASTVisitor* visitor, StatementsNode* node) {
    for (int i = 0; i < vector_size(node->statements); i++) {
        StatementNode* statementNode = (StatementNode*) vector_get(node->statements, i);
        statement_node_accept(statementNode, visitor);
    }
}

/**headers
 * @brief Visits a statement node.
 * Visits the statement based on its type.
 * 
 * @param visitor 
 * @param node 
 */
void visit_statement_node(ASTVisitor* visitor, StatementNode* node) {
    switch(node->statementType) {
        case LET:
            let_statement_node_accept(node->data.letStatement, visitor);
            break;
        case IF:
            if_statement_node_accept(node->data.ifStatement, visitor);
            break;
        case WHILE:
            while_statement_node_accept(node->data.whileStatement, visitor);
            break;
        case DO:
            do_statement_node_accept(node->data.doStatement, visitor);
            break;
        case RETURN:
            return_statement_node_accept(node->data.returnStatement, visitor);
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
void visit_let_statement_node(ASTVisitor* visitor, LetStatementNode* node) {

    Symbol* symbol = symbol_table_lookup(visitor->currentTable, node->varName);
    if (!symbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, "Variable %s not declared", node->varName);
        exit(EXIT_FAILURE);
    }

    if (node->indexExpression) {
        expression_node_accept(node->indexExpression, visitor);
    }

    expression_node_accept(node->rightExpression, visitor);
}

/**
 * @brief Visits an if statement node.
 * Visits the condition expression, the if branch, and the else branch (if it exists).
 * 
 * @param visitor 
 * @param node 
 */
void visit_if_statement_node(ASTVisitor* visitor, IfStatementNode* node) {
    expression_node_accept(node->condition, visitor);

    statements_node_accept(node->ifBranch, visitor);

    if (node->elseBranch) {
        statements_node_accept(node->elseBranch, visitor);
    }
}

/**
 * @brief Visits a while statement node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_while_statement_node(ASTVisitor* visitor, WhileStatementNode* node) {
    // Visit the condition expression
    expression_node_accept(node->condition, visitor);

    // Visit the body
    statements_node_accept(node->body, visitor);
}

/**
 * @brief Visits a do statement node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_do_statement_node(ASTVisitor* visitor, DoStatementNode* node) {
    subroutine_call_node_accept(node->subroutineCall, visitor);
}


/**
 * @brief Visits a return statement node.
 * If there is a return expression, visit it.
 * 
 * @param visitor 
 * @param node 
 */
void visit_return_statement_node(ASTVisitor* visitor, ReturnStatementNode* node) {

    if (node->expression) {
        expression_node_accept(node->expression, visitor);
    }
}

/**
 * @brief Visits a subroutine call node.
 * Visits each argument.
 * 
 * @param visitor 
 * @param node 
 */
void visit_subroutine_call_node(ASTVisitor* visitor, SubroutineCallNode* node) {
    // Check if the called subroutine exists in the symbol table
    Symbol* symbol = symbol_table_lookup(visitor->currentTable, node->subroutineName);
    if (!symbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, "Undefined subroutine: %s", node->subroutineName);
        exit(EXIT_FAILURE);
    }

    // Visit each argument
    for (int i = 0; i < vector_size(node->arguments); i++) {
        ExpressionNode* argument = (ExpressionNode*) vector_get(node->arguments, i);
        expression_node_accept(argument, visitor);
    }
}


/**
 * @brief Visits an expression node.
 * Visits the first term, then each operation.
 * 
 * @param visitor 
 * @param node 
 */
void visit_expression_node(ASTVisitor* visitor, ExpressionNode* node) {
    // Visit the first term
    term_node_accept(node->term, visitor);

    // Visit each operation
    for (int i = 0; i < vector_size(node->operations); i++) {
        Operation* operation = (Operation*) vector_get(node->operations, i);
        operation_accept(operation, visitor);
    }
}


/**
 * @brief Visits a term node.
 * 
 * @param visitor 
 * @param node 
 */
void visit_operation(ASTVisitor* visitor, Operation* node) {
    // Visit the term of the operation
    term_node_accept(node->term, visitor);
}


/**
 * @brief Visits a term node.
 * Visits the term based on its type.
 * 
 * @param visitor 
 * @param node 
 */
void visit_term_node(ASTVisitor* visitor, TermNode* node) {
    switch(node->termType) {
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
                Symbol* symbol = symbol_table_lookup(visitor->currentTable, node->data.varTerm.varName);
                if (!symbol) {
                    log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, "Undefined variable: %s", node->data.varTerm.varName);
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case ARRAY_ACCESS:
            {
                // Look up the array in the symbol table
                Symbol* symbol = symbol_table_lookup(visitor->currentTable, node->data.arrayAccess.arrayName);
                if (!symbol) {
                    log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__, "Undefined array: %s", node->data.arrayAccess.arrayName);
                    exit(EXIT_FAILURE);
                }

                expression_node_accept(node->data.arrayAccess.index, visitor);
            }
            break;
        case SUBROUTINE_CALL:
            visit_subroutine_call_node(visitor, node->data.subroutineCall);
            break;
        case EXPRESSION:
            visit_expression_node(visitor, node->data.expression);
            break;
        case UNARY_OP:
            visit_term_node(visitor, node->data.unaryOp.term);
            break;
        default:
            log_error(ERROR_INVALID_INPUT, __FILE__, __LINE__, "Invalid term type");
            exit(EXIT_FAILURE);
            break;
    }
}

