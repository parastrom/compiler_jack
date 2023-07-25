#include "headers/token.h"
#include "headers/logger.h"
#include <string.h>
#include <stdio.h>

static const TokenMapping tokenMappings[21] = {
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
    {"this", TOKEN_TYPE_THIS},
};

static const TokenCategory tokenCategories[62] = {
    [TOKEN_TYPE_ID] = TOKEN_CATEGORY_TYPE | TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_TILDE] = TOKEN_CATEGORY_FACTOR | TOKEN_CATEGORY_UNARY,
    [TOKEN_TYPE_HYPHEN] = TOKEN_CATEGORY_FACTOR | TOKEN_CATEGORY_UNARY | TOKEN_CATEGORY_ARITH,
    [TOKEN_TYPE_STATIC] = TOKEN_CATEGORY_CLASS_VAR,
    [TOKEN_TYPE_FIELD] = TOKEN_CATEGORY_CLASS_VAR,
    [TOKEN_TYPE_CONSTRUCTOR] = TOKEN_CATEGORY_SUBROUTINE_DEC,
    [TOKEN_TYPE_METHOD] = TOKEN_CATEGORY_SUBROUTINE_DEC,
    [TOKEN_TYPE_FUNCTION] = TOKEN_CATEGORY_SUBROUTINE_DEC,
    [TOKEN_TYPE_VAR] = TOKEN_CATEGORY_STATEMENT,
    [TOKEN_TYPE_LET] = TOKEN_CATEGORY_STATEMENT,
    [TOKEN_TYPE_IF] = TOKEN_CATEGORY_STATEMENT,
    [TOKEN_TYPE_WHILE] = TOKEN_CATEGORY_STATEMENT,
    [TOKEN_TYPE_DO] = TOKEN_CATEGORY_STATEMENT,
    [TOKEN_TYPE_RETURN] = TOKEN_CATEGORY_STATEMENT,
    [TOKEN_TYPE_INT] = TOKEN_CATEGORY_TYPE,
    [TOKEN_TYPE_CHAR] = TOKEN_CATEGORY_TYPE,
    [TOKEN_TYPE_BOOLEAN] = TOKEN_CATEGORY_TYPE,
    [TOKEN_TYPE_VOID] = TOKEN_CATEGORY_TYPE,
    [TOKEN_TYPE_NUM] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_OPEN_PAREN] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_OPEN_BRACKET] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_STRING] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_TRUE] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_FALSE] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_NULL] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_THIS] = TOKEN_CATEGORY_FACTOR,
    [TOKEN_TYPE_EQUAL] = TOKEN_CATEGORY_RELATIONAL,
    [TOKEN_TYPE_GREATER_THAN] = TOKEN_CATEGORY_RELATIONAL,
    [TOKEN_TYPE_LESS_THAN] = TOKEN_CATEGORY_RELATIONAL,
    [TOKEN_TYPE_PLUS] = TOKEN_CATEGORY_ARITH,
    [TOKEN_TYPE_ASTERISK] = TOKEN_CATEGORY_ARITH,
    [TOKEN_TYPE_SLASH] = TOKEN_CATEGORY_ARITH,
    [TOKEN_TYPE_AMPERSAND] = TOKEN_CATEGORY_BOOLEAN,
    [TOKEN_TYPE_BAR] = TOKEN_CATEGORY_BOOLEAN,
};

static const TokenType charToTokenType[256] = {
    ['('] = TOKEN_TYPE_OPEN_PAREN,
    [')'] = TOKEN_TYPE_CLOSE_PAREN,
    ['{'] = TOKEN_TYPE_OPEN_BRACE,
    ['}'] = TOKEN_TYPE_CLOSE_BRACE,
    ['['] = TOKEN_TYPE_OPEN_BRACKET,
    [']'] = TOKEN_TYPE_CLOSE_BRACKET,
    [','] = TOKEN_TYPE_COMMA,
    [';'] = TOKEN_TYPE_SEMICOLON,
    ['='] = TOKEN_TYPE_EQUAL,
    ['.'] = TOKEN_TYPE_PERIOD,
    ['+'] = TOKEN_TYPE_PLUS,
    ['-'] = TOKEN_TYPE_HYPHEN,
    ['*'] = TOKEN_TYPE_ASTERISK,
    ['/'] = TOKEN_TYPE_SLASH,
    ['&'] = TOKEN_TYPE_AMPERSAND,
    ['|'] = TOKEN_TYPE_BAR,
    ['~'] = TOKEN_TYPE_TILDE,
    ['<'] = TOKEN_TYPE_LESS_THAN,
    ['>'] = TOKEN_TYPE_GREATER_THAN,
};

