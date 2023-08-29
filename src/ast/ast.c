#include "ast.h"
#include <string.h>
#include "arena.h"

SemanticAnalyzer* init_semantic_analyzer(Arena* arena) {
    SemanticAnalyzer* analyzer = arena_alloc(arena, sizeof(SemanticAnalyzer));
    if (!analyzer) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for SemanticAnalyzer", __func__);
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
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for SymbolTableBuilder", __func__);
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

CodeGenerator* init_code_gen(Arena* arena) {
    CodeGenerator* generator = arena_alloc(arena, sizeof(CodeGenerator));
    if (!generator) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for CodeGenerator", __func__);
        return NULL;
    }

    generator->generate_program_node = &generate_program_node;
    generator->generate_class_node = &generate_class_node;
    generator->generate_class_var_dec_node = &generate_class_var_dec_node;
    generator->generate_sub_dec_node = &generate_sub_dec_node;
    generator->generate_param_list_node = &generate_param_list_node;
    generator->generate_sub_body_node = &generate_sub_body_node;
    generator->generate_stmts_node = &generate_stmts_node;
    generator->generate_stmt_node = &generate_stmt_node;
    generator->generate_let_node = &generate_let_node;
    generator->generate_if_node = &generate_if_node;
    generator->generate_while_node = &generate_while_node;
    generator->generate_do_node = &generate_do_node;
    generator->generate_return_node = &generate_return_node;
    generator->generate_sub_call_node = & generate_sub_call_node;
    generator->generate_expression_node = &generate_expression_node;
    generator->generate_term_node = &generate_term_node;
    generator->generate_var_tem_node = &generate_var_tem_node;

    return generator;
}


ASTVisitor* init_ast_visitor(Arena* arena, Phase initialPhase, SymbolTable* globalTable) {
    ASTVisitor* visitor = arena_alloc(arena, sizeof(ASTVisitor));
    if (!visitor) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for ASTVisitor", __func__);
        return NULL;
    }

    visitor->semanticAnalyzer = init_semantic_analyzer(arena);
    visitor->symbolTableBuilder = init_symbol_table_builder(arena);
    visitor->generator = init_code_gen(arena);

    visitor->currentTable = globalTable;
    visitor->phase = initialPhase;
    visitor->currentClassName = NULL;
    visitor->vmFile = NULL;
    visitor->arena = arena;
    visitor->labelCounters = vector_create();

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
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__,
                            "['%s'] : Unknown node type: %d\n", __func__, type);
            return NULL;
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
         log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__,
                            "['%s'] : Unsupported node type for analyzing", __func__);
    }
}

void execute_generator_function(ASTVisitor* visitor, ASTNode* node) {
    typedef void (*GenFunc)(ASTVisitor*, ASTNode*);
    GenFunc genFunctions[] = {
         [NODE_PROGRAM] = visitor->generator->generate_program_node,
         [NODE_CLASS] = visitor->generator->generate_class_node,
         [NODE_CLASS_VAR_DEC] = visitor->generator->generate_class_var_dec_node,
         [NODE_SUBROUTINE_DEC] = visitor->generator->generate_sub_dec_node,
         [NODE_PARAMETER_LIST] = visitor->generator->generate_param_list_node,
         [NODE_SUBROUTINE_BODY] = visitor->generator->generate_sub_body_node,
         [NODE_STATEMENTS] = visitor->generator->generate_stmts_node,
         [NODE_STATEMENT] = visitor->generator->generate_stmt_node,
         [NODE_LET_STATEMENT] = visitor->generator->generate_let_node,
         [NODE_IF_STATEMENT] = visitor->generator->generate_if_node,
         [NODE_WHILE_STATEMENT] = visitor->generator->generate_while_node,
         [NODE_DO_STATEMENT] = visitor->generator->generate_do_node,
         [NODE_RETURN_STATEMENT] = visitor->generator->generate_return_node,
         [NODE_SUBROUTINE_CALL] = visitor->generator->generate_sub_call_node,
         [NODE_EXPRESSION] = visitor->generator->generate_expression_node,
         [NODE_TERM] = visitor->generator->generate_term_node,
         [NODE_VAR_TERM] = visitor->generator->generate_var_tem_node,
     };
     if(node->nodeType < sizeof(genFunctions)/sizeof(GenFunc) && genFunctions[node->nodeType]) {
        genFunctions[node->nodeType](visitor, node);
    } else {
         log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_UNKNOWN_NODE_TYPE, __FILE__, __LINE__,
                            "['%s'] : Unsupported node type for code generation", __func__);
    }
}

