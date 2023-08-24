#include "ast.h"
#include <string.h>
#include "arena.h"

SemanticAnalyzer* init_semantic_analyzer(Arena* arena) {
    SemanticAnalyzer* analyzer = arena_alloc(arena, sizeof(SemanticAnalyzer));
    if (!analyzer) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for SemanticAnalyzer");
        return NULL;
    }

    analyzer->analyze_program_node = &analyze_program_node;
    analyzer->analyze_class_node = &analyze_class_node;
    analyzer->analyze_class_var_dec_node = &analyze_class_var_dec_node;
    analyzer->analyze_subroutine_dec_node = &analyze_subroutine_dec_node;
    analyzer->analyze_parameter_list_node = &analyze_parameter_list_node;
    analyzer->analyze_subroutine_body_node = &analyze_subroutine_body_node;
    analyzer->analyze_statements = &analyze_statements_node;
    analyzer->analyze_statement_node = &analyze_statement_node;
    analyzer->analyze_let_statement_node = &analyze_let_statement_node;
    analyzer->analyze_if_statement_node = &analyze_if_statement_node;
    analyzer->analyze_while_statement_node = &analyze_while_statement_node;
    analyzer->analyze_do_statement_node = &analyze_do_statement_node;
    analyzer->analyze_return_statement_node = &analyze_return_statement_node;
    analyzer->analyze_subroutine_call_node = &analyze_subroutine_call_node;
    analyzer->analyze_expression_node = &analyze_expression_node;
    analyzer->analyze_term_node = &analyze_term_node;
    analyzer->analyze_var_term_node = &analyze_var_term_node;

    return analyzer;
}


SymbolTableBuilder* init_symbol_table_builder(Arena* arena) {
    SymbolTableBuilder* builder = arena_alloc(arena, sizeof(SymbolTableBuilder));
    if (!builder) {
       log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for SymbolTableBuilder");
        return NULL;
    }

    builder->build_program_node = &build_program_node;
    builder->build_class_node = &build_class_node;
    builder->build_class_var_dec_node = &build_class_var_dec_node;
    builder->build_subroutine_dec_node = &build_subroutine_dec_node;
    builder->build_parameter_list_node = &build_parameter_list_node;
    builder->build_var_dec_node = &build_var_dec_node;
    builder->build_subroutine_body = &build_subroutine_body_node;

    return builder;
}


ASTVisitor* init_ast_visitor(Arena* arena, Phase initialPhase, SymbolTable* globalTable) {
    ASTVisitor* visitor = arena_alloc(arena, sizeof(ASTVisitor));
    if (!visitor) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for ASTVisitor");
        return NULL;
    }

    visitor->semanticAnalyzer = init_semantic_analyzer(arena);
    visitor->symbolTableBuilder = init_symbol_table_builder(arena);

    visitor->currentTable = globalTable;
    visitor->phase = initialPhase;
    visitor->currentClassName = NULL;
    visitor->arena = arena;

    return visitor;
}

