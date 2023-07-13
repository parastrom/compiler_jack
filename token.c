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

// Will default to TOKEN_TYPE_UNRECOGNIZED if the character is not recognized
TokenType token_type_from_char(char ch) {
    return charToTokenType[(unsigned char)ch];
}

TokenCategory get_token_category(TokenType type) {

    size_t numTokenCategories = sizeof(tokenCategories) / sizeof(TokenCategory);

    if (type < numTokenCategories) {
        return tokenCategories[type];
    }

    return TOKEN_CATEGORY_UNRECOGNISED;
}

bool is_token_of_category(TokenType type, TokenCategory category) {
    return (get_token_category(type) & category) != 0;
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
}


Token* init_token(TokenType type, const char* lx, int line) {
    Token* token = malloc(sizeof(Token));
    if (token == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for token");
        return NULL;
    }
    token->type = type;
    strcpy(token->lx, lx);
    token->line = line;

    return token;
}

void destroy_token(Token* token) {
    if (token != NULL) {
        free(token);
    }
}