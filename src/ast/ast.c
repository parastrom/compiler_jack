#include "ast.h"
#include "symbol.h"
#include <string.h>

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

void push_table(ASTVisitor* visitor, SymbolTable* table) {
    visitor->currentTable = table;
}

void pop_table(ASTVisitor* visitor) {
    if(visitor->currentTable && visitor->currentTable->parent) {
        visitor->currentTable = visitor->currentTable->parent;
    } else {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__,
                    "Unexpected NULL parent table");
        exit(EXIT_FAILURE);
    }
}

void build_class_node(ASTVisitor* visitor, ASTNode* node) {
    SymbolTable* classTable = create_table(SCOPE_CLASS, visitor->currentTable);
    Symbol* classSymbol = symbol_table_add(classTable, node->data.classDec->className,
                                           node->data.classDec->className, KIND_CLASS);
    classSymbol->childTable = classTable;

    push_table(visitor, classTable);

    for (int i = 0; i < vector_size(node->data.classDec->classVarDecs); i++) {
        ASTNode* classVarDecNode =  (ASTNode*) vector_get(node->data.classDec->classVarDecs, i);
        ast_node_accept(classVarDecNode, visitor);
    }
    for (int i = 0; i < vector_size(node->data.classDec->subroutineDecs); i++) {
        ASTNode* subroutineDecNode = (ASTNode*) vector_get(node->data.classDec->subroutineDecs, i);
        ast_node_accept(subroutineDecNode, visitor);
    }
    pop_table(visitor);
}

void build_class_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.classVarDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.classVarDec->varNames, i);
        if (symbol_table_lookup(visitor->currentTable, varName)) {
            log_error(ERROR_SEMANTIC_REDECLARED_SYMBOL, __FILE__, __LINE__,
                      "Variable %s already declared.", varName);
            exit(EXIT_FAILURE);
        }
        switch (node->data.classVarDec->classVarModifier) {
            case STATIC:
                (void) symbol_table_add(visitor->currentTable, varName, node->data.classVarDec->varType, KIND_STATIC);
                break;
            case FIELD:
                (void) symbol_table_add(visitor->currentTable, varName, node->data.classVarDec->varType, KIND_FIELD);
                break;
            default:
                log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__, "Invalid class var modifier");
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void build_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {
    SymbolTable* subroutineTable;
    Symbol* subSymbol;

    switch(node->data.subroutineDec->subroutineType) {
        case CONSTRUCTOR:
            subroutineTable = create_table(SCOPE_CONSTRUCTOR, visitor->currentTable);
            subSymbol = symbol_table_add(subroutineTable, node->data.subroutineDec->subroutineName,
                node->data.subroutineDec->returnType, KIND_CONSTRUCTOR);
            break;
        case METHOD:
            subroutineTable = create_table(SCOPE_METHOD, visitor->currentTable);
            subSymbol =symbol_table_add(subroutineTable, node->data.subroutineDec->subroutineName,
                node->data.subroutineDec->returnType, KIND_METHOD);
            break;
        case FUNCTION:
            subroutineTable = create_table(SCOPE_FUNCTION, visitor->currentTable);
            subSymbol = symbol_table_add(subroutineTable, node->data.subroutineDec->subroutineName,
                node->data.subroutineDec->returnType, KIND_FUNCTION);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_SUBROUTINE, __FILE__, __LINE__,
                      "Invalid subroutine type");
            exit(EXIT_FAILURE);
            break;
    }

    subSymbol->childTable = subroutineTable;
    push_table(visitor, subroutineTable);
    ast_node_accept(node->data.subroutineDec->parameters, visitor);
    ast_node_accept(node->data.subroutineDec->body, visitor);
    pop_table(visitor);
}