const char *token_type_names[] = {
    "TOKEN_TYPE_UNRECOGNISED", "TOKEN_TYPE_ID", "TOKEN_TYPE_NUM",
    "TOKEN_TYPE_STRING", "TOKEN_TYPE_CLASS", "TOKEN_TYPE_CONSTRUCTOR",
    "TOKEN_TYPE_METHOD", "TOKEN_TYPE_FUNCTION", "TOKEN_TYPE_INT",
    "TOKEN_TYPE_BOOLEAN", "TOKEN_TYPE_CHAR", "TOKEN_TYPE_VOID",
    "TOKEN_TYPE_VAR", "TOKEN_TYPE_STATIC", "TOKEN_TYPE_FIELD",
    "TOKEN_TYPE_LET", "TOKEN_TYPE_DO", "TOKEN_TYPE_IF",
    "TOKEN_TYPE_ELSE", "TOKEN_TYPE_WHILE", "TOKEN_TYPE_RETURN",
    "TOKEN_TYPE_TRUE", "TOKEN_TYPE_FALSE", "TOKEN_TYPE_NULL",
    "TOKEN_TYPE_THIS", "TOKEN_TYPE_OPEN_PAREN", "TOKEN_TYPE_CLOSE_PAREN",
    "TOKEN_TYPE_OPEN_BRACE", "TOKEN_TYPE_CLOSE_BRACE", "TOKEN_TYPE_OPEN_BRACKET",
    "TOKEN_TYPE_CLOSE_BRACKET", "TOKEN_TYPE_COMMA", "TOKEN_TYPE_SEMICOLON",
    "TOKEN_TYPE_EQUAL", "TOKEN_TYPE_PERIOD", "TOKEN_TYPE_PLUS",
    "TOKEN_TYPE_HYPHEN", "TOKEN_TYPE_ASTERISK", "TOKEN_TYPE_SLASH",
    "TOKEN_TYPE_AMPERSAND", "TOKEN_TYPE_BAR", "TOKEN_TYPE_TILDE",
    "TOKEN_TYPE_GREATER_THAN", "TOKEN_TYPE_LESS_THAN"};


const char* token_type_to_string(TokenType type) {
    return token_type_names[type];
};


/**
 * Get the TokenType corresponding to the given string.
 *
 * @param str The string representation of the token.
 * @return The TokenType corresponding to the string, or TOKEN_TYPE_ID if not found.
 */
TokenType token_type_from_str(const char *str)
{
    size_t numMappings = sizeof(tokenMappings) / sizeof(TokenMapping);

    for (size_t i = 0; i < numMappings; i++)
    {
        if (strcmp(str, tokenMappings[i].str) == 0)
        {
            return tokenMappings[i].tokenType;
        }
    }

    return TOKEN_TYPE_ID;
}

/**
 * Get the TokenType corresponding to the given character.
 *
 * @param ch The character representation of the token.
 * @return The TokenType corresponding to the character, or TOKEN_TYPE_UNRECOGNIZED if not found.
 */
TokenType token_type_from_char(char ch)
{
    return charToTokenType[(unsigned char)ch];
}

/**
 * Get the TokenCategory of the given TokenType.
 *
 * @param type The TokenType of the token.
 * @return The TokenCategory of the token, or TOKEN_CATEGORY_UNRECOGNISED if not found.
 */
TokenCategory get_token_category(TokenType type)
{

    size_t numTokenCategories = sizeof(tokenCategories) / sizeof(TokenCategory);

    if (type < numTokenCategories)
    {
        return tokenCategories[type];
    }

    return TOKEN_CATEGORY_UNRECOGNISED;
}

/**
 * Check if a token of the given TokenType belongs to the specified TokenCategory.
 *
 * @param type The TokenType of the token.
 * @param category The TokenCategory to check against.
 * @return True if the token belongs to the category, false otherwise.
 */
bool is_token_category(TokenType type, TokenCategory category)
{
    return (get_token_category(type) & category) != 0;
}

/**
 * Get the TokenType of the given token.
 *
 * @param token The token.
 * @return The TokenType of the token.
 */
TokenType get_token_type(const Token *token)
{
    return token->type;
}

/**
 * Get the lexeme of the given token.
 *
 * @param token The token.
 * @return The lexeme of the token.
 */
char *get_token_lx(const Token *token)
{
    return token->lx;
}

/**
 * Print the formatted representation of the token.
 *
 * @param token The token to format and print.
 */
void fmt(const Token *token)
{
    printf("Token {  type: %-30s Lexeme: %-10s Line: %d }",
           token_type_names[token->type],
           token->lx,
           token->line);
}

/**
 * @brief Formats a token into a string.
 * 
 * @param token 
 * @return char* 
 */

char* token_to_string(const Token *token)
{
    // Assuming the lexeme won't be more than 100 characters.
    // Please adjust this size as per your needs.
    static char buffer[200];
    snprintf(buffer, sizeof(buffer), "Token {  type: %-30s Lexeme: %-10s Line: %d }",
           token_type_names[token->type],
           token->lx,
           token->line);
    return buffer;
}

