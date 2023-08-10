#include "refac_lexer.h"
#include "token.h"
#include <string.h>
#include "safer.h"

/**
 * The transition table for the DFA.
 */
static int transition[NUM_STATES][NUM_EQ_CLASSES] = {
    [START] = {
        [C_white] = START,
        [C_newline] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_NUM,
        [C_double_quote] = IN_STRING,
        [C_slash] = COMMENT_START,
        [C_star] = IN_SYMBOL,
        [C_symbol] = IN_SYMBOL,
        [C_other] = ERROR,
        [C_eof] = START,
    },
    [IN_ID] = {
        [C_white] = START,
        [C_newline] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_ID,
        [C_double_quote] = IN_STRING,
        [C_slash] = COMMENT_START,
        [C_star] = IN_SYMBOL,
        [C_symbol] = START,
        [C_other] = ERROR,
        [C_eof] = ERROR,
    },
    [IN_NUM] = {
        [C_white] = START,
        [C_alpha] = START,
        [C_digit] = IN_NUM,
        [C_double_quote] = IN_STRING,
        [C_slash] = COMMENT_START,
        [C_star] = IN_SYMBOL,
        [C_symbol] = IN_SYMBOL,
        [C_other] = ERROR,
        [C_eof] = ERROR,
    },
    [IN_STRING] = {
        [C_white] = IN_STRING,
        [C_newline] = ERROR,
        [C_alpha] = IN_STRING,
        [C_digit] = IN_STRING,
        [C_double_quote] = START,
        [C_slash] = IN_STRING,
        [C_star] = IN_STRING,
        [C_symbol] = IN_STRING,
        [C_other] = IN_STRING,
        [C_eof] = ERROR,
    },
    [COMMENT_START] = {
        [C_white] = START,
        [C_newline] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_NUM,
        [C_double_quote] = IN_STRING,
        [C_slash] = IN_COMMENT_SINGLE,
        [C_star] = IN_COMMENT_MULTI,
        [C_symbol] = IN_SYMBOL,
        [C_other] = ERROR,
        [C_eof] = ERROR,
    },
    [IN_COMMENT_SINGLE] = {
        [C_white] = IN_COMMENT_SINGLE,
        [C_newline] = START,
        [C_alpha] = IN_COMMENT_SINGLE,
        [C_digit] = IN_COMMENT_SINGLE,
        [C_double_quote] = IN_COMMENT_SINGLE,
        [C_slash] = IN_COMMENT_SINGLE,
        [C_star] = IN_COMMENT_SINGLE,
        [C_symbol] = IN_COMMENT_SINGLE,
        [C_other] = IN_COMMENT_SINGLE,
        [C_eof] = ERROR,
    },
    [IN_COMMENT_MULTI] = {
        [C_white] = IN_COMMENT_MULTI,
        [C_newline] = IN_COMMENT_MULTI,
        [C_alpha] = IN_COMMENT_MULTI,
        [C_digit] = IN_COMMENT_MULTI,
        [C_double_quote] = IN_COMMENT_MULTI,
        [C_slash] = IN_COMMENT_MULTI,
        [C_star] = SEEN_STAR_IN_COMMENT,
        [C_symbol] = IN_COMMENT_MULTI,
        [C_other] = IN_COMMENT_MULTI,
        [C_eof] = ERROR,
    },
    [SEEN_STAR_IN_COMMENT] = {
        [C_white] = IN_COMMENT_MULTI,
        [C_newline] = IN_COMMENT_MULTI,
        [C_alpha] = IN_COMMENT_MULTI,
        [C_digit] = IN_COMMENT_MULTI,
        [C_double_quote] = IN_COMMENT_MULTI,
        [C_slash] = START,
        [C_star] = SEEN_STAR_IN_COMMENT,
        [C_symbol] = IN_COMMENT_MULTI,
        [C_other] = IN_COMMENT_MULTI,
        [C_eof] = ERROR,
    },
    [IN_SYMBOL] = {
        [C_white] = START,
        [C_newline] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_NUM,
        [C_double_quote] = IN_STRING,
        [C_slash] = COMMENT_START,
        [C_star] = IN_SYMBOL,
        [C_symbol] = IN_SYMBOL,
        [C_other] = ERROR,
        [C_eof] = START,
    },
};