void build_parameter_list_node(ASTVisitor* visitor, ASTNode* node) {
    for(int i = 0; i < vector_size(node->data.parameterList->parameterTypes); i++) {
        const char* parameterType = (char*) vector_get(node->data.parameterList->parameterTypes, i);
        const char* parameterName = (char*) vector_get(node->data.parameterList->parameterNames, i);
        (void) symbol_table_add(visitor->currentTable, parameterName, parameterType, KIND_ARG);
    }
}


void build_subroutine_body_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.subroutineBody->varDecs); i++) {
        ASTNode* varDecNode = vector_get(node->data.subroutineBody->varDecs, i);
        ast_node_accept(varDecNode, visitor);
    }
}

void build_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.varDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.varDec->varNames, i);
        (void) symbol_table_add(visitor->currentTable, varName, node->data.varDec->varType, KIND_VAR);
    }
}


void analyze_class_node(ASTVisitor* visitor, ASTNode* node) {
    visitor->currentClassName = node->data.classDec->className;

    // Retrieve the class's symbol table
    Symbol* classSymbol = symbol_table_lookup(visitor->currentTable, node->data.classDec->className);
    if (!classSymbol || classSymbol->kind != KIND_CLASS) {
        log_error(ERROR_SEMANTIC_INVALID_KIND, __FILE__, __LINE__,
                  "Undefined class: %s", node->data.classDec->className);
        exit(EXIT_FAILURE);
    }

    // Switch to the class's symbol table
    push_table(visitor, classSymbol->childTable);

    // Analyze class_var_decs  & subroutine_decs
    for (int i = 0; i < vector_size(node->data.classDec->classVarDecs); i++) {
        ASTNode* classVarDecNode =  (ASTNode*) vector_get(node->data.classDec->classVarDecs, i);
        ast_node_accept(classVarDecNode, visitor);
    }

    for (int i = 0; i < vector_size(node->data.classDec->subroutineDecs); i++) {
        ASTNode* subroutineDecNode = (ASTNode*) vector_get(node->data.classDec->subroutineDecs, i);
        ast_node_accept(subroutineDecNode, visitor);
    }

    // Return to the parent scope
    pop_table(visitor);

    visitor->currentClassName = NULL;
}

void analyze_class_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.classVarDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.classVarDec->varNames, i);
        Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, varName);
        if(!type_is_valid(visitor, varSymbol->type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                      "Invalid type %s.", varSymbol->type->userDefinedType);
            exit(EXIT_FAILURE);
        }
    }
}

void analyze_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {
    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable,
                                            node->data.subroutineDec->subroutineName);
    if(!subSymbol || subSymbol->kind != KIND_METHOD || subSymbol->kind != KIND_CONSTRUCTOR
            || subSymbol->kind != KIND_FUNCTION) {
        log_error(ERROR_SEMANTIC_INVALID_KIND, __FILE__, __LINE__,
                  "Undefined subroutine: %s", node->data.subroutineDec->subroutineName);
        exit(EXIT_FAILURE);
    }

    push_table(visitor, subSymbol->childTable);
    ast_node_accept(node->data.subroutineDec->parameters, visitor);
    ast_node_accept(node->data.subroutineDec->body, visitor);
    pop_table(visitor);
}

void analyze_parameter_list_node(ASTVisitor* visitor, ASTNode* node) {
    for(int i = 0; i < vector_size(node->data.parameterList->parameterTypes); i++) {
        char* paramName = (char*) vector_get(node->data.parameterList->parameterNames, i);
        Symbol* paramSymbol = symbol_table_lookup(visitor->currentTable, paramName);
        if(!type_is_valid(visitor, paramSymbol->type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                      "Invalid type %s.", paramSymbol->type->userDefinedType);
            exit(EXIT_FAILURE);
        }
    }
}