ASTNode* init_ast_node(ASTNodeType type, Arena* arena) {
    ASTNode* node = (ASTNode*) arena_alloc(arena, sizeof(ASTNode));
    node->nodeType = type;

    switch (type) {
        case NODE_PROGRAM:
            node->data.program = (ProgramNode*) arena_alloc(arena,sizeof(ProgramNode));
            node->data.program->classes = vector_create();
            break;
        case NODE_CLASS:
            node->data.classDec = (ClassNode*) arena_alloc(arena,sizeof(ClassNode));
            node->data.classDec->className = NULL;
            node->data.classDec->classVarDecs = vector_create();
            node->data.classDec->subroutineDecs = vector_create();
            break;
        case NODE_CLASS_VAR_DEC:
            node->data.classVarDec = (ClassVarDecNode*) arena_alloc(arena,sizeof(ClassVarDecNode));
            node->data.classVarDec->classVarModifier = CVAR_NONE;
            node->data.classVarDec->varType = NULL;
            node->data.classVarDec->varNames = vector_create();
            break;
        case NODE_SUBROUTINE_DEC:
            node->data.subroutineDec = (SubroutineDecNode*) arena_alloc(arena,sizeof(SubroutineDecNode));
            node->data.subroutineDec->subroutineType = SUB_NONE;
            node->data.subroutineDec->returnType = NULL;
            node->data.subroutineDec->subroutineName = NULL;
            node->data.subroutineDec->parameters = NULL;
            node->data.subroutineDec->body = NULL;
            break;
        case NODE_PARAMETER_LIST:
            node->data.parameterList =  (ParameterListNode*) arena_alloc(arena,sizeof(ParameterListNode));
            node->data.parameterList->parameterTypes = vector_create();
            node->data.parameterList->parameterNames = vector_create();
            break;
        case NODE_SUBROUTINE_BODY:
            node->data.subroutineBody = (SubroutineBodyNode*) arena_alloc(arena,sizeof(SubroutineBodyNode));
            node->data.subroutineBody->varDecs = vector_create();
            node->data.subroutineBody->statements = NULL;
            break;
        case NODE_VAR_DEC:
            node->data.varDec = (VarDecNode*) arena_alloc(arena,sizeof(VarDecNode));
            node->data.varDec->varType = NULL;
            node->data.varDec->varNames = vector_create();
            break;
        case NODE_STATEMENTS:
            node->data.statements = (StatementsNode*) arena_alloc(arena,sizeof(StatementsNode));
            node->data.statements->statements = vector_create();
            break;
        case NODE_STATEMENT:
            node->data.statement = (StatementNode*) arena_alloc(arena,sizeof(StatementNode));
            node->data.statement->statementType = STMT_NONE;
            node->data.statement->data.letStatement = NULL;
            node->data.statement->data.ifStatement = NULL;
            node->data.statement->data.whileStatement = NULL;
            node->data.statement->data.doStatement = NULL;
            node->data.statement->data.returnStatement = NULL;
            break;
        case NODE_LET_STATEMENT:
            node->data.letStatement = (LetStatementNode*) arena_alloc(arena,sizeof(LetStatementNode));
            node->data.letStatement->varName = NULL;
            node->data.letStatement->indexExpression = NULL;
            node->data.letStatement->rightExpression = NULL;
            break;
        case NODE_IF_STATEMENT:
            node->data.ifStatement = (IfStatementNode*) arena_alloc(arena,sizeof(IfStatementNode));
            node->data.ifStatement->condition = NULL;
            node->data.ifStatement->ifBranch = NULL;
            node->data.ifStatement->elseBranch = NULL;
            break;
        case NODE_WHILE_STATEMENT:
            node->data.whileStatement = (WhileStatementNode*) arena_alloc(arena,sizeof(WhileStatementNode));
            node->data.whileStatement->condition = NULL;
            node->data.whileStatement->body = NULL;
            break;
        case NODE_DO_STATEMENT:
            node->data.doStatement = (DoStatementNode*) arena_alloc(arena,sizeof(DoStatementNode));
            node->data.doStatement->subroutineCall = NULL;
            break;
        case NODE_RETURN_STATEMENT:
            node->data.returnStatement = (ReturnStatementNode*) arena_alloc(arena,sizeof(ReturnStatementNode));
            node->data.returnStatement->expression = NULL;
            break;
        case NODE_SUBROUTINE_CALL:
            node->data.subroutineCall = (SubroutineCallNode*) arena_alloc(arena,sizeof(SubroutineCallNode));
            node->data.subroutineCall->caller = NULL;
            node->data.subroutineCall->subroutineName = NULL;
            node->data.subroutineCall->arguments = vector_create();
            node->data.subroutineCall->type = (Type*) arena_alloc(arena, sizeof (Type));
            break;
        case NODE_EXPRESSION:
            node->data.expression = (ExpressionNode*) arena_alloc(arena,sizeof(ExpressionNode));
            node->data.expression->term = NULL;
            node->data.expression->operations = vector_create();
            node->data.expression->type = (Type*) arena_alloc(arena, sizeof (Type));
            break;
        case NODE_TERM:
            node->data.term = (TermNode*) arena_alloc(arena,sizeof(TermNode));
            node->data.term->termType = TRM_NONE;
            node->data.term->type = (Type*) arena_alloc(arena, sizeof (Type));
            break;
        case NODE_OPERATION:
            node->data.operation =  (Operation*) arena_alloc(arena,sizeof(Operation));
            node->data.operation->op = 0;
            node->data.operation->term = NULL;
            break;
        case NODE_VAR_TERM:
            node->data.varTerm = (VarTerm*) arena_alloc(arena,sizeof(VarTerm));
            node->data.varTerm->className = NULL;
            node->data.varTerm->varName = NULL;
            node->data.varTerm->type = (Type*) arena_alloc(arena, sizeof (Type));
            break;
        default:
            log_error(ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__,
                      "Unknown node type: %d\n", type);
            exit(EXIT_FAILURE);
    }
    return node;
}