void ast_node_accept(ASTVisitor *visitor, ASTNode *node) {
    if (!visitor || !node) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : Null visitor or node provided", __func__);
        return;
    }
    switch (visitor->phase) {
        case BUILD:
            execute_build_function(visitor, node);
            break;
        case ANALYZE:
            execute_analyze_function(visitor, node);
            break;
        case GENERATE:
            execute_generator_function(visitor, node);
            break;
        default:
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_INVALID_INPUT, __FILE__, __LINE__,
                            "['%s'] : Unsupported phase", __func__);
    }
}

void push_table(ASTVisitor* visitor, SymbolTable* table) {
    visitor->currentTable = table;
}

void pop_table(ASTVisitor* visitor) {
    if(visitor->currentTable && visitor->currentTable->parent) {
        visitor->currentTable = visitor->currentTable->parent;
    } else {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s'] : Unexpected NULL parent table", __func__);
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_REDECLARED_SYMBOL, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Variable %s is already declared in this scope", __func__, varName);
        }
        switch (node->data.classVarDec->classVarModifier) {
            case STATIC:
                (void) symbol_table_add(visitor->currentTable, varName, node->data.classVarDec->varType, KIND_STATIC);
                break;
            case FIELD:
                (void) symbol_table_add(visitor->currentTable, varName, node->data.classVarDec->varType, KIND_FIELD);
                break;
            default:
                log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid class var modifier", __func__);
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_SUBROUTINE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid subroutine type", __func__);
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
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_KIND , node->filename, node->line,
                              node->byte_offset, "['%s'] : Undefined class >  '%s'", __func__, node->data.classDec->className );
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_TYPE , node->filename, node->line,
                              node->byte_offset, "['%s'] : Invalid type %s", __func__, varSymbol->type->userDefinedType );
        }
    }
}

void analyze_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {
    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable,
                                            node->data.subroutineDec->subroutineName, LOOKUP_LOCAL);
    if(!subSymbol || (subSymbol->kind != KIND_METHOD && subSymbol->kind != KIND_CONSTRUCTOR
            && subSymbol->kind != KIND_FUNCTION)) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_KIND , node->filename, node->line,
                              node->byte_offset, "['%s'] : Undefined subroutine > '%s'", __func__, node->data.subroutineDec->subroutineName );
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_TYPE , node->filename, node->line,
                              node->byte_offset, "['%s'] : Invalid type ['%s'] for this parameter > '%s'", __func__,
                              paramSymbol->type->userDefinedType, paramName);
        }
    }
}

void analyze_subroutine_body_node(ASTVisitor* visitor, ASTNode* node) {
    for (int i = 0; i < vector_size(node->data.subroutineBody->varDecs); i++) {
        ASTNode* varDecNode = (ASTNode*) vector_get(node->data.subroutineBody->varDecs, i);
        char* varName = (char*) vector_get(varDecNode->data.varDec->varNames, i);
        Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, varName, LOOKUP_LOCAL);
        if (!type_is_valid(visitor, varSymbol->type)) {
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_TYPE , node->filename, node->line,
                              node->byte_offset, "['%s'] : Invalid type ['%s'] for this variable > '%s'", __func__,
                              varSymbol->type->userDefinedType, varName);
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_STATEMENT , node->filename, node->line,
                              node->byte_offset, "['%s'] : Invalid statement", __func__);
    }
}

