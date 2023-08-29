#ifndef AST_H
#define AST_H

#include "symbol.h"


#define PATH_TO_WR_DEF_FILE TOSTRING(DEF_FILES_DIR/writer.def)

#define SEGMENT(seg, string) seg,
#define SEGMENT_ENUM
typedef enum {
#include PATH_TO_WR_DEF_FILE
}Segment;
#undef SEGMENT_ENUM

#define COMMAND(com, string, char_repr) com,
#define COMMAND_ENUM
typedef enum {
#include PATH_TO_WR_DEF_FILE
} Command;
#undef COMMAND_ENUM


typedef struct ASTNode ASTNode;

/*
    The ProgramNode is the root node of the AST.
    It contains a vector of ClassNodes.
*/
typedef struct ProgramNode ProgramNode;

/**
*   @brief The ClassNode represents a class declaration.
*   It contains a vector of ClassVarDecNodes and a vector of SubroutineDecNodes.
*/
typedef struct ClassNode ClassNode;
/**
 * @brief The ClassVarDecNode represents a class variable declaration.
 * It contains a vector of variable names, the variable type, and the class variable modifier.
 */

typedef struct ClassVarDecNode ClassVarDecNode;
/**
 * @brief The SubroutineDecNode represents a subroutine declaration.
 * It contains a list of parameters, the subroutine body, and the subroutine type.
 */

typedef struct SubroutineDecNode SubroutineDecNode;
/**
 * @brief The ParameterListNode represents a list of parameters.
 * It contains a vector of parameter types and a vector of parameter names.
 * 
 */
typedef struct ParameterListNode ParameterListNode;
/**
 * @brief The SubroutineBodyNode represents the body of a subroutine.
 *  It contains a vector of variable declarations and a list of statements.
 * 
 */
typedef struct SubroutineBodyNode SubroutineBodyNode;
/**
 * @brief The VarDecNode represents a variable declaration.
 * It contains a vector of variable names and the variable type.
 */
typedef struct VarDecNode VarDecNode;
/**
 * @brief The StatementsNode represents a list of statements.
 * It contains a vector of StatementNodes.
 */
typedef struct StatementsNode StatementsNode;
/**
 * @brief The StatementNode represents a statement.
 * It contains a union of the different types of statements.
 */
typedef struct StatementNode StatementNode;
/**
 * @brief The LetStatementNode represents a let statement.
 * It contains the variable name, the index expression (if present), and the right expression.
 */
typedef struct LetStatementNode LetStatementNode;
/**
 * @brief The IfStatementNode represents an if statement.
 * It contains the condition expression, the if branch, and the else branch (if present).
 */
typedef struct IfStatementNode IfStatementNode;
/**
 * @brief The WhileStatementNode represents a while statement.
 * It contains the condition expression and the body.
 */
typedef struct WhileStatementNode WhileStatementNode;
/**
 * @brief The DoStatementNode represents a do statement.
 * It contains the subroutine call.
 */
typedef struct DoStatementNode DoStatementNode;
/**
 * @brief 
 * It contains the return expression (if present).
 */
typedef struct ReturnStatementNode ReturnStatementNode;
/**
 * @brief The ExpressionNode represents an expression.
 * It contains the first term and a vector of operations.
 */
typedef struct ExpressionNode ExpressionNode;

/**
 * @brief The TermNode represents a term.
 * It contains a union of the different types of terms.
 */
typedef struct TermNode TermNode;
/**
 * @brief The Operation represents an operation in an expression.
 * It contains the operator character and the term.
 */
typedef struct Operation Operation;
/**
 * @brief The VarTerm represents a term that is a variable.
 * structure that can handle both varName and className.varName cases
 */
typedef struct VarTerm VarTerm;
/**
 * @brief The SubroutineCallNode represents a subroutine call.
 * It contains the caller (if present), the subroutine name, and the list of expressions.
 */
typedef struct SubroutineCallNode SubroutineCallNode;

typedef struct SemanticAnalyzer SemanticAnalyzer;
typedef struct SymbolTableBuilder SymbolTableBuilder;
typedef struct CodeGenerator CodeGenerator;
typedef struct LabelCounter LabelCounter;


