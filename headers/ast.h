#include "vector.h"

typedef struct ASTNode ASTNode;

// Forward declarations for each node type
typedef struct ProgramNode ProgramNode;
typedef struct ClassNode ClassNode;
typedef struct ClassVarDecNode ClassVarDecNode;
typedef struct SubroutineDecNode SubroutineDecNode;
typedef struct ParameterListNode ParameterListNode;
typedef struct SubroutineBodyNode SubroutineBodyNode;
typedef struct VarDecNode VarDecNode;
typedef struct StatementsNode StatementsNode;
typedef struct StatementNode StatementNode;
typedef struct LetStatementNode LetStatementNode;
typedef struct IfStatementNode IfStatementNode;
typedef struct WhileStatementNode WhileStatementNode;
typedef struct DoStatementNode DoStatementNode;
typedef struct ReturnStatementNode ReturnStatementNode;
typedef struct ExpressionNode ExpressionNode;
typedef struct TermNode TermNode;
typedef struct Operation Operation;
typedef struct VarTerm VarTerm;
typedef struct SubroutineCallNode SubroutineCallNode;

typedef struct {
    void (*visit_program_node)(ProgramNode*);
    void (*visit_class_node)(ClassNode*);
    void (*visit_class_var_dec_node)(ClassVarDecNode*);
    void (*visit_subroutine_dec_node)(SubroutineDecNode*);
    void (*visit_parameter_list_node)(ParameterListNode*);
    void (*visit_subroutine_body_node)(SubroutineBodyNode*);
    void (*visit_var_dec_node)(VarDecNode*);
    void (*visit_statements_node)(StatementsNode*);
    void (*visit_statement_node)(StatementNode*);
    void (*visit_let_statement_node)(LetStatementNode*);
    void (*visit_if_statement_node)(IfStatementNode*);
    void (*visit_while_statement_node)(WhileStatementNode*);
    void (*visit_do_statement_node)(DoStatementNode*);
    void (*visit_return_statement_node)(ReturnStatementNode*);
    void (*visit_subroutine_call_node)(SubroutineCallNode*);
    void (*visit_expression_node)(ExpressionNode*);
    void (*visit_term_node)(TermNode*);
} ASTVisitor;