void analyze_let_statement_node(ASTVisitor* visitor, ASTNode* node) {

    LetStatementNode* letStmtNode = node->data.letStatement;
    char* varName = letStmtNode->varName;
    Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, varName, LOOKUP_CLASS);

    if(!varSymbol) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_UNDECLARED_SYMBOL , node->filename, node->line,
                              node->byte_offset, "['%s'] : This variable is undeclared > '%s'", __func__, varName);
        return;
    }

    if(letStmtNode->indexExpression) {
        ast_node_accept(visitor, letStmtNode->indexExpression);
        Type* indexExprType = letStmtNode->indexExpression->data.expression->type;
        if(indexExprType->userDefinedType != TYPE_INT) {
             log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_EXPRESSION , node->filename, node->line,
                              node->byte_offset, "['%s'] : Array index must be an integer.", __func__);
        }

        //TODO -  May need to confirm varName is an array
    }

    ast_node_accept(visitor, letStmtNode->rightExpression);
    Type* rightExprType =  letStmtNode->rightExpression->data.expression->type;
    if(!types_are_equal(rightExprType, varSymbol->type)) {
         log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE , node->filename, node->line,
                              node->byte_offset, "['%s'] : Type mismatch in assignment", __func__);
    }
}

void analyze_if_statement_node(ASTVisitor* visitor, ASTNode* node) {
    IfStatementNode* ifStmtNode = node->data.ifStatement;

    ast_node_accept(visitor, ifStmtNode->condition);
    Type* conditionType = ifStmtNode->condition->data.expression->type;
    if (conditionType->basicType != TYPE_BOOLEAN) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                              node->byte_offset, "['%s'] : Condition must evaluate to a bool type", __func__);
    }

    ast_node_accept(visitor, ifStmtNode->ifBranch);

    if(ifStmtNode->elseBranch) {
        ast_node_accept(visitor, ifStmtNode->elseBranch);
    }
}

void analyze_while_statement_node(ASTVisitor* visitor, ASTNode* node) {
    WhileStatementNode* whileStmtNode = node->data.whileStatement;

    ast_node_accept(visitor, whileStmtNode->condition);
    Type* conditionType = whileStmtNode->condition->data.expression->type;

    if (conditionType->basicType != TYPE_BOOLEAN) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                              node->byte_offset, "['%s'] : Condition must evaluate to a bool", __func__);
    }

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
        log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_NULL_POINTER, node->filename, node->line,
                              node->byte_offset, "['%s'] : Could not find subroutine symbol in parent table", __func__);
        return;
    }

    Type* subroutineType = subSymbol->type;
    ReturnStatementNode* returnStmt = node->data.returnStatement;
    if (returnStmt->expression) {
        ast_node_accept(visitor, returnStmt->expression);
        if(!types_are_equal(subroutineType, returnStmt->expression->data.expression->type)) {
            log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Return type > '%s', mismatch with subroutine return type '%s'",
                                  __func__, type_to_str(returnStmt->expression->data.expression->type), type_to_str(subroutineType));
        }
    } else {
        // When return format is just 'return;', subroutine type should be void
        if (subroutineType->basicType != TYPE_VOID) {
            log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Expected subroutine return type > '%s', but no return value provided.",  __func__, type_to_str(subroutineType));
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
                log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TERM, node->filename, node->line,
                                      node->byte_offset, "['%s'] : Invalid keyword constant.",  __func__ );
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TERM, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid term type",  __func__);
    }
}

bool type_is_valid(ASTVisitor* visitor, Type* type) {
      if(!type) {
        return false;
    }

    if(type->userDefinedType) {
        if (!symbol_table_lookup(visitor->currentTable, type->userDefinedType, LOOKUP_GLOBAL)) {
            return false;
        }
    }
    return true;
}
bool type_arithmetic_compat(Type* type1, Type* type2) {
    if(!type1 || !type2) {
        return false;
    }

    return type1->basicType == TYPE_INT && type2->basicType == TYPE_INT;
}

bool type_comparison_compat(Type* type1, Type* type2) {
    if(!type1 || !type2) {
        return false;
    }
    return (type1->basicType == TYPE_INT || type1->basicType == TYPE_CHAR) &&
        (type2->basicType == TYPE_INT || type2->basicType == TYPE_CHAR);
}

bool type_is_boolean(Type* type) {
      if(!type) {
        return false;
    }
    return type->basicType == TYPE_BOOLEAN;
}

