#ifndef REFAC_LEXER_H
#define REFAC_LEXER_H

#include <stdio.h>
#include "ringbuffer.h"


#define NUM_STATES 22
#define NUM_EQ_CLASSES 20
typedef struct {
    const char* input;
    size_t position;
    Token* peeked_token;
    Token* current_token;
    int cur_len;
    RingBuffer* queue;
} Lexer;



typedef enum
{
    C_white,        // space // \t \r
    C_newline,      // \n
    C_alpha,        // a-z // A-Z
    C_digit,        // 0-9
    C_double_quote, // "
    C_slash,        // </>
    C_star,         // <*>
    C_less,         // '<'
    C_greater,      // '>'
    C_equal,        // '='
    C_and,          // '&'
    C_hyphen,       // '-'
    C_or,           // '|'
    C_exclamation,  // '!'
    C_plus,         // '+'
    C_period,         // '.'
    C_underscore,   // '_'
    C_symbol,       // <all symbols>
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
    SEEN_LESS_THAN,
    SEEN_GREATER_THAN,
    SEEN_EQUAL,
    SEEN_EXCLAMATION,
    SEEN_AND,
    SEEN_HYPHEN,
    SEEN_PLUS,
    SEEN_ASTERISK,
    SEEN_SLASH,
    SEEN_OR,
    SEEN_STAR_IN_COMMENT,
    ERROR,
    IN_SYMBOL
} States;



Lexer* init_lexer(const char* input);
void initialize_eq_classes();
void process_input(Lexer* lexer);
void destroy_lexer(Lexer* lexer);
char* read_file_into_string(const char* filename);
char* strip(const char* str);



#endif  // REFAC_LEXER_H