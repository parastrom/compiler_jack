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