bool types_are_equal(Type* type1, Type* type2) {

    if(!type1 || !type2) {
        return false;
    }

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

                    log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid types for arithmetic operations > ['%s', '%s']",
                                  __func__, type_to_str(resultType), type_to_str(nextType));
                }
                resultType->basicType = TYPE_INT;
                resultType->userDefinedType = NULL;
                break;
            case '>':
            case '<':
            case '=':
                if (!type_comparison_compat(resultType, nextType)) {
                      log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid types for comparison operations > ['%s', '%s']",
                                  __func__, type_to_str(resultType), type_to_str(nextType));
                }
                resultType->basicType = TYPE_BOOLEAN;
                resultType->userDefinedType = NULL;
                break;
            case '&':
            case '|':
                if (!type_is_boolean(resultType) || !type_is_boolean(nextType)) {
                      log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid types for boolean operations > ['%s', '%s']",
                                  __func__, type_to_str(resultType), type_to_str(nextType));
                }
                resultType->basicType = TYPE_BOOLEAN;
                resultType->userDefinedType = NULL;
                break;
            default:
                  log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_OPERATION, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Invalid operation",
                                  __func__);
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
        Symbol* callerSymbol = symbol_table_lookup(visitor->currentTable, subCall->caller, LOOKUP_GLOBAL);

        // If it's not a global, it might be an object in the class scope.
        if (!callerSymbol) {
              log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_UNDECLARED_SYMBOL, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Caller class is undeclared > '%s'",
                                  __func__, subCall->caller);
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
        log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_EXPRESSION, node->filename, node->line,
                              node->byte_offset, "['%s'] : Subroutine > '%s', has not been declared yet ",
                              __func__, subCall->subroutineName);
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
             log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_TYPE, node->filename, node->line,
                                  node->byte_offset, "['%s'] : Argument type > '%s', mismatch with subroutine argument type '%s'",
                                  __func__, type_to_str(argType), type_to_str(expectedArgSymbol->type));
        }
    }
}

void analyze_var_term_node(ASTVisitor* visitor, ASTNode* node) {
    VarTerm* term = node->data.varTerm;
    //!  TODO - Change from varname to something inlcuding classname as well
    Symbol* termSymbol = symbol_table_lookup(visitor->currentTable, term->varName, LOOKUP_CLASS);
    if (!termSymbol) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_UNDECLARED_SYMBOL , node->filename, node->line,
                              node->byte_offset, "['%s'] : Undefined variable >  '%s'", __func__, term->varName);
        exit(EXIT_FAILURE);
    }

    term->type = termSymbol->type;

    if (term->className) {
        Symbol* classSymbol = symbol_table_lookup(visitor->currentTable, term->className, LOOKUP_GLOBAL);
        Symbol* attributeOrMethod = symbol_table_lookup(classSymbol->childTable, term->varName, LOOKUP_LOCAL);
        if (!attributeOrMethod) {
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_TERM , node->filename, node->line,
                              node->byte_offset, "['%s'] : Variable > '%s', is not a valid attribute or method of class > '%s'"
                              , __func__, term->varName, term->className);
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_TYPE , node->filename, node->line,
                              node->byte_offset, "['%s'] : Expected boolean got > '%s' instead.", __func__, type_to_str(type));
        }
    } else if (op == '-') {
        if (type->basicType != TYPE_INT) {
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_TYPE , node->filename, node->line,
                              node->byte_offset, "['%s'] : Expected boolean got > '%s' instead.", __func__, type_to_str(type));
        }
    } else {
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_OPERATION , node->filename, node->line,
                              node->byte_offset, "['%s'] : Invalid unary operation", __func__);
        exit(EXIT_FAILURE);
    }
}

void analyze_array_access_node(ASTVisitor* visitor, ASTNode* node) {

    ASTNode* indexNode = node->data.term->data.arrayAccess.index;
    ast_node_accept(visitor, indexNode);

    Symbol* arrSymbol = symbol_table_lookup(visitor->currentTable
                        ,node->data.term->data.arrayAccess.arrayName, LOOKUP_CLASS);

    if (!arrSymbol) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_UNDECLARED_SYMBOL , node->filename, node->line,
                              node->byte_offset, "['%s'] : Array > '%s' is undeclared", __func__,
                              node->data.term->data.arrayAccess.arrayName);
    }

    node->data.term->data.arrayAccess.type = arrSymbol->type;

    // TODO - check whether index is valid
}