void execute_build_function(ASTVisitor* visitor, ASTNode* node) {
    typedef void (*BuilderFunc)(ASTVisitor*, ASTNode*);
    BuilderFunc buildFunctions[] = {
            [NODE_PROGRAM] = visitor->symbolTableBuilder->build_program_node,
            [NODE_CLASS] = visitor->symbolTableBuilder->build_class_node,
            [NODE_CLASS_VAR_DEC] = visitor->symbolTableBuilder->build_class_var_dec_node,
            [NODE_SUBROUTINE_DEC] = visitor->symbolTableBuilder->build_subroutine_dec_node,
            [NODE_PARAMETER_LIST] = visitor->symbolTableBuilder->build_parameter_list_node,
            [NODE_SUBROUTINE_BODY] = visitor->symbolTableBuilder->build_subroutine_body,
            [NODE_VAR_DEC]  = visitor->symbolTableBuilder->build_var_dec_node
    };

    if(node->nodeType < sizeof(buildFunctions)/sizeof(BuilderFunc) && buildFunctions[node->nodeType]) {
        buildFunctions[node->nodeType](visitor, node);
    } else {
        log_message(LOG_LEVEL_WARNING, ERROR_NONE,__FILE__, __LINE__, "Unsupported node type for building");
    }

}

void execute_analyze_function(ASTVisitor* visitor, ASTNode* node) {
    typedef void (*AnalyzerFunc)(ASTVisitor*, ASTNode*);
    AnalyzerFunc analyzeFunctions[] = {
         [NODE_PROGRAM] = visitor->semanticAnalyzer->analyze_program_node,
         [NODE_CLASS] = visitor->semanticAnalyzer->analyze_class_node,
         [NODE_CLASS_VAR_DEC] = visitor->semanticAnalyzer->analyze_class_var_dec_node,
         [NODE_SUBROUTINE_DEC] = visitor->semanticAnalyzer->analyze_subroutine_dec_node,
         [NODE_PARAMETER_LIST] = visitor->semanticAnalyzer->analyze_parameter_list_node,
         [NODE_SUBROUTINE_BODY] = visitor->semanticAnalyzer->analyze_subroutine_body_node,
         [NODE_STATEMENTS] = visitor->semanticAnalyzer->analyze_statements,
         [NODE_STATEMENT] = visitor->semanticAnalyzer->analyze_statement_node,
         [NODE_LET_STATEMENT] = visitor->semanticAnalyzer->analyze_let_statement_node,
         [NODE_IF_STATEMENT] = visitor->semanticAnalyzer->analyze_if_statement_node,
         [NODE_WHILE_STATEMENT] = visitor->semanticAnalyzer->analyze_while_statement_node,
         [NODE_DO_STATEMENT] = visitor->semanticAnalyzer->analyze_do_statement_node,
         [NODE_RETURN_STATEMENT] = visitor->semanticAnalyzer->analyze_return_statement_node,
         [NODE_SUBROUTINE_CALL] = visitor->semanticAnalyzer->analyze_subroutine_call_node,
         [NODE_EXPRESSION] = visitor->semanticAnalyzer->analyze_expression_node,
         [NODE_TERM] = visitor->semanticAnalyzer->analyze_term_node,
         [NODE_VAR_TERM] = visitor->semanticAnalyzer->analyze_var_term_node,
     };
     if(node->nodeType < sizeof(analyzeFunctions)/sizeof(AnalyzerFunc) && analyzeFunctions[node->nodeType]) {
        analyzeFunctions[node->nodeType](visitor, node);
    } else {
        log_error(ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__, "Unsupported node type for analyzing");
        exit(EXIT_FAILURE);
    }
}

