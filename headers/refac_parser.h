#ifndef REFAC_PARSER_H
#define REFAC_PARSER_H
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

// Define the ASTNode structure
struct ASTNode {
    enum {
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
        NODE_EXPRESSION,
        NODE_TERM,
        // Add more node types here as needed
    } type;
    union {
        ClassNode* classNode;
        ClassVarDecNode* classVarDecNode;
        SubroutineDecNode* subroutineDecNode;
        ParameterListNode* parameterListNode;
        SubroutineBodyNode* subroutineBodyNode;
        VarDecNode* varDecNode;
        StatementsNode* statementsNode;
        StatementNode* statementNode;
        LetStatementNode* letStatementNode;
        IfStatementNode* ifStatementNode;
        WhileStatementNode* whileStatementNode;
        DoStatementNode* doStatementNode;
        ReturnStatementNode* returnStatementNode;
        ExpressionNode* expressionNode;
        TermNode* termNode;
        // Add more node types here as needed
    } data;
};


// Define the structures for each node type
struct ClassNode {
    char* className;
    ClassVarDecNode** classVarDecs;
    int classVarDecCount;
    SubroutineDecNode** subroutineDecs;
    int subroutineDecCount;
};

struct ClassVarDecNode {
    enum { STATIC, FIELD } classVarModifier;
    char* varType;
    char** varNames;
    int varNameCount;
};

struct SubroutineDecNode {
    enum { CONSTRUCTOR, FUNCTION, METHOD } subroutineType;
    char* returnType;
    char* subroutineName;
    ParameterListNode* parameters;
    SubroutineBodyNode* body;
};

struct ParameterListNode {
    char** parameterTypes;
    char** parameterNames;
    int parameterCount;
};

struct SubroutineBodyNode {
    VarDecNode** varDecs;
    int varDecCount;
    StatementsNode* statements;
};

struct VarDecNode {
    char* varType;
    char** varNames;
    int varNameCount;
};

struct StatementsNode {
    StatementNode** statements;
    int statementCount;
};

struct StatementNode {
    enum { LET, IF, WHILE, DO, RETURN } statementType;
    union {
        LetStatementNode* letStatement;
        IfStatementNode* ifStatement;
        WhileStatementNode* whileStatement;
        DoStatementNode* doStatement;
        ReturnStatementNode* returnStatement;
    } data;
};

struct LetStatementNode {
    char* varName;
    ExpressionNode* indexExpression; // NULL if not present
    ExpressionNode* rightExpression;
};

struct IfStatementNode {
    ExpressionNode* condition;
    StatementsNode* ifBranch;
    StatementsNode* elseBranch; // NULL if not present
};

struct WhileStatementNode {
    ExpressionNode* condition;
    StatementsNode* body;
};

struct DoStatementNode {
    char* subroutineName;
    ExpressionNode** expressions;
    int expressionCount;
};

struct ReturnStatementNode {
    ExpressionNode* expression; // NULL if not present
};

struct ExpressionNode {
    TermNode* term;
    struct {
        char op; // Operator character
        TermNode* term;
    }* operations;
    int operationCount;
};

struct TermNode {
    enum { INTEGER_CONSTANT, STRING_CONSTANT, KEYWORD_CONSTANT, VAR_NAME,
           ARRAY_ACCESS, SUBROUTINE_CALL, EXPRESSION, UNARY_OP } termType;
    union {
        int intValue;
        char* stringValue;
        char* keywordValue;
        char* varName;
        struct {
            char* arrayName;
            ExpressionNode* index;
        } arrayAccess;
        struct {
            char* subroutineName;
            ExpressionNode** expressions;
            int expressionCount;
        } subroutineCall;
        ExpressionNode* expression;
        struct {
            char unaryOp;
            TermNode* term;
        } unaryOp;
    } data;
};



#endif // REFAC_PARSER_H