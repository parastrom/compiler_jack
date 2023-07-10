#include "token.h"
#include "logger.h"
#include "error.h"
#include <string.h>
#include <stdio.h>


TokenType token_type_from_str(const char* str) {
    size_t numMappings = sizeof(tokenMappings) / sizeof(TokenMapping);

    for (size_t i = 0; i < numMappings; i++) {
        if (strcmp(str, tokenMappings[i].str) == 0) {
            return tokenMappings[i].tokenType;
        }
    }

    return TOKEN_TYPE_ID;
}

TokenType token_type_from_char(char ch) {
    size_t numMappings = sizeof(charMappings) / sizeof(CharMapping);

    for (size_t i = 0; i < numMappings; i++) {
        if (ch == charMappings[i].ch) {
            return charMappings[i].tokenType;
        }
    }

    return TOKEN_TYPE_UNRECOGNISED;
}

bool is_token_type_class_var(TokenType type) {
    return (type == TOKEN_TYPE_STATIC || type == TOKEN_TYPE_FIELD);
}

bool is_token_type_sub_dec(TokenType type) {
    return (type == TOKEN_TYPE_CONSTRUCTOR || type == TOKEN_TYPE_METHOD || type == TOKEN_TYPE_FUNCTION);
}

bool is_token_type_stmt(TokenType type) {
    return (type == TOKEN_TYPE_VAR || type == TOKEN_TYPE_LET || type == TOKEN_TYPE_IF ||
            type == TOKEN_TYPE_WHILE || type == TOKEN_TYPE_DO || type == TOKEN_TYPE_RETURN);
}

bool is_token_type_type(TokenType type) {
    return (type == TOKEN_TYPE_INT || type == TOKEN_TYPE_CHAR || type == TOKEN_TYPE_BOOLEAN ||
            type == TOKEN_TYPE_ID || type == TOKEN_TYPE_VOID);
}

bool is_token_type_factor(TokenType type) {
    return (type == TOKEN_TYPE_NUM || type == TOKEN_TYPE_ID || type == TOKEN_TYPE_OPEN_PAREN ||
            type == TOKEN_TYPE_OPEN_BRACKET || type == TOKEN_TYPE_STRING || type == TOKEN_TYPE_TRUE ||
            type == TOKEN_TYPE_FALSE || type == TOKEN_TYPE_NULL || type == TOKEN_TYPE_THIS ||
            type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_TILDE);
}

bool is_token_type_unary(TokenType type) {
    return (type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_TILDE);
}

bool is_token_type_relational(TokenType type) {
    return (type == TOKEN_TYPE_EQUAL || type == TOKEN_TYPE_GREATER_THAN || type == TOKEN_TYPE_LESS_THAN);
}

bool is_token_type_bool(TokenType type) {
    return (type == TOKEN_TYPE_AMPERSAND || type == TOKEN_TYPE_BAR);
}

bool is_token_type_arith(TokenType type) {
    return (type == TOKEN_TYPE_PLUS || type == TOKEN_TYPE_SLASH || type == TOKEN_TYPE_HYPHEN || type == TOKEN_TYPE_ASTERISK);
}

TokenType get_token_type(const Token* token) {
    return token->type;
}

char* get_token_lx(const Token* token) {
    return token->lx;
}


void fmt(const Token* token) {
    printf("Token {");
    printf("  type: %d\t", token->type);
    printf("  Lexeme: %s\t", token->lx);
    printf("  Line: %d\t", token->line);
    printf("  File: %s  }\n", token->fl);
}


Token* init_token(TokenType type, const char* lx, int line, const char* fl) {
    Token* token = malloc(sizeof(Token));
    if (token == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for token");
        return NULL;
    }
    token->type = type;
    strcpy(token->lx, lx);
    token->line = line;
    strcpy(token->fl, fl);

    return token;
}

void destroy_token(Token* token) {
    if (token != NULL) {
        free(token);
    }
}