void ast_node_accept(ASTVisitor *visitor, ASTNode *node) {
    if (!visitor || !node) {
        log_error(ERROR_NULL_POINTER, __FILE__, __LINE__, "Null visitor or node provided");
        exit(EXIT_FAILURE);
        return;
    }
    switch (visitor->phase) {
        case BUILD:
            execute_build_function(visitor, node);
            break;
        case ANALYZE:
            execute_analyze_function(visitor, node);
            break;
        default:
            log_error(ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__,
                        "Unsupported phase");
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

void build_program_node(ASTVisitor* visitor, ASTNode* node) {
    ProgramNode* programNode = node->data.program;

    for(int i = 0; i < vector_size(programNode->classes); i++) {
        ASTNode* classNode = (ASTNode*) vector_get(programNode->classes, i);
        ast_node_accept(visitor, classNode);
    }
}

void build_class_node(ASTVisitor* visitor, ASTNode* node) {
    SymbolTable* classTable = create_table(SCOPE_CLASS, visitor->currentTable, visitor->arena);
    Symbol* classSymbol = symbol_table_add(visitor->currentTable, node->data.classDec->className,
                                           node->data.classDec->className, KIND_CLASS);
    classSymbol->childTable = classTable;

    push_table(visitor, classTable);

    for (int i = 0; i < vector_size(node->data.classDec->classVarDecs); i++) {
        ASTNode* classVarDecNode =  (ASTNode*) vector_get(node->data.classDec->classVarDecs, i);
        ast_node_accept(visitor, classVarDecNode);
    }
    for (int i = 0; i < vector_size(node->data.classDec->subroutineDecs); i++) {
        ASTNode* subroutineDecNode = (ASTNode*) vector_get(node->data.classDec->subroutineDecs, i);
        ast_node_accept(visitor, subroutineDecNode);
    }
    pop_table(visitor);
}

void build_class_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.classVarDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.classVarDec->varNames, i);
        if (symbol_table_lookup(visitor->currentTable, varName, LOOKUP_LOCAL)) {
            log_error(ERROR_SEMANTIC_REDECLARED_SYMBOL, node->filename, node->line,
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
                log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line, "Invalid class var modifier");
                exit(EXIT_FAILURE);
        }
    }
}

void build_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {
    SymbolTable* subroutineTable;
    Symbol* subSymbol;

    switch(node->data.subroutineDec->subroutineType) {
        case CONSTRUCTOR:
            subroutineTable = create_table(SCOPE_CONSTRUCTOR, visitor->currentTable, visitor->arena);
            subSymbol = symbol_table_add(visitor->currentTable, node->data.subroutineDec->subroutineName,
                node->data.subroutineDec->returnType, KIND_CONSTRUCTOR);
            break;
        case METHOD:
            subroutineTable = create_table(SCOPE_METHOD, visitor->currentTable, visitor->arena);
            subSymbol =symbol_table_add(visitor->currentTable, node->data.subroutineDec->subroutineName,
                node->data.subroutineDec->returnType, KIND_METHOD);
            break;
        case FUNCTION:
            subroutineTable = create_table(SCOPE_FUNCTION, visitor->currentTable, visitor->arena);
            subSymbol = symbol_table_add(visitor->currentTable, node->data.subroutineDec->subroutineName,
                node->data.subroutineDec->returnType, KIND_FUNCTION);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_SUBROUTINE, node->filename, node->line,
                      "Invalid subroutine type");
            exit(EXIT_FAILURE);
    }

    subSymbol->childTable = subroutineTable;
    push_table(visitor, subroutineTable);
    ast_node_accept(visitor, node->data.subroutineDec->parameters);
    ast_node_accept(visitor, node->data.subroutineDec->body);
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
        ast_node_accept(visitor, varDecNode);
    }
}

void build_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.varDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.varDec->varNames, i);
        (void) symbol_table_add(visitor->currentTable, varName, node->data.varDec->varType, KIND_VAR);
    }
}

void analyze_program_node(ASTVisitor* visitor, ASTNode* node) {
    ProgramNode* programNode = node->data.program;

    for(int i = 0; i < vector_size(programNode->classes); i++) {
        ASTNode* classNode = (ASTNode*) vector_get(programNode->classes, i);
        ast_node_accept(visitor, classNode);
    }
}

void analyze_class_node(ASTVisitor* visitor, ASTNode* node) {
    visitor->currentClassName = node->data.classDec->className;

    Symbol* classSymbol = symbol_table_lookup(visitor->currentTable, node->data.classDec->className
                                                , LOOKUP_LOCAL);
    if (!classSymbol || classSymbol->kind != KIND_CLASS) {
        log_error(ERROR_SEMANTIC_INVALID_KIND, node->filename, node->line,
                  "Undefined class: %s", node->data.classDec->className);
        exit(EXIT_FAILURE);
    }

    push_table(visitor, classSymbol->childTable);

    for (int i = 0; i < vector_size(node->data.classDec->classVarDecs); i++) {
        ASTNode* classVarDecNode =  (ASTNode*) vector_get(node->data.classDec->classVarDecs, i);
        ast_node_accept(visitor, classVarDecNode);
    }

    for (int i = 0; i < vector_size(node->data.classDec->subroutineDecs); i++) {
        ASTNode* subroutineDecNode = (ASTNode*) vector_get(node->data.classDec->subroutineDecs, i);
        ast_node_accept(visitor, subroutineDecNode);
    }

    // Return to the parent scope
    pop_table(visitor);
    visitor->currentClassName = NULL;
}