void analyze_subroutine_body_node(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_var_dec_node(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_statements(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_let_statement(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_if_statement(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_while_statement(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_do_statement(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_return_statement(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_term(ASTVisitor* visitor, ASTNode* node) {
    TermNode* termNode = node->data.term;
    char* keyword = termNode->data.keywordValue;

    switch (termNode->termType)
    {
        case INTEGER_CONSTANT:
            termNode->type.basicType = TYPE_INT;
            termNode->type.userDefinedType = NULL;
            break;
        case STRING_CONSTANT:
            termNode->type.basicType = TYPE_STRING;
            termNode->type.userDefinedType = NULL;
            break;
        case KEYWORD_CONSTANT:
            if (strcmp(keyword, "true") == 0 || strcmp(keyword, "false") == 0) {
                termNode->type.basicType = TYPE_BOOLEAN;
                termNode->type.userDefinedType = NULL;
            } else if (strcmp(keyword, "null") == 0) {
                termNode->type.basicType = TYPE_NULL;
                termNode->type.userDefinedType = NULL;
            } else if (strcmp(keyword, "this") == 0) {
                termNode->type.basicType = TYPE_USER_DEFINED;
                termNode->type.userDefinedType = visitor->currentClassName; // Assuming you have this field in ASTVisitor
            } else {
                log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__, "Invalid keyword constant");
                exit(EXIT_FAILURE);
            }
            break;
        case VAR_TERM:
            analyze_var_term(visitor, node);
            break;
        case SUBROUTINE_CALL:
            analyze_subroutine_call(visitor, node);
            break;
        case EXPRESSION:
            analyze_expression(visitor, node);
            break;
        case UNARY_OP:
            analyze_unary_op(visitor, node);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__, "Invalid term type");
            exit(EXIT_FAILURE);
            break;
    }
}

bool type_is_valid(ASTVisitor* visitor, Type* type) {
    if(type->userDefinedType) {
        if (!symbol_table_lookup(visitor->currentTable, type->userDefinedType)) {
            return false;
        }
    }

    return true;
}
bool type_arithmetic_compat(Type type1, Type type2) {
    return type1.basicType == TYPE_INT && type2.basicType == TYPE_INT;
}

bool type_comparison_compat(Type type1, Type type2) {
    return (type1.basicType == TYPE_INT || type1.basicType == TYPE_CHAR) &&
        (type2.basicType == TYPE_INT || type2.basicType == TYPE_CHAR);
}

bool type_is_boolean(Type type) {
    return type.basicType == TYPE_BOOLEAN;
}


void analyze_expression(ASTVisitor* visitor, ASTNode* node) {
    analyze_term(visitor, node->data.expression->term);
    Type curType = node->data.expression->term->data.term->type;

    for (int i = 0; i < vector_size(node->data.expression->operations); i++) {
        ASTNode *opNode = (ASTNode *) vector_get(node->data.expression->operations, i);

        analyze_term(visitor, opNode->data.operation->term);
        Type nextType = opNode->data.operation->term->data.term->type;

        switch (opNode->data.operation->op) {
            case '+':
            case '-':
            case '*':
            case '/': // arithmetic
                if (!type_arithmetic_compat(curType, nextType)) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__, "Invalid type for arithmetic operation");
                    exit(EXIT_FAILURE);
                }
                curType.basicType = TYPE_INT;
                curType.userDefinedType = NULL;
                break;
            case '>':
            case '<':
            case '=':
                if (!type_comparison_compat(curType, nextType)) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                                "Invalid type for comparison operation");
                    exit(EXIT_FAILURE);
                }
                curType.basicType = TYPE_BOOLEAN;
                curType.userDefinedType = NULL;
                break;
            case '&':
            case '|':
                if (!type_is_boolean(curType) || !type_is_boolean(nextType)) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                                "Invalid type for boolean operation");
                    exit(EXIT_FAILURE);
                }
                curType.basicType = TYPE_BOOLEAN;
                curType.userDefinedType = NULL;
                break;
            default:
                log_error(ERROR_SEMANTIC_INVALID_OPERATION, __FILE__, __LINE__,
                            "Invalid operation");
                break;
        }
    }

    // Assign the resultant type of the expression to the node itself
    node->data.expression->type = curType;
}


