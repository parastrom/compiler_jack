#include "headers/ast.h"
#include "headers/symbol.h"

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
                log_error(ERROR_INVALID_INPUT, __FILE__, __LINE__, "Invalid class var modifier");
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void visit_subroutine_dec_node(ASTVisitor* visitor, SubroutineDecNode* node) {
    
    SymbolTable* table;
    visitor->currentTable = table;

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
            log_error(ERROR_INVALID_INPUT, __FILE__, __LINE__, "Invalid subroutine type");
            exit(EXIT_FAILURE);
            break;
    }

    visitor->currentTable = table;
   
    parameter_list_node_accept(node->parameters, visitor);

    subroutine_body_node_accept(node->body, visitor);

    visitor->currentTable = visitor->currentTable->parent;
}

