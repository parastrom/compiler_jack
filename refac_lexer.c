#include "headers/refac_lexer.h"
#include "headers/logger.h"
#include "headers/token.h"
#include <string.h>

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
        [C_double_quote] = START,
        [C_slash] = START,
        [C_star] = START,
        [C_symbol] = START,
        [C_other] = ERROR,
        [C_eof] = ERROR,
    },
    [IN_NUM] = {
        [C_white] = START,
        [C_alpha] = START,
        [C_digit] = IN_NUM,
        [C_double_quote] = START,
        [C_slash] = START,
        [C_star] = START,
        [C_symbol] = START,
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
        [C_alpha] = START,
        [C_digit] = START,
        [C_double_quote] = START,
        [C_slash] = IN_COMMENT_SINGLE,
        [C_star] = IN_COMMENT_MULTI,
        [C_symbol] = START,
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
        [C_eof] = ERROR,
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
            if (strlen(token_str) == 1) {
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

    while (true) {
        char c = lexer->input[lexer->position];
        EqClasses eq_class = eq_classes[(int)c];
        int old_state = state;
        state = transition[state][eq_class];

        // Handle comment discarding
        if (old_state == COMMENT_START && !(state == IN_COMMENT_SINGLE || state == IN_COMMENT_MULTI)) {
            state = transition[START][eq_class]; // transition as if we were in the START state
            if (state == IN_SYMBOL) {
                size_t token_len = lexer->position - token_start;
                const char* token_str = strndup(lexer->input + token_start, token_len);
                TokenType type = determine_token_type(token_str, old_state);
                Token* token = new_token(type, token_str, line);
                ringbuffer_push(lexer->queue, token);
                token_count++;
                token_start = lexer->position;
            }
        }

        // Handle newline increment
        if (c == '\n') line++;

        // Check if we are not in a comment and have transitioned to a different state
        in_comment = (state >= IN_COMMENT_SINGLE && state <= SEEN_STAR_IN_COMMENT);
        if (!in_comment && old_state != state && old_state != START) {
            if (old_state != IN_SYMBOL) {
                size_t token_len = lexer->position - token_start;
                const char* token_str = strndup(lexer->input + token_start, token_len);
                TokenType type = determine_token_type(token_str, old_state);
                Token* token = new_token(type, token_str, line);
                ringbuffer_push(lexer->queue, token);
                token_count++;
                token_start = lexer->position;
            }
        }

        // If the current character is a symbol, create a new token for it
        if (eq_class == C_symbol) {
            const char* token_str = strndup(&c, 1);
            TokenType type = determine_token_type(token_str, IN_SYMBOL);
            Token* token = new_token(type, token_str, line);
            ringbuffer_push(lexer->queue, token);
            token_count++;
            token_start = lexer->position + 1;
        }

        // Handle lexer error
        if (state == ERROR) {
            printf("Lexer error\n");
            return;
        }

        // Advance lexer position
        lexer->position++;

        if (c == '\0') break; // end of input
    }

    log_message(LOG_LEVEL_DEBUG, "Lexer processed %d tokens\n", token_count);
}




