#ifndef REFAC_LEXER_H
#define REFAC_LEXER_H

#define PATH_TO_EQ_DEF_FILE TOSTRING(DEF_FILES_DIR/eq_classes.def)

#include <stdio.h>
#include "logger.h"
#include "token_queue.h"

#define NUM_STATES 11
#define NUM_EQ_CLASSES 10
typedef struct
{
    char *input;
    size_t position;
    const char *filename;
    int cur_len;
    TokenQueue* queue;
    ErrorCode error_code;
    vector line_starts;
    Arena* arena;
    Arena* errorArena;
} Lexer;

typedef enum
{
    C_white,        // space // \t \r
    C_newline,      // \n
    C_alpha,        // a-z // A-Z // _
    C_digit,        // 0-9
    C_double_quote, // "
    C_slash,        // /
    C_star,         // \*
    C_symbol,       // {, }, (, ), [, ], ., ,, ;, +, -, *, /, &, |, <, >, =, ~
    C_other,        // <all other characters>
    C_eof
} EqClasses;

typedef enum
{
    START,
    IN_ID,
    IN_NUM,
    IN_STRING,
    COMMENT_START,
    IN_COMMENT_SINGLE,
    IN_COMMENT_MULTI,
    SEEN_STAR_IN_COMMENT,
    SEEN_END_QUOTE,
    IN_SYMBOL,
    IN_ERROR,
} States;

Lexer *init_lexer(const char *filename, Arena* arena);
void initialize_eq_classes();
void destroy_lexer(Lexer *lexer);
ErrorCode process_input(Lexer *lexer);
char *strip(const char *str);

#endif // REFAC_LEXER_H