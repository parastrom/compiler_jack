#ifndef AST_H
#define AST_H

#include "symbol.h"

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

typedef struct {
    void(*visit_ast_node)(ASTNode*);
    SymbolTable* currentTable;
} ASTVisitor;


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
};
struct ExpressionNode
{
    ASTNode* term;
    vector operations; // vector of Operation pointers - ops
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
        VarTerm varTerm;
        struct
        {
            char *arrayName;
            ASTNode* index;
        } arrayAccess;
        ASTNode* subroutineCall;
        ASTNode* expression;
        struct
        {
            char unaryOp;
            ASTNode* term;
        } unaryOp;
    } data;
};

void visit_ast_node(ASTVisitor* visitor, ASTNode* node);
void ast_node_accept(ASTNode* node, ASTVisitor* visitor)
void visit_class_node(ASTVisitor* visitor, ASTNode* node);
void visit_class_var_dec_node(ASTVisitor* visitor, ASTNode* node);
void visit_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node);
void visit_parameter_list_node(ASTVisitor* visitor, ASTNode* node);
void visit_subroutine_body_node(ASTVisitor* visitor, ASTNode* node);
void visit_var_dec_node(ASTVisitor* visitor, ASTNode* node);
void visit_statements_node(ASTVisitor* visitor, ASTNode* node);
void visit_statement_node(ASTVisitor* visitor, ASTNode* node);
void visit_let_statement_node(ASTVisitor* visitor, ASTNode* node);
void visit_if_statement_node(ASTVisitor* visitor, ASTNode* node);
void visit_while_statement_node(ASTVisitor* visitor, ASTNode* node);
void visit_do_statement_node(ASTVisitor* visitor, ASTNode* node);
void visit_return_statement_node(ASTVisitor* visitor, ASTNode* node);
void visit_subroutine_call_node(ASTVisitor* visitor, ASTNode* node);
void visit_expression_node(ASTVisitor* visitor, ASTNode* node);
void visit_operation(ASTVisitor* visitor, ASTNode* node);
void visit_term_node(ASTVisitor* visitor, ASTNode* node);
#endif //AST_H