/**
 * The equivalence classes for character classification.
 */
EqClasses eq_classes[128];
/**
 * Initialize the equivalence classes for character classification.
 */
void initialize_eq_classes() {
    for (int i = 0; i < 128; ++i) {
        eq_classes[i] = C_other;
    }

    for (char c = 'a'; c <= 'z'; ++c) {
        eq_classes[c] = C_alpha;
    }

    for (char c = 'A'; c <= 'Z'; ++c) {
        eq_classes[c] = C_alpha;
    }

    for (char c = '0'; c <= '9'; ++c) {
        eq_classes[c] = C_digit;
    }

    //Set specific
    eq_classes[' '] = C_white;
    eq_classes['\t'] = C_white;
    eq_classes['\r'] = C_white;
    eq_classes['\n'] = C_newline;
    eq_classes['"'] = C_double_quote;
    eq_classes['/'] = C_slash;
    eq_classes['*'] = C_star;
    eq_classes['+'] = C_symbol; 
    eq_classes['-'] = C_symbol;
    eq_classes['='] = C_symbol;
    eq_classes['!'] = C_symbol;
    eq_classes['&'] = C_symbol;
    eq_classes['|'] = C_symbol;
    eq_classes['<'] = C_symbol;
    eq_classes['>'] = C_symbol;
    eq_classes['.'] = C_symbol;
    eq_classes['_'] = C_alpha;
    eq_classes['('] = C_symbol;
    eq_classes[')'] = C_symbol;
    eq_classes['['] = C_symbol;
    eq_classes[']'] = C_symbol;
    eq_classes['{'] = C_symbol;
    eq_classes['}'] = C_symbol;
    eq_classes[';'] = C_symbol;
    eq_classes[','] = C_symbol;
    eq_classes['~'] = C_symbol;
    eq_classes['\0'] = C_eof;
}

/**
 * Initialize a new lexer with the given filename.
 * The lexer will read the contents of the file into a string buffer.
 * The buffer is owned by the lexer.
 * @param filename The name of the file to be processed by the lexer - 
 * @return A pointer to the initialized lexer.
 */
Lexer* init_lexer(const char* filename) {
    Lexer* lexer = safer_malloc(sizeof(Lexer));
    if (lexer == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for lexer\n");
        return NULL;
    }

    lexer->input = read_file_into_string(filename);
    if (lexer->input == NULL) {
        free(lexer);
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for lexer input\n");
        return NULL;
        
    }
    
    lexer->filename = strdup(filename);
    
    lexer->position = 0;
    lexer->queue = init_ringbuffer();
    if (lexer->queue == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for lexer queue\n");
        free(lexer);
        return NULL;
    }

    lexer->error_code =process_input(lexer);

    return lexer;
}

void destroy_lexer(Lexer* lexer) {
    if (lexer != NULL) {
        if (lexer->input != NULL) {
            free(lexer->input);
        }
        if (lexer->queue != NULL) {
            ringbuffer_destroy(lexer->queue);
        }
        free(lexer);
    }
}

/**
 * Strip newline and carriage return characters from a string.
 *
 * @param str The input string.
 * @return A newly allocated string without newline and carriage return characters.
 *         The caller owns the memory and should free it when no longer needed.
 */
char* strip(const char* str) {
    size_t len = strlen(str);
    char* p = calloc(len + 1, sizeof(char));

    if (p) {
        size_t i = 0;
        for (size_t j = 0; j < len; j++) {
            if (str[j] != '\r' && str[j] != '\n') {
                p[i++] = str[j];
            }
        }
        p[i] = '\0';
    }

    return p;
}

/**
 * Determine the TokenType based on the token string and the previous state.
 *
 * @param token_str The token string.
 * @param old_state The previous state of the lexer.
 * @return The TokenType determined based on the token string and the previous state.
 */