void analyze_subroutine_call(ASTVisitor* visitor, ASTNode* node){
    SubroutineCallNode * subCall =  node->data.subroutineCall;

    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable, subCall->subroutineName);
    if (!subSymbol || !(subSymbol->kind == KIND_FUNCTION ||
        subSymbol->kind == KIND_CONSTRUCTOR || subSymbol->kind == KIND_METHOD)) {
        log_error(ERROR_SEMANTIC_INVALID_EXPRESSION, __FILE__, __LINE__,
                  "Undefined subroutine: %s", subCall->subroutineName);
        exit(EXIT_FAILURE);
    }


    if(subSymbol->kind == KIND_METHOD) {
        if (subCall->caller) {
            Symbol* callerSymbol = symbol_table_lookup(visitor->currentTable, subCall->caller);
            if (!callerSymbol ||
                strcmp(callerSymbol->type->userDefinedType, subSymbol->type->userDefinedType) != 0) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                              "Caller type mismatch for method: %s", subCall->subroutineName);
                    exit(EXIT_FAILURE);
            }
        }
    }

    SymbolTable* subroutineTable = subSymbol->table;
    vector args = get_symbols_of_kind(subroutineTable, KIND_ARG);

    //Check Arguments
    for(int i = 0; i < vector_size(subCall->arguments); i++) {
        ASTNode* arg = vector_get(subCall->arguments, i);
        analyze_expression(visitor, arg);
        Type argType = arg->data.expression->term->data.term->type;
        Symbol* expectedArgSymbol = vector_get(args, i);
        if (expectedArgSymbol->type->basicType != argType.basicType ||
            strcmp(expectedArgSymbol->type->userDefinedType, argType.userDefinedType) != 0) {
                log_error(ERROR_SEMANTIC_INVALID_ARGUMENT, __FILE__, __LINE__,
                          "Mismatched argument type for subroutine: %s", subCall->subroutineName);
                exit(EXIT_FAILURE);
        }
    }
}

void analyze_var_term(ASTVisitor* visitor, ASTNode* node) {
    VarTerm* term = node->data.varTerm;

    Symbol* termSymbol = symbol_table_lookup(visitor->currentTable, term->varName);
    if (!termSymbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, __FILE__, __LINE__,
                  "Undefined variable: %s", term->varName);
        exit(EXIT_FAILURE);
    }

    if (term->className) {
        Symbol* classSymbol = symbol_table_lookup(visitor->currentTable, term->className);
        Symbol* attributeOrMethod = symbol_table_lookup(classSymbol->childTable, term->varName);
        if (!attributeOrMethod) {
            log_error(ERROR_SEMANTIC_INVALID_TERM, __FILE__, __LINE__,
                      "Variable %s not a valid attribute or method of class %s", term->varName, term->className);
            exit(EXIT_FAILURE);
        }
    }
    node->data.term->type = *termSymbol->type;
}

void analyze_unary_op(ASTVisitor* visitor, ASTNode* node) {

    ASTNode* unaryOpTerm = node->data.term->data.unaryOp.term;
    analyze_term(visitor, unaryOpTerm);
    char op = node->data.term->data.unaryOp.unaryOp;
    Type type = unaryOpTerm->data.term->type;

    if (op == '~') {
        if (!type_is_boolean(type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                        "Invalid type for unary operation");
            exit(EXIT_FAILURE);
        }
    } else if (op == '-') {
        if (type.basicType != TYPE_INT) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, __FILE__, __LINE__,
                        "Invalid type for unary operation");
            exit(EXIT_FAILURE);
        }
    } else {
        log_error(ERROR_SEMANTIC_INVALID_OPERATION, __FILE__, __LINE__,
                    "Invalid unary operation");
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