/**
 * Create a new token. Takes ownership of the 'lx' string,
 *
 * @param type The type of the token.
 * @param lx The string associated with the token.
 * @param line The line number where the token was found.
 * @return A pointer to the newly created token.
 */
Token *new_token(TokenType type, const char *lx, int line)
{
    Token *token = malloc(sizeof(Token));
    if (token == NULL)
    {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for token");
        return NULL;
    }
    token->type = type;
    token->lx = lx; // Take ownership of the lx string
    token->line = line;

    return token;
}

void destroy_token(Token *token)
{
    if (token != NULL)
    {
        free(token->lx); // Free the dynamically allocated memory for 'lx'
        free(token);     // Free the 'Token' struct itself
    }
}

/**
 * Create a new token table with the specified capacity.
 *
 * @param capacity The initial capacity of the token table.
 * @return A pointer to the newly created token table.
 */
TokenTable *new_token_table(size_t capacity)
{
    TokenTable *table = malloc(sizeof(TokenTable));
    if (table == NULL)
    {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for token table");
        return NULL;
    }

    table->keys = calloc(capacity, sizeof(char *));
    table->values = calloc(capacity, sizeof(TokenType));
    table->size = capacity;

    init_token_table(table);

    return table;
}

/**
 * Compute the hash value for the given key.
 *
 * @param key The key to compute the hash value for.
 * @param size The size of the token table.
 * @return The computed hash value.
 */
size_t hash(const char *key, size_t size)
{
    size_t hashval = 0;
    while (*key != '\0')
    {
        hashval = (hashval << 5) - hashval + *key++;
    }
    return hashval % size;
}

/**
 * Insert a key-value pair into the token table.
 *
 * @param table The token table.
 * @param key The key to insert.
 * @param value The value associated with the key.
 */
void token_table_insert(TokenTable *table, const char *key, TokenType value)
{
    size_t index = hash(key, table->size);
    while (table->keys[index] != NULL)
    {
        if (strcmp(table->keys[index], key) == 0)
        {
            table->values[index] = value;
            return;
        }
        index = (index + 1) % table->size;
    }
    table->keys[index] = strdup(key);
    table->values[index] = value;
}

/**
 * Look up the value associated with the given key in the token table.
 *
 * @param table The token table.
 * @param key The key to look up.
 * @return The value associated with the key, or TOKEN_TYPE_ID if not found.
 */
TokenType token_table_lookup(TokenTable *table, const char *key)
{
    size_t index = hash(key, table->size);
    while (table->keys[index] != NULL)
    {
        if (strcmp(table->keys[index], key) == 0)
        {
            return table->values[index];
        }
        index = (index + 1) % table->size;
    }
    return TOKEN_TYPE_ID;
}

void destroy_token_table(TokenTable *table)
{
    if (table != NULL)
    {
        if (table->keys != NULL)
        {
            for (size_t i = 0; i < table->size; i++)
            {
                if (table->keys[i] != NULL)
                {
                    free(table->keys[i]);
                }
            }
            free(table->keys);
        }
        if (table->values != NULL)
        {
            free(table->values);
        }
        free(table);
    }
}

/**
 * Initialize the token table with predefined key-value pairs.
 *
 * @param table The token table to initialize.
 */
void init_token_table(TokenTable *table)
{
    token_table_insert(table, "class", TOKEN_TYPE_CLASS);
    token_table_insert(table, "constructor", TOKEN_TYPE_CONSTRUCTOR);
    token_table_insert(table, "function", TOKEN_TYPE_FUNCTION);
    token_table_insert(table, "method", TOKEN_TYPE_METHOD);
    token_table_insert(table, "field", TOKEN_TYPE_FIELD);
    token_table_insert(table, "static", TOKEN_TYPE_STATIC);
    token_table_insert(table, "var", TOKEN_TYPE_VAR);
    token_table_insert(table, "int", TOKEN_TYPE_INT);
    token_table_insert(table, "char", TOKEN_TYPE_CHAR);
    token_table_insert(table, "boolean", TOKEN_TYPE_BOOLEAN);
    token_table_insert(table, "void", TOKEN_TYPE_VOID);
    token_table_insert(table, "true", TOKEN_TYPE_TRUE);
    token_table_insert(table, "false", TOKEN_TYPE_FALSE);
    token_table_insert(table, "null", TOKEN_TYPE_NULL);
    token_table_insert(table, "this", TOKEN_TYPE_THIS);
    token_table_insert(table, "let", TOKEN_TYPE_LET);
    token_table_insert(table, "do", TOKEN_TYPE_DO);
    token_table_insert(table, "if", TOKEN_TYPE_IF);
    token_table_insert(table, "else", TOKEN_TYPE_ELSE);
    token_table_insert(table, "while", TOKEN_TYPE_WHILE);
    token_table_insert(table, "return", TOKEN_TYPE_RETURN);
}