void generate_program_node(ASTVisitor* visitor, ASTNode* node) {

}
void generate_class_node(ASTVisitor* visitor, ASTNode* node) {
    visitor->currentClassName = node->data.classDec->className;

    Symbol* classSymbol = symbol_table_lookup(visitor->currentTable, node->data.classDec->className
                                                , LOOKUP_LOCAL);
    if (!classSymbol || classSymbol->kind != KIND_CLASS) {
        log_error_with_offset(ERROR_PHASE_CODEGEN,ERROR_SEMANTIC_INVALID_KIND , node->filename, node->line,
                              node->byte_offset, "['%s'] : Undefined class >  '%s'", __func__, node->data.classDec->className );
    }

    push_table(visitor, classSymbol->childTable);

    for (int i = 0; i < vector_size(node->data.classDec->classVarDecs); i++) {
        ASTNode* classVarDecNode = (ASTNode*) vector_get(node->data.classDec->classVarDecs, i);
        ast_node_accept(visitor, classVarDecNode);
    }

    for (int i = 0; i < vector_size(node->data.classDec->subroutineDecs); i++) {
        ASTNode* subroutineDecNode = (ASTNode*) vector_get(node->data.classDec->subroutineDecs, i);
        ast_node_accept(visitor, subroutineDecNode);
    }

    pop_table(visitor);
    visitor->currentClassName = NULL;
}

void generate_class_var_dec_node(ASTVisitor* visitor, ASTNode* node) {
    (void) visitor;
    (void) node;
}


void generate_sub_dec_node(ASTVisitor* visitor, ASTNode* node) {

    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable, node->data.subroutineDec->subroutineName, LOOKUP_LOCAL);
    if (!subSymbol || (subSymbol->kind != KIND_METHOD && subSymbol->kind != KIND_CONSTRUCTOR && subSymbol->kind != KIND_FUNCTION)) {
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_KIND , node->filename, node->line,
                              node->byte_offset, "['%s'] : Undefined subroutine > '%s'", __func__, node->data.subroutineDec->subroutineName );
        return;
    }

    char* functionLabel = arena_sprintf(visitor->arena, "%s.%s", visitor->currentClassName, node->data.subroutineDec->subroutineName);
    vector localSymbols = get_symbols_of_kind(subSymbol->childTable, KIND_VAR);
    int numLocals = vector_size(localSymbols);
    write_function(visitor->vmFile, functionLabel, numLocals);

    if (node->data.subroutineDec->subroutineType == CONSTRUCTOR) {
        vector fieldSymbols = get_symbols_of_kind(subSymbol->table, KIND_FIELD);
        int numFields = vector_size(fieldSymbols);
        write_push(visitor->vmFile, SEG_CONST, numFields);
        write_call(visitor->vmFile, "Memory.alloc", 1);
        write_pop(visitor->vmFile, SEG_POINTER, 0);  // set the `this` pointer
    }

    if (node->data.subroutineDec->subroutineType == METHOD) {
        write_push(visitor->vmFile, SEG_ARG, 0);
        write_pop(visitor->vmFile, SEG_POINTER, 0);  // set the `this` pointer
    }

    push_table(visitor, subSymbol->childTable);
    ast_node_accept(visitor, node->data.subroutineDec->body);
    pop_table(visitor);
}

void generate_param_list_node(ASTVisitor* visitor, ASTNode* node) {

    (void) visitor;
    (void) node;
}

void generate_sub_body_node(ASTVisitor* visitor, ASTNode* node) {
    ast_node_accept(visitor, node->data.subroutineBody->statements);
}
void generate_stmts_node(ASTVisitor* visitor, ASTNode* node) {
    for(int i = 0; i < vector_size(node->data.statements->statements); i++) {
        ASTNode* stmtNode = (ASTNode*) vector_get(node->data.statements->statements, i);
        ast_node_accept(visitor, stmtNode);
    }
}
void generate_stmt_node(ASTVisitor* visitor, ASTNode* node) {
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
            log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_STATEMENT , node->filename, node->line,
                              node->byte_offset, "['%s'] : Invalid statement", __func__);
    }
}