void analyze_class_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.classVarDec->varNames); i++) {
        char* varName = (char*) vector_get(node->data.classVarDec->varNames, i);
        Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, varName, LOOKUP_GLOBAL);
        if(!type_is_valid(visitor, varSymbol->type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                      "Invalid type %s.", varSymbol->type->userDefinedType);
            exit(EXIT_FAILURE);
        }
    }
}

void analyze_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {
    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable,
                                            node->data.subroutineDec->subroutineName, LOOKUP_LOCAL);
    if(!subSymbol || (subSymbol->kind != KIND_METHOD && subSymbol->kind != KIND_CONSTRUCTOR
            && subSymbol->kind != KIND_FUNCTION)) {
        log_error(ERROR_SEMANTIC_INVALID_KIND, node->filename, node->line,
                  "Undefined subroutine: %s", node->data.subroutineDec->subroutineName);
        exit(EXIT_FAILURE);
    }

    push_table(visitor, subSymbol->childTable);
    ast_node_accept(visitor, node->data.subroutineDec->parameters);
    ast_node_accept(visitor, node->data.subroutineDec->body);
    pop_table(visitor);
}

void analyze_parameter_list_node(ASTVisitor* visitor, ASTNode* node) {
    for(int i = 0; i < vector_size(node->data.parameterList->parameterTypes); i++) {
        char* paramName = (char*) vector_get(node->data.parameterList->parameterNames, i);
        Symbol* paramSymbol = symbol_table_lookup(visitor->currentTable, paramName, LOOKUP_GLOBAL);
        if(!type_is_valid(visitor, paramSymbol->type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                      "Invalid type %s.", paramSymbol->type->userDefinedType);
            exit(EXIT_FAILURE);
        }
    }
}

void analyze_subroutine_body_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.subroutineBody->varDecs); i++) {
        ASTNode* varDecNode = (ASTNode*) vector_get(node->data.subroutineBody->varDecs, i);
        char* varName = (char*) vector_get(varDecNode->data.varDec->varNames, i);
        Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, varName, LOOKUP_LOCAL);
        if (!type_is_valid(visitor, varSymbol->type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                      "Invalid type %s for variable %s.", varSymbol->type->userDefinedType, varName);
            exit(EXIT_FAILURE);
        }
    }
    ast_node_accept(visitor, node->data.subroutineBody->statements);
}

// Var Dec analysis done in subroutine dec
void analyze_var_dec_node(ASTVisitor* visitor, ASTNode* node) {

}

void analyze_statements_node(ASTVisitor* visitor, ASTNode* node) {
    for(int i = 0; i < vector_size(node->data.statements->statements); i++) {
        ASTNode* stmtNode = (ASTNode*) vector_get(node->data.statements->statements, i);
        ast_node_accept(visitor, stmtNode);
    }
}

void analyze_statement_node(ASTVisitor* visitor, ASTNode* node) {

    switch (node->data.statement->statementType) {
        case LET:
            ast_node_accept(visitor, node->data.statement->data.letStatement);
            break;
        case IF:
            ast_node_accept(visitor, node->data.statement->data.ifStatement);
            break;
        case WHILE:
            ast_node_accept(visitor, node->data.statement->data.whileStatement);
            break;
        case DO:
            ast_node_accept(visitor, node->data.statement->data.doStatement);
            break;
        case RETURN:
            ast_node_accept(visitor, node->data.statement->data.returnStatement);
            break;
        default:
            log_error(ERROR_SEMANTIC_INVALID_STATEMENT, __FILE__, __LINE__,
                      "Invalid statement node at %s:%d", node->filename, node->line);
            exit(EXIT_FAILURE);
    }
}

void analyze_let_statement_node(ASTVisitor* visitor, ASTNode* node) {

    LetStatementNode* letStmtNode = node->data.letStatement;
    char* varName = letStmtNode->varName;
    Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, varName, LOOKUP_CLASS);

    if(!varSymbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, node->filename, node->line,
                  "Undeclared variable : %s", varName);
        exit(EXIT_FAILURE);
    }

    if(letStmtNode->indexExpression) {
        ast_node_accept(visitor, letStmtNode->indexExpression);
        Type* indexExprType = letStmtNode->indexExpression->data.expression->type;
        if(indexExprType->userDefinedType != TYPE_INT) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                      "Array index must be an integer.");
            exit(EXIT_FAILURE);
        }

        // May need to confirm varName is an array
    }

    ast_node_accept(visitor, letStmtNode->rightExpression);
    Type* rightExprType =  letStmtNode->rightExpression->data.expression->type;
    if(!types_are_equal(rightExprType, varSymbol->type)) {
        log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                  "Type mismatch in assignment.");
        exit(EXIT_FAILURE);
    }
}