struct ASTNode {
    enum{
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
    } nodeType;
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


/*
    The ProgramNode is the root node of the AST.
    It contains a vector of ClassNodes.
*/
struct ProgramNode {
    vector classes;
};


/**
*   @brief The ClassNode represents a class declaration.
*   It contains a vector of ClassVarDecNodes and a vector of SubroutineDecNodes.
*/
struct ClassNode {
    char* className;
    vector classVarDecs;
    vector subroutineDecs;
};


/**
 * @brief The ClassVarDecNode represents a class variable declaration.
 * It contains a vector of variable names, the variable type, and the class variable modifier.
 */
struct ClassVarDecNode {
    enum { CVAR_NONE, STATIC, FIELD } classVarModifier;
    char* varType;
    vector varNames;
};

//Write a comment for each node type describing what it represents

/**
 * @brief The SubroutineDecNode represents a subroutine declaration.
 * It contains a list of parameters, the subroutine body, and the subroutine type.
 */

struct SubroutineDecNode {
    enum { SUB_NONE, CONSTRUCTOR, FUNCTION, METHOD } subroutineType;
    char* returnType;
    char* subroutineName;
    ParameterListNode* parameters;
    SubroutineBodyNode* body;
};


/**
 * @brief The ParameterListNode represents a list of parameters.
 * It contains a vector of parameter types and a vector of parameter names.
 * 
 */
struct ParameterListNode {
    vector parameterTypes;
    vector parameterNames;
};

/**
 * @brief The SubroutineBodyNode represents the body of a subroutine.
 *  It contains a vector of variable declarations and a list of statements.
 */
struct SubroutineBodyNode {
    vector varDecs;
    StatementsNode* statements;
};

/**
 * @brief The VarDecNode represents a variable declaration.
 * It contains a vector of variable names and the variable type.
 */
struct VarDecNode {
    char* varType;
    vector varNames;
};

/**
 * @brief The StatementsNode represents a list of statements.
 * It contains a vector of StatementNodes.
 */
struct StatementsNode {
    vector statements;
};

/**
 * @brief The StatementNode represents a statement.
 * It contains a union of the different types of statements.
 */
struct StatementNode {
    enum { STMT_NONE, LET, IF, WHILE, DO, RETURN } statementType;
    union {
        LetStatementNode* letStatement;
        IfStatementNode* ifStatement;
        WhileStatementNode* whileStatement;
        DoStatementNode* doStatement;
        ReturnStatementNode* returnStatement;
    } data;
};

/**
 * @brief The LetStatementNode represents a let statement.
 * It contains the variable name, the index expression (if present), and the right expression.
 */
struct LetStatementNode {
    char* varName;
    ExpressionNode* indexExpression; // NULL if not present
    ExpressionNode* rightExpression;
};

/**
 * @brief The IfStatementNode represents an if statement.
 * It contains the condition expression, the if branch, and the else branch (if present).
 */
struct IfStatementNode {
    ExpressionNode* condition;
    StatementsNode* ifBranch;
    StatementsNode* elseBranch; // NULL if not present
};

/**
 * @brief The WhileStatementNode represents a while statement.
 * It contains the condition expression and the body.
 */
struct WhileStatementNode {
    ExpressionNode* condition;
    StatementsNode* body;
};

/**
 * @brief The DoStatementNode represents a do statement.
 * It contains the subroutine call.
 */
struct DoStatementNode {
    SubroutineCallNode* subroutineCall;
};


/** @brief The ReturnStatementNode represents a return statement.
 * It contains the return expression (if present).
 */
struct ReturnStatementNode {
    ExpressionNode* expression; // NULL if not present
};

/**
 * @brief The SubroutineCallNode represents a subroutine call.
 * It contains the caller (if present), the subroutine name, and the list of expressions.
 */
struct SubroutineCallNode {
    char* caller; // This could be a varName or className. NULL if not present.
    char* subroutineName;
    vector arguments; // vector of ExpressionNode pointers - args
};

/**
 * @brief The ExpressionNode represents an expression.
 * It contains the first term and a vector of operations.
 */
struct ExpressionNode {
    TermNode* term;
    vector operations;
};

/**
 * @brief The Operation represents an operation in an expression.
 * It contains the operator character and the term.
 */
struct Operation {
    char op; // Operator character
    TermNode* term;
};


/**
 * @brief The VarTerm represents a term that is a variable.
 * structure that can handle both varName and className.varName cases
 */
struct VarTerm {
    char* className; // NULL if not present
    char* varName;
};

/**
 * @brief The TermNode represents a term.
 * It contains a union of the different types of terms.
 */
struct TermNode {
    enum { TRM_NONE,INTEGER_CONSTANT, STRING_CONSTANT, KEYWORD_CONSTANT, VAR_TERM,
           ARRAY_ACCESS, SUBROUTINE_CALL, EXPRESSION, UNARY_OP } termType;
    union {
        int intValue;
        char* stringValue;
        char* keywordValue;
        VarTerm varTerm;
        struct {
            char* arrayName;
            ExpressionNode* index;
        } arrayAccess;
        SubroutineCallNode* subroutineCall;
        ExpressionNode* expression;
        struct {
            char unaryOp;
            TermNode* term;
        } unaryOp;
    } data;
};

void class_node_accept(ClassNode* node, ASTVisitor* visitor);
void class_var_dec_node_accept(ClassVarDecNode* node, ASTVisitor* visitor);
void subroutine_dec_node_accept(SubroutineDecNode* node, ASTVisitor* visitor);
void parameter_list_node_accept(ParameterListNode* node, ASTVisitor* visitor);
void subroutine_body_node_accept(SubroutineBodyNode* node, ASTVisitor* visitor);
void var_dec_node_accept(VarDecNode* node, ASTVisitor* visitor);
void statements_node_accept(StatementsNode* node, ASTVisitor* visitor);
void statement_node_accept(StatementNode* node, ASTVisitor* visitor);
void let_statement_node_accept(LetStatementNode* node, ASTVisitor* visitor);
void if_statement_node_accept(IfStatementNode* node, ASTVisitor* visitor);
void while_statement_node_accept(WhileStatementNode* node, ASTVisitor* visitor);
void do_statement_node_accept(DoStatementNode* node, ASTVisitor* visitor);
void return_statement_node_accept(ReturnStatementNode* node, ASTVisitor* visitor);
void subroutine_call_node_accept(SubroutineCallNode* node, ASTVisitor* visitor);
void expression_node_accept(ExpressionNode* node, ASTVisitor* visitor);
void term_node_accept(TermNode* node, ASTVisitor* visitor);