typedef enum {
    NODE_PROGRAM,
    NODE_CLASS,
    NODE_CLASS_VAR_DEC,
    NODE_SUBROUTINE_DEC,
    NODE_PARAMETER_LIST,
    NODE_SUBROUTINE_BODY,
    NODE_VAR_DEC,
    NODE_STATEMENTS,
    NODE_STATEMENT,
    NODE_LET_STATEMENT,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_DO_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_SUBROUTINE_CALL,
    NODE_EXPRESSION,
    NODE_TERM,
    NODE_OPERATION,
    NODE_VAR_TERM
} ASTNodeType;

struct ASTNode {
    ASTNodeType nodeType;
    union {
        ProgramNode* program;
        ClassNode* classDec;
        ClassVarDecNode* classVarDec;
        SubroutineDecNode* subroutineDec;
        ParameterListNode* parameterList;
        SubroutineBodyNode* subroutineBody;
        VarDecNode* varDec;
        StatementsNode* statements;
        StatementNode* statement;
        LetStatementNode* letStatement;
        IfStatementNode* ifStatement;
        WhileStatementNode* whileStatement;
        DoStatementNode* doStatement;
        ReturnStatementNode* returnStatement;
        SubroutineCallNode* subroutineCall;
        ExpressionNode* expression;
        TermNode* term;
        Operation* operation;
        VarTerm* varTerm;
    } data;
    const char* filename;
    int line;
    size_t byte_offset;
};

typedef enum {
    BUILD,
    ANALYZE,
    GENERATE,
    LINK,
    RUN,
} Phase;

typedef struct {
    SemanticAnalyzer* semanticAnalyzer;
    SymbolTableBuilder* symbolTableBuilder;
    CodeGenerator* generator;
    SymbolTable* currentTable;
    FILE* vmFile;
    Phase phase;
    char* currentClassName;
    vector labelCounters;
    Arena* arena;
} ASTVisitor;

struct SymbolTableBuilder {
    void (*build_program_node)(ASTVisitor*, ASTNode*);
    void (*build_class_node)(ASTVisitor*, ASTNode*);
    void (*build_class_var_dec_node)(ASTVisitor*, ASTNode*);
    void (*build_subroutine_dec_node)(ASTVisitor*, ASTNode*);
    void (*build_parameter_list_node)(ASTVisitor*, ASTNode*);
    void (*build_var_dec_node)(ASTVisitor*, ASTNode*);
    void (*build_subroutine_body)(ASTVisitor*, ASTNode*);
};

struct SemanticAnalyzer {
    void (*analyze_program_node)(ASTVisitor*, ASTNode*);
    void (*analyze_class_node)(ASTVisitor*, ASTNode*);
    void (*analyze_class_var_dec_node)(ASTVisitor*, ASTNode*);
    void (*analyze_subroutine_dec_node)(ASTVisitor*, ASTNode*);
    void (*analyze_parameter_list_node)(ASTVisitor*, ASTNode*);
    void (*analyze_subroutine_body_node)(ASTVisitor*, ASTNode*);
    void (*analyze_statements)(ASTVisitor*, ASTNode*);
    void (*analyze_statement_node)(ASTVisitor*, ASTNode*);
    void (*analyze_let_statement_node)(ASTVisitor*, ASTNode*);
    void (*analyze_if_statement_node)(ASTVisitor*, ASTNode*);
    void (*analyze_while_statement_node)(ASTVisitor*, ASTNode*);
    void (*analyze_do_statement_node)(ASTVisitor*, ASTNode*);
    void (*analyze_return_statement_node)(ASTVisitor*, ASTNode*);
    void (*analyze_subroutine_call_node)(ASTVisitor*, ASTNode*);
    void (*analyze_expression_node)(ASTVisitor*, ASTNode*);
    void (*analyze_term_node)(ASTVisitor*, ASTNode*);
    void (*analyze_operation_node)(ASTVisitor*, ASTNode*);
    void (*analyze_var_term_node)(ASTVisitor*, ASTNode*);
};

struct CodeGenerator {
    void(*generate_program_node)(ASTVisitor*, ASTNode*);
    void(*generate_class_node)(ASTVisitor*, ASTNode*);
    void(*generate_class_var_dec_node)(ASTVisitor*, ASTNode*);
    void(*generate_sub_dec_node)(ASTVisitor*, ASTNode*);
    void(*generate_param_list_node)(ASTVisitor*, ASTNode*);
    void(*generate_sub_body_node)(ASTVisitor*, ASTNode*);
    void(*generate_stmts_node)(ASTVisitor*, ASTNode*);
    void(*generate_stmt_node)(ASTVisitor*, ASTNode*);
    void(*generate_let_node)(ASTVisitor*, ASTNode*);
    void(*generate_if_node)(ASTVisitor*, ASTNode*);
    void(*generate_while_node)(ASTVisitor*, ASTNode*);
    void(*generate_do_node)(ASTVisitor*, ASTNode*);
    void(*generate_return_node)(ASTVisitor*, ASTNode*);
    void(*generate_sub_call_node)(ASTVisitor*, ASTNode*);
    void(*generate_expression_node)(ASTVisitor*, ASTNode*);
    void(*generate_term_node)(ASTVisitor*, ASTNode*);
    void(*generate_var_tem_node)(ASTVisitor*, ASTNode*);
};

