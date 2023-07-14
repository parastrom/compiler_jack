#include "headers/refac_lexer.h"
#include "headers/logger.h"
#include "headers/token.h"
#include <string.h>

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
        [C_underscore] = IN_ID,
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
        [C_other] = ERROR,
        [C_eof] = START,
    },
    [IN_ID] = {
        [C_white] = START,
        [C_alpha] = IN_ID,
        [C_digit] = IN_ID,
        [C_underscore] = IN_ID,
        [C_double_quote] = START,
        [C_slash] = START,
        [C_symbol] = START,
        [C_eof] = ERROR,
    },
    [IN_NUM] = {
        [C_white] = START,
        [C_alpha] = START,
        [C_digit] = IN_NUM,
        [C_double_quote] = START,
        [C_slash] = START,
        [C_symbol] = START,
        [C_eof] = ERROR, // Handle EOF in number
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
        [C_eof] = ERROR, // Handle EOF in single line comment //
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
        [C_eof] = ERROR, // Handle EOF in multi-line comment
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
        [C_eof] = ERROR, // Handle EOF in multi-line comment
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
        [C_less] = IN_SYMBOL,  // '<<' found
    },
    [SEEN_GREATER_THAN] = {
        [C_equal] = IN_SYMBOL,  // '>=' found
        [C_greater] = IN_SYMBOL,  // '>>' found
    },
};

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
    eq_classes['+'] = C_plus;
    eq_classes['-'] = C_hyphen;
    eq_classes['='] = C_equal;
    eq_classes['!'] = C_exclamation;
    eq_classes['&'] = C_and;
    eq_classes['|'] = C_or;
    eq_classes['<'] = C_less;
    eq_classes['>'] = C_greater;
    eq_classes['.'] = C_period;
    eq_classes['_'] = C_underscore;
    eq_classes['('] = C_symbol;
    eq_classes[')'] = C_symbol;
    eq_classes['['] = C_symbol;
    eq_classes[']'] = C_symbol;
    eq_classes['{'] = C_symbol;
    eq_classes['}'] = C_symbol;
    eq_classes[';'] = C_symbol;
    eq_classes[','] = C_symbol;
    eq_classes['~'] = C_symbol;
}



/**
 * Read the contents of the file and store them in a string buffer.
 *
 * @param filename The name of the file to be read.
 * @return A pointer to the allocated buffer containing the file contents, or NULL on failure.
 *         The buffer is owned by the caller and should be freed when no longer needed.
 */
char* read_file_into_string(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        log_error(ERROR_FILE_OPEN, __FILE__, __LINE__, "Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    if (length < 0) {
        log_error(ERROR_FILE_READ, __FILE__, __LINE__, "Failed to determine file size");
        fclose(file);
        return NULL;
    }
    fseek(file, 0, SEEK_SET);

    if (length == 0) {
        log_error(ERROR_FILE_READ, __FILE__, __LINE__, "File is empty");
        fclose(file);
        return NULL;
    }

    char* buffer = malloc(length + 1 * sizeof(char));
    if (buffer == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Failed to allocate memory for file content");
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, length, file);
    if (bytes_read != (size_t)length) {
        log_error(ERROR_FILE_READ, __FILE__, __LINE__, "Failed to read file");
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[length] = '\0';

    fclose(file);
    return buffer;
}


/**
 * Initialize a new lexer with the given filename.
 *
 * @param filename The name of the file to be processed by the lexer.
 * @return A pointer to the initialized lexer.
 */
Lexer* init_lexer(const char* filename) {
    Lexer* lexer = malloc(sizeof(Lexer));
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
    lexer->position = 0;
    lexer->queue = init_ringbuffer();
    lexer->peeked_token = NULL;
    lexer->current_token = NULL;
    if (lexer->queue == NULL) {
        log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for lexer queue\n");
        free(lexer);
        return NULL;
    }

    process_input(lexer);

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
            if (strlen(token_str) > 1) {
                return token_type_from_str(token_str);
            } else {
                return token_type_from_char(token_str[0]);
            }
        default:
            return TOKEN_TYPE_UNRECOGNISED;
    }
}

/**
 * Process the input string and tokenize it using the lexer.
 *
 * @param lexer The lexer object.
 */
void process_input(Lexer* lexer) {
    int state = START;
    int line = 1;
    size_t token_start = 0;
    bool in_comment = false;
    int token_count = 0;

    while (lexer->input[lexer->position] != '\0') {
        char c = lexer->input[lexer->position++];
        if (c == '\n') line++;

        EqClasses eq_class = eq_classes[(int)c];
        //log_message(LOG_LEVEL_DEBUG, "Read character '%c' (eq_class: %d)\n", c, eq_class);
        int old_state = state;
        state = transition[state][eq_class];
        //log_message(LOG_LEVEL_DEBUG, "State transition: %d -> %d ('%c')\n", old_state, state, c);

        // Check if we are entering or leaving a comment
        if (state == IN_COMMENT_SINGLE || state == IN_COMMENT_MULTI) {
            in_comment = true;
        } else if (old_state == IN_COMMENT_SINGLE || old_state == IN_COMMENT_MULTI) {
            in_comment = false;
        }

        // Start of a new token
        if (!in_comment && old_state == START && state != START) {
            token_start = lexer->position - 1;
        }

        // End of token reached
        if (!in_comment && old_state != START && state == START) {   
            size_t token_len = lexer->position - token_start - 1; 
            const char* token_str = strndup(lexer->input + token_start, token_len);
            TokenType type = determine_token_type(token_str, old_state);
            Token* token = new_token(type, token_str, line);
            ringbuffer_push(lexer->queue, token);
            token_count++;
        } 

        // Handle lexer error
        if (state == ERROR) {
            printf("Lexer error\n");
            // log_error(ERROR_LEXER, __FILE__, __LINE__, "Lexer error");
            return;
        }
    }
    log_message(LOG_LEVEL_DEBUG, "Lexer processed %d tokens\n", token_count);
    // handle end of input...
}