void generate_let_node(ASTVisitor* visitor, ASTNode* node) {
    LetStatementNode* letStmtNode = node->data.letStatement;
    Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, letStmtNode->varName, LOOKUP_CLASS);

    ast_node_accept(visitor, letStmtNode->rightExpression);

    if(letStmtNode->indexExpression) {
        ast_node_accept(visitor, letStmtNode->indexExpression);

        write_push(visitor->vmFile, kind_to_segment(varSymbol->kind), varSymbol->index);
        write_arithmetic(visitor->vmFile, COM_ADD);

        write_pop(visitor->vmFile, SEG_TEMP, 0);
        write_pop(visitor->vmFile, SEG_POINTER, 1);
        write_push(visitor->vmFile, SEG_TEMP, 0);
        write_pop(visitor->vmFile, SEG_THAT, 0);
    } else {
        write_pop(visitor->vmFile, kind_to_segment(varSymbol->kind), varSymbol->index);
    }
}

void generate_if_node(ASTVisitor* visitor, ASTNode* node) {
    char* trueLabel = generate_unique_label(visitor, "IF_TRUE");
    char* falseLabel = generate_unique_label(visitor, "IF_FALSE");
    char* endLabel = generate_unique_label(visitor, "IF_END");

    // Generate code for the condition expression
    ast_node_accept(visitor, node->data.ifStatement->condition);

    write_arithmetic(visitor->vmFile, COM_NOT);
    write_if(visitor->vmFile, falseLabel);

    // IF true part
    write_label(visitor->vmFile, trueLabel);
    ast_node_accept(visitor, node->data.ifStatement->ifBranch);
    write_goto(visitor->vmFile, endLabel);

    // IF false part (if exists)
    write_label(visitor->vmFile, falseLabel);
    if (node->data.ifStatement->elseBranch) {
        ast_node_accept(visitor, node->data.ifStatement->elseBranch);
    }

    write_label(visitor->vmFile, endLabel);
}

void generate_while_node(ASTVisitor* visitor, ASTNode* node) {
    char* loopStartLabel = generate_unique_label(visitor, "WHILE_START");
    char* loopEndLabel = generate_unique_label(visitor, "WHILE_END");

    write_label(visitor->vmFile, loopStartLabel);
    ast_node_accept(visitor, node->data.whileStatement->condition);
    write_arithmetic(visitor->vmFile, COM_NOT);
    write_if(visitor->vmFile, loopEndLabel);

    ast_node_accept(visitor, node->data.whileStatement->body);
    write_goto(visitor->vmFile, loopStartLabel);
    write_label(visitor->vmFile, loopEndLabel);
}

void generate_do_node(ASTVisitor* visitor, ASTNode* node) {

    ast_node_accept(visitor, node->data.doStatement->subroutineCall);

    write_pop(visitor->vmFile, SEG_TEMP, 0);

}
void generate_return_node(ASTVisitor* visitor, ASTNode* node) {

    if (node->data.returnStatement->expression) {
        ast_node_accept(visitor, node->data.returnStatement->expression);
    } else {
        write_push(visitor->vmFile, SEG_CONST, 0);
    }

    write_return(visitor->vmFile);
}