void analyze_if_statement_node(ASTVisitor* visitor, ASTNode* node) {
    IfStatementNode* ifStmtNode = node->data.ifStatement;

    ast_node_accept(visitor, ifStmtNode->condition);
    Type* conditionType = ifStmtNode->condition->data.expression->type;
    if (conditionType->basicType != TYPE_BOOLEAN) {
        log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                  "If statement condition must evaluate to a boolean.");
        exit(EXIT_FAILURE);
    }

    ast_node_accept(visitor, ifStmtNode->ifBranch);

    if(ifStmtNode->elseBranch) {
        ast_node_accept(visitor, ifStmtNode->elseBranch);
    }
}

void analyze_while_statement_node(ASTVisitor* visitor, ASTNode* node) {
    WhileStatementNode* whileStmtNode = node->data.whileStatement;

    // Analyze the condition expression
    ast_node_accept(visitor, whileStmtNode->condition);
    Type* conditionType = whileStmtNode->condition->data.expression->type;

    // Ensure the condition evaluates to a boolean
    if (conditionType->basicType != TYPE_BOOLEAN) {
        log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                  "While statement condition must evaluate to a boolean.");
        exit(EXIT_FAILURE);
    }

    // Recursively analyze the statements inside the 'while' block
    ast_node_accept(visitor, whileStmtNode->body);
}


void analyze_do_statement_node(ASTVisitor* visitor, ASTNode* node) {
    DoStatementNode* doStmtNode = node->data.doStatement;

    // Analyze the subroutine call
    ast_node_accept(visitor, doStmtNode->subroutineCall);
}

void analyze_return_statement_node(ASTVisitor* visitor, ASTNode* node) {
    // A Return statement can only be reached within a subroutine node
    SymbolTable* parent = visitor->currentTable->parent;
    Symbol* subSymbol;
    for(int i = 0 ; i < vector_size(parent->symbols); ++i) {
        Symbol* symbol = vector_get(parent->symbols, i);
        if(symbol->childTable == visitor->currentTable) {
            subSymbol = symbol;
        }
    }

    if(!subSymbol) {
        log_error(ERROR_NULL_POINTER, node->filename, node->line, "Could not find subroutine in class");
        exit(EXIT_FAILURE);
    }

    Type* subroutineType = subSymbol->type;
    ReturnStatementNode* returnStmt = node->data.returnStatement;
    if (returnStmt->expression) {
        ast_node_accept(visitor, returnStmt->expression);
        if(!types_are_equal(subroutineType, returnStmt->expression->data.expression->type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                      "Return expression type: (%s) , mismatch with subroutine return type (%s)",
                      type_to_str(returnStmt->expression->data.expression->type), type_to_str(subroutineType));
        }
        ast_node_accept(visitor, returnStmt->expression);
    } else {
        // When return format is just 'return;', subroutine type should be void
        if (subroutineType->basicType != TYPE_VOID) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                      "Subroutine is expected to return type (%s), but no return value provided.",
                      type_to_str(subroutineType));
            exit(EXIT_FAILURE);
        }
    }
}

void analyze_term_node(ASTVisitor* visitor, ASTNode* node) {
    TermNode* termNode = node->data.term;
    char* keyword = termNode->data.keywordValue;

    switch (termNode->termType)
    {
        case INTEGER_CONSTANT:
            termNode->type->basicType = TYPE_INT;
            termNode->type->userDefinedType = NULL;
            break;
        case STRING_CONSTANT:
            termNode->type->basicType = TYPE_STRING;
            termNode->type->userDefinedType = NULL;
            break;
        case KEYWORD_CONSTANT:
            if (strcmp(keyword, "true") == 0 || strcmp(keyword, "false") == 0) {
                termNode->type->basicType = TYPE_BOOLEAN;
                termNode->type->userDefinedType = NULL;
            } else if (strcmp(keyword, "null") == 0) {
                termNode->type->basicType = TYPE_NULL;
                termNode->type->userDefinedType = NULL;
            } else if (strcmp(keyword, "this") == 0) {
                termNode->type->basicType = TYPE_USER_DEFINED;
                termNode->type->userDefinedType = visitor->currentClassName; // Assuming you have this field in ASTVisitor
            } else {
                log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line, "Invalid keyword constant");
                exit(EXIT_FAILURE);
            }
            break;
        case VAR_TERM:
            ast_node_accept(visitor, termNode->data.varTerm);
            termNode->type = termNode->data.varTerm->data.varTerm->type;
            break;
        case SUBROUTINE_CALL:
            ast_node_accept(visitor, termNode->data.subroutineCall);
            termNode->type = termNode->data.subroutineCall->data.subroutineCall->type;
            break;
        case EXPRESSION:
            ast_node_accept(visitor, termNode->data.expression);
            termNode->type = termNode->data.expression->data.expression->type;
            break;
        case UNARY_OP:
            analyze_unary_op_node(visitor,node);
            termNode->type = termNode->data.unaryOp.term->data.term->type;
            break;
        case ARRAY_ACCESS:
            analyze_array_access_node(visitor, node);
            termNode->type = termNode->data.arrayAccess.type;
        default:
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line, "Invalid term type");
            exit(EXIT_FAILURE);
    }
}

