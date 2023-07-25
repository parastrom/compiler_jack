#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>
#include <stdio.h>

typedef enum
{
    TOKEN_TYPE_UNRECOGNISED = 0,
    TOKEN_TYPE_ID,
    TOKEN_TYPE_NUM,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_CLASS,
    TOKEN_TYPE_CONSTRUCTOR,
    TOKEN_TYPE_METHOD,
    TOKEN_TYPE_FUNCTION,
    TOKEN_TYPE_INT,
    TOKEN_TYPE_BOOLEAN,
    TOKEN_TYPE_CHAR,
    TOKEN_TYPE_VOID,
    TOKEN_TYPE_VAR,
    TOKEN_TYPE_STATIC,
    TOKEN_TYPE_FIELD,
    TOKEN_TYPE_LET,
    TOKEN_TYPE_DO,
    TOKEN_TYPE_IF,
    TOKEN_TYPE_ELSE,
    TOKEN_TYPE_WHILE,
    TOKEN_TYPE_RETURN,
    TOKEN_TYPE_TRUE,
    TOKEN_TYPE_FALSE,
    TOKEN_TYPE_NULL,
    TOKEN_TYPE_THIS,
    TOKEN_TYPE_OPEN_PAREN,
    TOKEN_TYPE_CLOSE_PAREN,
    TOKEN_TYPE_OPEN_BRACE,
    TOKEN_TYPE_CLOSE_BRACE,
    TOKEN_TYPE_OPEN_BRACKET,
    TOKEN_TYPE_CLOSE_BRACKET,
    TOKEN_TYPE_COMMA,
    TOKEN_TYPE_SEMICOLON,
    TOKEN_TYPE_EQUAL,
    TOKEN_TYPE_PERIOD,
    TOKEN_TYPE_PLUS,
    TOKEN_TYPE_HYPHEN,
    TOKEN_TYPE_ASTERISK,
    TOKEN_TYPE_SLASH,
    TOKEN_TYPE_AMPERSAND,
    TOKEN_TYPE_BAR,
    TOKEN_TYPE_TILDE,
    TOKEN_TYPE_GREATER_THAN,
    TOKEN_TYPE_LESS_THAN,
} TokenType;

typedef enum
{
    TOKEN_CATEGORY_UNRECOGNISED = 0,
    TOKEN_CATEGORY_CLASS_VAR = 1 << 0,
    TOKEN_CATEGORY_SUBROUTINE_DEC = 1 << 1,
    TOKEN_CATEGORY_STATEMENT = 1 << 2,
    TOKEN_CATEGORY_TYPE = 1 << 3,
    TOKEN_CATEGORY_FACTOR = 1 << 4,
    TOKEN_CATEGORY_UNARY = 1 << 5,
    TOKEN_CATEGORY_RELATIONAL = 1 << 6,
    TOKEN_CATEGORY_BOOLEAN = 1 << 7,
    TOKEN_CATEGORY_ARITH = 1 << 8
} TokenCategory;

typedef struct
{
    TokenType type;
    char* lx;
    int line;
} Token;

typedef struct
{
    const char *str;
    TokenType tokenType;
} TokenMapping;


typedef struct {
    char** keys;
    TokenType* values;
    size_t size;
} TokenTable;


TokenTable* new_token_table(size_t size);
size_t hash(const char* str, size_t size);
void token_table_insert(TokenTable* table, const char* key, TokenType value);
TokenType token_table_lookup(TokenTable* table, const char* key);
void init_token_table(TokenTable* table);


const char* token_type_to_string(TokenType type);
TokenType token_type_from_str(const char *str);
TokenType token_type_from_char(char ch);

TokenCategory get_token_category(TokenType type);
bool is_token_category(TokenType type, TokenCategory category);
Token *new_token(TokenType type, const char *lx, int line);
void destroy_token(Token *token);
TokenType get_token_type(const Token *token);
char *get_token_lx(const Token *token);

void fmt(const Token *token);
char* token_to_string(const Token *token);

#endif // TOKEN_H