void generate_sub_call_node(ASTVisitor* visitor, ASTNode* node) {
    SubroutineCallNode* subCall = node->data.subroutineCall;
    int nArgs = vector_size(subCall->arguments);

    if (subCall->caller) {
         Symbol* callerSymbol = symbol_table_lookup(visitor->currentTable, subCall->caller, LOOKUP_GLOBAL);
         if(!callerSymbol) {
             //Error handled during analysis phase
             return;
         }
         if (callerSymbol->kind == KIND_VAR) {
             write_push(visitor->vmFile, kind_to_segment(callerSymbol->kind), callerSymbol->index);
             nArgs++;
         }
    }

    for (int i = 0; i < vector_size(subCall->arguments); i++) {
        ASTNode* arg = vector_get(subCall->arguments, i);
        ast_node_accept(visitor, arg);
    }

    char* callName;
    if (subCall->caller) {
        callName = arena_sprintf(visitor->arena, "%s.%s", subCall->caller, subCall->subroutineName);
    } else {
        callName = arena_sprintf(visitor->arena, "%s.%s", visitor->currentClassName, subCall->subroutineName);
    }

    write_call(visitor->vmFile, callName, nArgs);

}
void generate_expression_node(ASTVisitor* visitor, ASTNode* node) {

    ast_node_accept(visitor, node->data.expression->term);

    for (int i = 0; i < vector_size(node->data.expression->operations); ++i) {

        ASTNode* opNode = (ASTNode*) vector_get((node->data.expression->operations), i);
        ast_node_accept(visitor, opNode->data.operation->term);

        char op = opNode->data.operation->op;
        switch (op) {
            case '+': write_arithmetic(visitor->vmFile, COM_ADD); break;
            case '-': write_arithmetic(visitor->vmFile, COM_SUB); break;
            case '*': write_call(visitor->vmFile, "Math.multiply", 2); break;
            case '/': write_call(visitor->vmFile, "Math.divide", 2); break;
            case '&': write_arithmetic(visitor->vmFile, COM_AND); break;
            case '|': write_arithmetic(visitor->vmFile, COM_OR); break;
            case '<': write_arithmetic(visitor->vmFile, COM_LT); break;
            case '>': write_arithmetic(visitor->vmFile, COM_GT); break;
            case '=': write_arithmetic(visitor->vmFile, COM_EQ); break;
            default: break;
        }
    }

}
void generate_term_node(ASTVisitor* visitor, ASTNode* node) {
    TermNode* termNode = node->data.term;
    switch (termNode->termType) {
        case INTEGER_CONSTANT:
            write_push(visitor->vmFile, SEG_CONST, termNode->data.intValue);
            break;
        case STRING_CONSTANT:
            {
                char* str = termNode->data.stringValue;
                int len = strlen(str);
                write_push(visitor->vmFile, SEG_CONST, len);
                write_call(visitor->vmFile, "String.new", 1);
                for(int i = 0; i < len; i++) {
                    write_push(visitor->vmFile, SEG_CONST, str[i]);
                    write_call(visitor->vmFile, "String.appendChar", 2);
                }
            }
            break;
        case KEYWORD_CONSTANT:
            // True -> -1, else 0
            write_push(visitor->vmFile, SEG_CONST, (strcmp(termNode->data.keywordValue, "true") == 0) ? -1 : 0);
            if (strcmp(termNode->data.keywordValue, "this") == 0) {
                write_pop(visitor->vmFile, SEG_POINTER, 0);
            }
            break;
        case VAR_TERM:
            {
                Symbol* varSymbol = symbol_table_lookup(visitor->currentTable, termNode->data.varTerm->data.varTerm->varName, LOOKUP_CLASS);
                write_push(visitor->vmFile, kind_to_segment(varSymbol->kind), varSymbol->index);
            }
            break;
        case SUBROUTINE_CALL:
            ast_node_accept(visitor, termNode->data.subroutineCall);
            break;
        case EXPRESSION:
            ast_node_accept(visitor, termNode->data.expression);
            break;
        case UNARY_OP:
            ast_node_accept(visitor, termNode->data.unaryOp.term);
            char op = termNode->data.unaryOp.unaryOp;
            if (op == '-') {
                write_arithmetic(visitor->vmFile, COM_NEG);
            } else if (op == '~') {
                write_arithmetic(visitor->vmFile, COM_NOT);
            }
            break;
        case ARRAY_ACCESS:
            {
                Symbol* arrSymbol = symbol_table_lookup(visitor->currentTable, termNode->data.arrayAccess.arrayName, LOOKUP_CLASS);
                ast_node_accept(visitor, termNode->data.arrayAccess.index);
                write_push(visitor->vmFile, kind_to_segment(arrSymbol->kind), arrSymbol->index);
                write_arithmetic(visitor->vmFile, COM_ADD);
                write_pop(visitor->vmFile, SEG_POINTER, 1);
                write_push(visitor->vmFile, SEG_THAT, 0);
            }
            break;
        default:
            break; // Errors will be deteced during analysis
    }
}

void generate_var_tem_node(ASTVisitor* visitor, ASTNode* node) {

    (void) visitor;
    (void) node;
}