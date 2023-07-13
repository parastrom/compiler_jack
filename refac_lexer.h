#ifndef REFAC_LEXER_H
#define REFAC_LEXER_H

#include <stdio.h>
#include "ringbuffer.h"
#include "token.h"


#define NUM_STATES 21
#define NUM_EQ_CLASSES 18
typedef struct {
    const char* input;
    size_t position;
    Token* peeked_token;
    Token* current_token;
    int cur_len;
    RingBuffer* queue;
} Lexer;

enum EqClasses
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
    C_symbol,       // <all symbols>
    C_eof
};

enum States
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
    SEEN_GREATER_THAN_SHIFT,
    SEEN_LESS_THAN_SHIFT,
    SEEN_STAR_IN_COMMENT,
    IN_SYMBOL
};

static int transition[NUM_STATES][NUM_EQ_CLASSES] = {
    [START] = {
        [C_white] = START,
        [C_newline] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_NUM,
        [C_double_quote] = IN_STRING,
        [C_slash] = COMMENT_START,
        [C_star] = START,
        [C_symbol] = IN_SYMBOL,
        [C_equal] = SEEN_EQUAL,
        [C_exclamation] = SEEN_EXCLAMATION,
        [C_less] = SEEN_LESS_THAN,
        [C_greater] = SEEN_GREATER_THAN,
        [C_and] = SEEN_AND,
        [C_hyphen] = SEEN_HYPHEN,
        [C_plus] = SEEN_PLUS,
        [C_star] = SEEN_ASTERISK,
        [C_slash] = SEEN_SLASH,
        [C_or] = SEEN_OR,
        [C_eof] = START,
    },
    [IN_ID] = {
        [C_white] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_ID,
        [C_double_quote] = START,
        [C_slash] = START,
        [C_symbol] = START,
        [C_eof] = START,
    },
    [IN_NUM] = {
        [C_white] = START,
        [C_alpha] = START,
        [C_digit] = IN_NUM,
        [C_double_quote] = START,
        [C_slash] = START,
        [C_symbol] = START,
        [C_eof] = START, // Handle EOF in number
    },
    [IN_STRING] = {
        [C_white] = IN_STRING,
        [C_alpha] = IN_STRING,
        [C_digit] = IN_STRING,
        [C_double_quote] = START,
        [C_slash] = IN_STRING,
        [C_symbol] = IN_STRING,
        [C_eof] = START, // Handle EOF in string
    },
    [COMMENT_START] = {
        [C_white] = START,
        [C_alpha] = START,
        [C_digit] = START,
        [C_double_quote] = START,
        [C_slash] = IN_COMMENT_SINGLE,
        [C_star] = IN_COMMENT_MULTI,
        [C_symbol] = START,
        [C_eof] = START,
    },
    [IN_COMMENT_SINGLE] = {
        [C_white] = IN_COMMENT_SINGLE,
        [C_alpha] = IN_COMMENT_SINGLE,
        [C_digit] = IN_COMMENT_SINGLE,
        [C_double_quote] = IN_COMMENT_SINGLE,
        [C_slash] = IN_COMMENT_SINGLE, 
        [C_star] = IN_COMMENT_SINGLE,
        [C_symbol] = IN_COMMENT_SINGLE,
        [C_newline] = START,  // end of single-line comment
        [C_eof] = START, // Handle EOF in single line comment //
    },
    [IN_COMMENT_MULTI] = {
        [C_white] = IN_COMMENT_MULTI,
        [C_newline] = IN_COMMENT_MULTI,
        [C_alpha] = IN_COMMENT_MULTI,
        [C_digit] = IN_COMMENT_MULTI,
        [C_double_quote] = IN_COMMENT_MULTI,
        [C_slash] = IN_COMMENT_MULTI, 
        [C_star] = SEEN_STAR_IN_COMMENT, // potential end of comment
        [C_symbol] = IN_COMMENT_MULTI,
        [C_eof] = START, // Handle EOF in multi-line comment
    },
    [SEEN_STAR_IN_COMMENT] = {
        [C_white] = IN_COMMENT_MULTI, 
        [C_newline] = IN_COMMENT_MULTI, 
        [C_alpha] = IN_COMMENT_MULTI, 
        [C_digit] = IN_COMMENT_MULTI, 
        [C_double_quote] = IN_COMMENT_MULTI, 
        [C_slash] = START, // end of comment
        [C_star] = SEEN_STAR_IN_COMMENT, // it's still potential end of comment
        [C_symbol] = IN_COMMENT_MULTI, 
        [C_eof] = START, // Handle EOF in multi-line comment
    },
    [IN_SYMBOL] = {
        [C_white] = START,
        [C_alpha] = START,
        [C_digit] = START,
        [C_double_quote] = START,
        [C_slash] = START,
        [C_star] = START,
        [C_symbol] = START,
        [C_eof] = START,
    },
    [SEEN_EQUAL] = {
        [C_equal] = IN_SYMBOL,  // '==' found
    },
    [SEEN_EXCLAMATION] = {
        [C_equal] = IN_SYMBOL,  // '!=' found
    },
    [SEEN_AND] = {
        [C_and] = IN_SYMBOL,  // '&&' found
        [C_equal] = IN_SYMBOL,  // '&=' found
    },
    [SEEN_HYPHEN] = {
        [C_hyphen] = IN_SYMBOL,  // '--' found
        [C_equal] = IN_SYMBOL,  // '-=' found
    },
    [SEEN_PLUS] = {
        [C_plus] = IN_SYMBOL,  // '++' found
        [C_equal] = IN_SYMBOL,  // '+=' found
    },
    [SEEN_ASTERISK] = {
        [C_equal] = IN_SYMBOL,  // '*=' found
    },
    [SEEN_OR] = {
        [C_or] = IN_SYMBOL,  // '||' found
        [C_equal] = IN_SYMBOL,  // '|=' found
    },
    [SEEN_LESS_THAN] = {
        [C_equal] = IN_SYMBOL,  // '<=' found
        [C_less] = SEEN_LESS_THAN_SHIFT,  // '<<' found
    },
    [SEEN_GREATER_THAN] = {
        [C_equal] = IN_SYMBOL,  // '>=' found
        [C_greater] = SEEN_GREATER_THAN_SHIFT,  // '>>' found
    },
    [SEEN_LESS_THAN_SHIFT] = {
        [C_equal] = IN_SYMBOL,  // '<<=' found
    },
    [SEEN_GREATER_THAN_SHIFT] = {
        [C_equal] = IN_SYMBOL,  // '>>=' found
    },
};



Lexer* init_lexer(const char* input);
char* read_file_into_string(const char* filename);
char* strip(const char* str);



#endif  // REFAC_LEXER_H