struct ProgramNode
{
    vector classes;
};

struct ClassNode
{
    char *className;
    vector classVarDecs; // vector of ClassVarDecNode
    vector subroutineDecs; // vector of SubroutineDecNode
};

struct ClassVarDecNode
{
    enum
    {
        CVAR_NONE,
        STATIC,
        FIELD
    } classVarModifier;
    char *varType;
    vector varNames; // vector of char*
};
struct SubroutineDecNode
{
    enum
    {
        SUB_NONE,
        CONSTRUCTOR,
        FUNCTION,
        METHOD
    } subroutineType;
    char *returnType;
    char *subroutineName;
    ASTNode* parameters;
    ASTNode* body;
};
struct ParameterListNode
{
    vector parameterTypes; // vector of char*
    vector parameterNames; // vector of char*
};

struct SubroutineBodyNode
{
    vector varDecs; // vector of VarDecNode
    ASTNode* statements;
};

struct VarDecNode
{
    char *varType;
    vector varNames; // vector of char*
};
struct StatementsNode
{
    vector statements; //vector of StatementNode
};

struct StatementNode
{
    enum
    {
        STMT_NONE,
        LET,
        IF,
        WHILE,
        DO,
        RETURN
    } statementType;
    union
    {
        ASTNode* letStatement;
        ASTNode* ifStatement;
        ASTNode* whileStatement;
        ASTNode* doStatement;
        ASTNode* returnStatement;
    } data;
};

struct LetStatementNode
{
    char *varName;
    ASTNode *indexExpression; // NULL if not present
    ASTNode *rightExpression;
};

struct IfStatementNode
{
    ASTNode* condition;
    ASTNode* ifBranch;
    ASTNode* elseBranch; // NULL if not present
};

struct WhileStatementNode
{
    ASTNode* condition;
    ASTNode* body;
};
struct DoStatementNode
{
    ASTNode* subroutineCall;
};

struct ReturnStatementNode
{
    ASTNode* expression; // NULL if not present
};
struct SubroutineCallNode
{
    char *caller; // This could be a varName or className. NULL if not present.
    char *subroutineName;
    vector arguments; // vector of ExpressionNode pointers - args
    Type* type;
};
struct ExpressionNode
{
    ASTNode* term;
    vector operations; // vector of Operation pointers - ops
    Type* type;
};
struct Operation
{
    char op; // Operator character
    ASTNode* term;
};

struct VarTerm
{
    char *className; // NULL if not present
    char *varName;
    Type* type;
};
struct TermNode
{
    enum
    {
        TRM_NONE,
        INTEGER_CONSTANT,
        STRING_CONSTANT,
        KEYWORD_CONSTANT,
        VAR_TERM,
        ARRAY_ACCESS,
        SUBROUTINE_CALL,
        EXPRESSION,
        UNARY_OP
    } termType;
    union
    {
        int intValue;
        char *stringValue;
        char *keywordValue;
        ASTNode* varTerm;
        struct
        {
            char *arrayName;
            ASTNode* index;
            Type* type;
        } arrayAccess;
        ASTNode* subroutineCall;
        ASTNode* expression;
        struct
        {
            char unaryOp;
            ASTNode* term;
        } unaryOp;
    } data;
    Type* type;
};

ASTNode* init_ast_node(ASTNodeType type, Arena* arena);
ASTVisitor* init_ast_visitor(Arena* arena, Phase initialPhase, SymbolTable* globalTable);
void ast_node_accept(ASTVisitor *visitor, ASTNode *node);

void execute_build_function(ASTVisitor* visitor, ASTNode* node);
void execute_analyze_function(ASTVisitor* visitor, ASTNode* node);
void execute_generator_function(ASTVisitor* visitor, ASTNode* node);

void push_table(ASTVisitor* visitor, SymbolTable* table);
void pop_table(ASTVisitor* visitor);

