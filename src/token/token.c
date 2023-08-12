#include "token.h"
#include "logger.h"
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

const char* token_caetgory_names[] = {
        "TOKEN_CATEGORY_UNRECOGNISED",
        "TOKEN_CATEGORY_CLASS_VAR",
        "TOKEN_CATEGORY_SUBROUTINE_DEC",
        "TOKEN_CATEGORY_STATEMENT",
        "TOKEN_CATEGORY_TYPE",
        "TOKEN_CATEGORY_FACTOR",
        "TOKEN_CATEGORY_UNARY",
        "TOKEN_CATEGORY_RELATIONAL",
        "TOKEN_CATEGORY_BOOLEAN",
        "TOKEN_CATEGORY_ARITH",
};


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

const char* category_to_string(TokenCategory category) {
    switch (category) {
        case TOKEN_CATEGORY_UNRECOGNISED:
            return "TOKEN_CATEGORY_UNRECOGNISED";
        case TOKEN_CATEGORY_CLASS_VAR:
            return "TOKEN_CATEGORY_CLASS_VAR";
        case TOKEN_CATEGORY_SUBROUTINE_DEC:
            return "TOKEN_CATEGORY_SUBROUTINE_DEC";
        case TOKEN_CATEGORY_STATEMENT:
            return "TOKEN_CATEGORY_STATEMENT";
        case TOKEN_CATEGORY_TYPE:
            return "TOKEN_CATEGORY_TYPE";
        case TOKEN_CATEGORY_FACTOR:
            return "TOKEN_CATEGORY_FACTOR";
        case TOKEN_CATEGORY_UNARY:
            return "TOKEN_CATEGORY_UNARY";
        case TOKEN_CATEGORY_RELATIONAL:
            return "TOKEN_CATEGORY_RELATIONAL";
        case TOKEN_CATEGORY_BOOLEAN:
            return "TOKEN_CATEGORY_BOOLEAN";
        case TOKEN_CATEGORY_ARITH:
            return "TOKEN_CATEGORY_ARITH";
    }
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
Token *new_token(const char* filename, TokenType type, char *lx, int line, Arena* arena)
{
    Token *token = arena_alloc(arena, sizeof(Token));
    if (token == NULL)
    {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for token");
        return NULL;
    }
    token->filename = filename;
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
    }
}