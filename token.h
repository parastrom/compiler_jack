#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

typedef enum {
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
    TOKEN_TYPE_UNRECOGNISED
} TokenType;

typedef struct {
    TokenType type;
    char lx[128];
    int line;
    char fl[64];
} Token;

typedef struct {
    const char* str;
    TokenType tokenType;
} TokenMapping;

typedef struct {
    char ch;
    TokenType tokenType;
} CharMapping;


const TokenMapping tokenMappings[] = {
    {"class", TOKEN_TYPE_CLASS},
    {"constructor", TOKEN_TYPE_CONSTRUCTOR},
    {"method", TOKEN_TYPE_METHOD},
    {"function", TOKEN_TYPE_FUNCTION},
    {"int", TOKEN_TYPE_INT},
    {"boolean", TOKEN_TYPE_BOOLEAN},
    {"char", TOKEN_TYPE_CHAR},
    {"var", TOKEN_TYPE_VAR},
    {"void", TOKEN_TYPE_VOID},
    {"static", TOKEN_TYPE_STATIC},
    {"field", TOKEN_TYPE_FIELD},
    {"let", TOKEN_TYPE_LET},
    {"do", TOKEN_TYPE_DO},
    {"if", TOKEN_TYPE_IF},
    {"else", TOKEN_TYPE_ELSE},
    {"while", TOKEN_TYPE_WHILE},
    {"return", TOKEN_TYPE_RETURN},
    {"true", TOKEN_TYPE_TRUE},
    {"false", TOKEN_TYPE_FALSE},
    {"null", TOKEN_TYPE_NULL},
    {"this", TOKEN_TYPE_THIS}
};

const CharMapping charMappings[] = {
    {'(', TOKEN_TYPE_OPEN_PAREN},
    {')', TOKEN_TYPE_CLOSE_PAREN},
    {'{', TOKEN_TYPE_OPEN_BRACE},
    {'}', TOKEN_TYPE_CLOSE_BRACE},
    {'[', TOKEN_TYPE_OPEN_BRACKET},
    {']', TOKEN_TYPE_CLOSE_BRACKET},
    {',', TOKEN_TYPE_COMMA},
    {';', TOKEN_TYPE_SEMICOLON},
    {'=', TOKEN_TYPE_EQUAL},
    {'.', TOKEN_TYPE_PERIOD},
    {'+', TOKEN_TYPE_PLUS},
    {'-', TOKEN_TYPE_HYPHEN},
    {'*', TOKEN_TYPE_ASTERISK},
    {'/', TOKEN_TYPE_SLASH},
    {'&', TOKEN_TYPE_AMPERSAND},
    {'|', TOKEN_TYPE_BAR},
    {'~', TOKEN_TYPE_TILDE},
    {'<', TOKEN_TYPE_LESS_THAN},
    {'>', TOKEN_TYPE_GREATER_THAN}
};


TokenType token_type_from_str(const char* str);
TokenType token_type_from_char(char ch);

bool is_token_type_class_var(TokenType type);
bool is_token_type_sub_dec(TokenType type);
bool is_token_type_stmt(TokenType type);
bool is_token_type_type(TokenType type);
bool is_token_type_factor(TokenType type);
bool is_token_type_unary(TokenType type);
bool is_token_type_relational(TokenType type);
bool is_token_type_bool(TokenType type);
bool is_token_type_arith(TokenType type);

TokenType get_token_type(const Token* token);
char* get_token_lx(const Token* token);

void fmt(const Token* token);


#endif  // TOKEN_H