TokenType determine_token_type(const char* token_str, int old_state) {
    switch (old_state) {
        case IN_ID:
            return token_type_from_str(token_str);
        case IN_NUM:
            return TOKEN_TYPE_NUM;
        case IN_STRING:
            return TOKEN_TYPE_STRING;
        case IN_SYMBOL:
        case COMMENT_START:
            if (strlen(token_str) == 1) {
                return token_type_from_char(token_str[0]);
            }
        default:
            return TOKEN_TYPE_UNRECOGNISED;
    }
}


/**
 * @brief Create a token object
 * 
 * @param lexer - The lexer object
 * @param old_state - The previous state of the lexer
 * @param token_start - The start position of the token
 * @param token_len - The length of the token
 * @param line - The line number of the token
 * @param token_count - The number of tokens created so far
 */
void create_token(Lexer* lexer, int old_state, size_t token_start, size_t token_len, int line) {
    const char* token_str = strndup(lexer->input + token_start, token_len);
    TokenType type = determine_token_type(token_str, old_state);
    Token* token = new_token(type, token_str, line);
    ringbuffer_push(lexer->queue, token);
}

/**
 * Process the input string and tokenize it using the lexer.
 *
 * @param lexer The lexer object.
 */
ErrorCode process_input(Lexer* lexer) {
    int state = START;
    int line = 1;
    size_t token_start = 0;
    bool in_comment = false;
    int old_state = START;
    bool was_in_comment = false;
    int token_count = 0;

    while (true) {
        char c = lexer->input[lexer->position];
        EqClasses eq_class = eq_classes[(int)c];
        old_state = state;
        int next_state = transition[state][eq_class];

        // Handle newline increment
        if (c == '\n') line++;

        // Check if we were in a comment
        was_in_comment = (state >= IN_COMMENT_SINGLE && state <= SEEN_STAR_IN_COMMENT);

        // Check if we are moving into a comment
        in_comment = (next_state >= IN_COMMENT_SINGLE && next_state <= SEEN_STAR_IN_COMMENT);

        // Check if we have transitioned to a different state and not in a comment
        if (!was_in_comment && next_state != state && !in_comment) {
            // If the old state was not START, IN_SYMBOL, or COMMENT_START, create a token
            if (state != START && state != IN_SYMBOL) {
                create_token(lexer, state, token_start, lexer->position - token_start, line);
                token_count++;
            }
            // Update token_start when transitioning from START state to another state
            if (state == START || state == COMMENT_START) {
                token_start = lexer->position;
            }
        }

        state = next_state;  // update state after generating tokens
        
        // If the current character is a symbol and we are not in a string or comment, create a new token for it
        if ((eq_class == C_symbol || eq_class == C_star) && state != IN_STRING && !in_comment) {
            create_token(lexer, IN_SYMBOL, lexer->position, 1, line);
            token_count++;
            token_start = lexer->position + 1;
        }

       // Handle lexer error
        if (state == ERROR) {
            if (old_state == IN_STRING && c == '\n') {
                log_error(ERROR_LEXER_NEWLINE_IN_STRING, __FILE__, line, "Lexer error: newline in string at line %d", line);
                return ERROR_LEXER_NEWLINE_IN_STRING;
            } else if (old_state == IN_STRING && c == '\0') {
                log_error(ERROR_LEXER_EOF_IN_STRING, __FILE__, line, "Lexer error: EOF in string at line %d", line);
                return ERROR_LEXER_EOF_IN_STRING;
            } else if (c == '\0') {
                log_error(ERROR_LEXER_UNEXPECTED_EOF, __FILE__, line, "Lexer error: unexpected EOF at line %d", line);
                return ERROR_LEXER_UNEXPECTED_EOF;
            } else {
                log_error(ERROR_LEXER_ILLEGAL_SYMBOL, __FILE__, line, "Lexer error: illegal symbol '%c' at line %d", c, line);
                return ERROR_LEXER_ILLEGAL_SYMBOL;
            }
        }

        // Advance lexer position
        lexer->position++;

        if (c == '\0') break; // end of input
    }

    log_message(LOG_LEVEL_DEBUG, "Lexer processed %d tokens\n", token_count);
    
    return ERROR_NONE;
}