bool type_is_valid(ASTVisitor* visitor, Type* type) {
    if(type->userDefinedType) {
        if (!symbol_table_lookup(visitor->currentTable, type->userDefinedType, LOOKUP_GLOBAL)) {
            return false;
        }
    }
    return true;
}
bool type_arithmetic_compat(Type* type1, Type* type2) {
    return type1->basicType == TYPE_INT && type2->basicType == TYPE_INT;
}

bool type_comparison_compat(Type* type1, Type* type2) {
    return (type1->basicType == TYPE_INT || type1->basicType == TYPE_CHAR) &&
        (type2->basicType == TYPE_INT || type2->basicType == TYPE_CHAR);
}

bool type_is_boolean(Type* type) {
    return type->basicType == TYPE_BOOLEAN;
}

bool types_are_equal(Type* type1, Type* type2) {

    if (type1->basicType != type2->basicType) {
        return false;
    }

    if (type1->userDefinedType) {
        return strcmp(type1->userDefinedType, type2->userDefinedType) == 0;
    }

    return true;
}

void analyze_expression_node(ASTVisitor* visitor, ASTNode* node) {
    ast_node_accept(visitor, node->data.expression->term);

    Type* resultType = node->data.expression->type;
    *resultType = *(node->data.expression->term->data.term->type);

    for (int i = 0; i < vector_size(node->data.expression->operations); i++) {
        ASTNode *opNode = (ASTNode *) vector_get(node->data.expression->operations, i);

        ast_node_accept(visitor, opNode->data.operation->term);
        Type* nextType = opNode->data.operation->term->data.term->type;

        switch (opNode->data.operation->op) {
            case '+':
            case '-':
            case '*':
            case '/': // arithmetic
                if (!type_arithmetic_compat(resultType, nextType)) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                              "Invalid type for arithmetic operation");
                    exit(EXIT_FAILURE);
                }
                resultType->basicType = TYPE_INT;
                resultType->userDefinedType = NULL;
                break;
            case '>':
            case '<':
            case '=':
                if (!type_comparison_compat(resultType, nextType)) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                "Invalid type for comparison operation");
                    exit(EXIT_FAILURE);
                }
                resultType->basicType = TYPE_BOOLEAN;
                resultType->userDefinedType = NULL;
                break;
            case '&':
            case '|':
                if (!type_is_boolean(resultType) || !type_is_boolean(nextType)) {
                    log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                "Invalid type for boolean operation");
                    exit(EXIT_FAILURE);
                }
                resultType->basicType = TYPE_BOOLEAN;
                resultType->userDefinedType = NULL;
                break;
            default:
                log_error(ERROR_SEMANTIC_INVALID_OPERATION, node->filename, node->line,
                            "Invalid operation");
                break;
        }
    }

    // Assign the resultant type of the expression to the node itself
    node->data.expression->type = resultType;
}