void build_program_node(ASTVisitor* visitor, ASTNode* node);
void build_class_node(ASTVisitor* visitor, ASTNode* node);
void build_class_var_dec_node(ASTVisitor* visitor, ASTNode* node);
void build_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node);
void build_parameter_list_node(ASTVisitor* visitor, ASTNode* node);
void build_subroutine_body_node(ASTVisitor* visitor, ASTNode* node);
void build_var_dec_node(ASTVisitor* visitor, ASTNode* node);


bool type_arithmetic_compat(Type* type1, Type* type2);
bool type_comparison_compat(Type* type1, Type* type2);
bool type_is_boolean(Type* type);
bool type_is_valid(ASTVisitor* visitor, Type* type);
bool types_are_equal(Type* type1, Type* type2);


void analyze_program_node(ASTVisitor* visitor, ASTNode* node);
void analyze_class_node(ASTVisitor* visitor, ASTNode* node);
void analyze_class_var_dec_node(ASTVisitor* visitor, ASTNode* node);
void analyze_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node);
void analyze_parameter_list_node(ASTVisitor* visitor, ASTNode* node);
void analyze_subroutine_body_node(ASTVisitor* visitor, ASTNode* node);
void analyze_var_dec_node(ASTVisitor* visitor, ASTNode* node);
void analyze_statements_node(ASTVisitor* visitor, ASTNode* node);
void analyze_statement_node(ASTVisitor* visitor, ASTNode* node);
void analyze_let_statement_node(ASTVisitor* visitor, ASTNode* node) ;
void analyze_if_statement_node(ASTVisitor* visitor, ASTNode* node);
void analyze_while_statement_node(ASTVisitor* visitor, ASTNode* node);
void analyze_do_statement_node(ASTVisitor* visitor, ASTNode* node);
void analyze_return_statement_node(ASTVisitor* visitor, ASTNode* node);
void analyze_term_node(ASTVisitor* visitor, ASTNode* node);
void analyze_expression_node(ASTVisitor* visitor, ASTNode* node);
void analyze_subroutine_call_node(ASTVisitor* visitor, ASTNode* node);
void analyze_var_term_node(ASTVisitor* visitor, ASTNode* node);
void analyze_unary_op_node(ASTVisitor* visitor, ASTNode* node);
void analyze_array_access_node(ASTVisitor* visitor, ASTNode* node);

void generate_program_node(ASTVisitor* visitor, ASTNode* node);
void generate_class_node(ASTVisitor* visitor, ASTNode* node);
void generate_class_var_dec_node(ASTVisitor* visitor, ASTNode* node);
void generate_sub_dec_node(ASTVisitor* visitor, ASTNode* node);
void generate_param_list_node(ASTVisitor* visitor, ASTNode* node);
void generate_sub_body_node(ASTVisitor* visitor, ASTNode* node);
void generate_stmts_node(ASTVisitor* visitor, ASTNode* node);
void generate_stmt_node(ASTVisitor* visitor, ASTNode* node);
void generate_let_node(ASTVisitor* visitor, ASTNode* node);
void generate_if_node(ASTVisitor* visitor, ASTNode* node);
void generate_while_node(ASTVisitor* visitor, ASTNode* node);
void generate_do_node(ASTVisitor* visitor, ASTNode* node);
void generate_return_node(ASTVisitor* visitor, ASTNode* node);
void generate_sub_call_node(ASTVisitor* visitor, ASTNode* node);
void generate_expression_node(ASTVisitor* visitor, ASTNode* node);
void generate_term_node(ASTVisitor* visitor, ASTNode* node);
void generate_var_tem_node(ASTVisitor* visitor, ASTNode* node);

Command symbol_to_command(char symbol);
const char* command_to_string(Command command);
const char* segment_to_string(Segment segment);
Segment kind_to_segment(Kind kind);
void write_push(FILE* fptr, Segment segment, int index);
void write_pop(FILE* fptr, Segment segment, int index);
void write_arithmetic(FILE* fptr, Command command);
void write_label(FILE* fptr, char* label);
void write_goto(FILE* fptr, char* label);
void write_if(FILE* fptr, char* label);
void write_call(FILE* fptr, char* name, int n_args);
void write_function(FILE* fptr, char* name, int n_locals);
void write_return(FILE* fptr);
char* generate_unique_label(ASTVisitor* visitor, const char* labelPrefix);
#endif //AST_H