void analyze_subroutine_call_node(ASTVisitor* visitor, ASTNode* node){
    SubroutineCallNode * subCall =  node->data.subroutineCall;

    Symbol* subSymbol = NULL;

    if (subCall->caller) {
        // First, attempt to treat the caller as a class or object name.
        Symbol* callerSymbol = symbol_table_lookup(visitor->currentTable, subCall->caller, LOOKUP_GLOBAL);

        // If it's not a global, it might be an object in the class scope.
        if (!callerSymbol) {
            log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, node->filename, node->line,
                      "Undeclared caller : %s", subCall->caller);
            exit(EXIT_FAILURE);
        }

        SymbolTable* targetTable = NULL;
        if (callerSymbol->kind == KIND_CLASS) {
            targetTable = callerSymbol->childTable;
        } else if (callerSymbol->type) {
            Symbol* typeSymbol = symbol_table_lookup(visitor->currentTable, callerSymbol->type->userDefinedType, LOOKUP_GLOBAL);
            if (typeSymbol) {
                targetTable = typeSymbol->childTable;
            }
        }

        // Look up the subroutine in the determined symbol table.
        if (targetTable) {
            subSymbol = symbol_table_lookup(targetTable, subCall->subroutineName, LOOKUP_LOCAL);
        }
    } else {
        // No caller, so proceed with the current lookup logic
        subSymbol = symbol_table_lookup(visitor->currentTable, subCall->subroutineName, LOOKUP_GLOBAL);
    }


    if (!subSymbol || !(subSymbol->kind == KIND_FUNCTION ||
        subSymbol->kind == KIND_CONSTRUCTOR || subSymbol->kind == KIND_METHOD)) {
        log_error(ERROR_SEMANTIC_INVALID_EXPRESSION, node->filename, node->line,
                  "Undefined subroutine: %s", subCall->subroutineName);
        exit(EXIT_FAILURE);
    }

    subCall->type = subSymbol->type;

    SymbolTable* subroutineTable = subSymbol->childTable;
    vector args = get_symbols_of_kind(subroutineTable, KIND_ARG);

    //Check Arguments
    for(int i = 0; i < vector_size(subCall->arguments); i++) {
        ASTNode* arg = vector_get(subCall->arguments, i);
        ast_node_accept(visitor, arg);
        Type* argType = arg->data.expression->term->data.term->type;
        Symbol* expectedArgSymbol = vector_get(args, i);


         // Special case for Memory.deAlloc
        if (strcmp(subCall->subroutineName, "deAlloc") == 0 && strcmp(subCall->caller, "Memory") == 0) {
            // Bypass type check for Memory.deAlloc
            continue;
        }


        if(!types_are_equal(expectedArgSymbol->type, argType)) {
            log_error(ERROR_SEMANTIC_INVALID_ARGUMENT, node->filename, node->line,
                          "Mismatched argument type for subroutine: %s", subCall->subroutineName);
                exit(EXIT_FAILURE);
        }
    }
}

void analyze_var_term_node(ASTVisitor* visitor, ASTNode* node) {
    VarTerm* term = node->data.varTerm;
    //!  TODO - Change from varname to something inlcuding classname as well
    Symbol* termSymbol = symbol_table_lookup(visitor->currentTable, term->varName, LOOKUP_CLASS);
    if (!termSymbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, node->filename, node->line,
                  "Undefined variable: %s", term->varName);
        exit(EXIT_FAILURE);
    }

    term->type = termSymbol->type;

    if (term->className) {
        Symbol* classSymbol = symbol_table_lookup(visitor->currentTable, term->className, LOOKUP_GLOBAL);
        Symbol* attributeOrMethod = symbol_table_lookup(classSymbol->childTable, term->varName, LOOKUP_LOCAL);
        if (!attributeOrMethod) {
            log_error(ERROR_SEMANTIC_INVALID_TERM, node->filename, node->line,
                      "Variable %s not a valid attribute or method of class %s", term->varName, term->className);
            exit(EXIT_FAILURE);
        }
    }
}

void analyze_unary_op_node(ASTVisitor* visitor, ASTNode* node) {

    ASTNode* unaryOpTerm = node->data.term->data.unaryOp.term;
    ast_node_accept(visitor, unaryOpTerm);
    char op = node->data.term->data.unaryOp.unaryOp;
    Type* type = unaryOpTerm->data.term->type;

    if (op == '~') {
        if (!type_is_boolean(type)) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                        "Invalid type for unary operation");
            exit(EXIT_FAILURE);
        }
    } else if (op == '-') {
        if (type->basicType != TYPE_INT) {
            log_error(ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                        "Invalid type for unary operation");
            exit(EXIT_FAILURE);
        }
    } else {
        log_error(ERROR_SEMANTIC_INVALID_OPERATION, node->filename, node->line,
                    "Invalid unary operation");
        exit(EXIT_FAILURE);
    }
}

void analyze_array_access_node(ASTVisitor* visitor, ASTNode* node) {

    ASTNode* indexNode = node->data.term->data.arrayAccess.index;
    ast_node_accept(visitor, indexNode);

    Symbol* arrSymbol = symbol_table_lookup(visitor->currentTable
                        ,node->data.term->data.arrayAccess.arrayName, LOOKUP_CLASS);

    if (!arrSymbol) {
        log_error(ERROR_SEMANTIC_UNDECLARED_SYMBOL, node->filename, node->line,
                  "Undefined variable: %s", node->data.term->data.arrayAccess.arrayName);
        exit(EXIT_FAILURE);
    }

    node->data.term->data.arrayAccess.type = arrSymbol->type;

    // TODO - check whether index is valid
}