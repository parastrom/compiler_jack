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
                [C_other] = IN_ERROR,
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
                [C_other] = IN_ERROR,
                [C_eof] = IN_ERROR,
        },
        [IN_NUM] = {
                [C_white] = START,
                [C_alpha] = START,
                [C_digit] = IN_NUM,
                [C_double_quote] = IN_STRING,
                [C_slash] = COMMENT_START,
                [C_star] = IN_SYMBOL,
                [C_symbol] = IN_SYMBOL,
                [C_other] = IN_ERROR,
                [C_eof] = IN_ERROR,
        },
        [IN_STRING] = {
                [C_white] = IN_STRING,
                [C_newline] = IN_ERROR,
                [C_alpha] = IN_STRING,
                [C_digit] = IN_STRING,
                [C_double_quote] = START,
                [C_slash] = IN_STRING,
                [C_star] = IN_STRING,
                [C_symbol] = IN_STRING,
                [C_other] = IN_STRING,
                [C_eof] = IN_ERROR,
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
                [C_other] = IN_ERROR,
                [C_eof] = IN_ERROR,
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
                [C_eof] = IN_ERROR,
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
                [C_eof] = IN_ERROR,
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
                [C_eof] = IN_ERROR,
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
                [C_other] = IN_ERROR,
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

    #define EQ_CLASS_DEF_RANGE(start_char, end_char, class_val) \
        for (char c = start_char; c <= end_char; ++c) { \
            eq_classes[c] = class_val; \
        }

    #define EQ_CLASS_DEF_SINGLE(char_val, class_val) eq_classes[char_val] = class_val;

    #include PATH_TO_EQ_DEF_FILE

    #undef EQ_CLASS_DEF_RANGE
    #undef EQ_CLASS_DEF_SINGLE
}

/**
 * Initialize a new lexer with the given filename.
 * The lexer will read the contents of the file into a string buffer.
 * The buffer is owned by the lexer.
 * @param filename The name of the file to be processed by the lexer - 
 * @return A pointer to the initialized lexer.
 */
Lexer* init_lexer(const char *filename, Arena *lexerArena) {
    Lexer* lexer = arena_alloc(lexerArena, sizeof(Lexer));
    if (lexer == NULL) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for the lexer", __func__);
        return NULL;
    }

    lexer->input = read_file_into_string(filename);
    if (lexer->input == NULL) {
        free(lexer);
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for file buffer", __func__);
        return NULL;

    }

    lexer->arena = lexerArena;
    lexer->filename = strdup(filename);

    lexer->position = 0;
    lexer->queue = queue_init(lexerArena);
    lexer->line_starts = vector_create(); // owned by parser
    if (lexer->queue == NULL) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for lexer queue", __func__);
        free(lexer);
        return NULL;
    }

    lexer->error_code = process_input(lexer);

    return lexer;
}

void destroy_lexer(Lexer *lexer) {
    if (lexer != NULL) {
        free(lexer->input);
        free((char*) lexer->filename);
    }
}

/**
 * Determine the TokenType based on the token string and the previous state.
 *
 * @param token_str The token string.
 * @param old_state The previous state of the lexer.
 * @return The TokenType determined based on the token string and the previous state.
 */
TokenType determine_token_type(const char *token_str, int old_state) {
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
void create_token(Lexer *lexer, int old_state, size_t token_start, size_t token_len, int line) {
    char* token_str = strndup(lexer->input + token_start, token_len);
    TokenType type = determine_token_type(token_str, old_state);
    Token* token = new_token(lexer->filename, type, token_str, line, lexer->arena);
    queue_push(lexer->queue, token);
}

/**
 * Process the input string and tokenize it using the lexer.
 *
 * @param lexer The lexer object.
 */
ErrorCode process_input(Lexer *lexer) {
    int state = START;
    int line = 0;
    size_t token_start = 0;
    bool in_comment = false;
    int old_state = START;
    bool was_in_comment = false;
    int token_count = 0;

    while (true) {
        char c = lexer->input[lexer->position];
        EqClasses eq_class = eq_classes[(int) c];
        old_state = state;
        int next_state = transition[state][eq_class];

        // Handle newline increment
        if (c == '\n') {
             line++;
             size_t* offset = arena_alloc(loggerArena, sizeof(size_t));
            *offset = lexer->position;
            vector_push(lexer->line_starts, offset);
        }

        // Check if we were in a comment
        was_in_comment = (state >= IN_COMMENT_SINGLE && state <= SEEN_STAR_IN_COMMENT);

        // Check if we are moving into a comment
        in_comment = (next_state >= IN_COMMENT_SINGLE && next_state <= SEEN_STAR_IN_COMMENT);

        // Check if we have transitioned to a different state and not in a comment
        if (!was_in_comment && next_state != state && !in_comment) {
            // If the old state was not START, IN_SYMBOL, or COMMENT_START, create a token
            if (state != START && state != IN_SYMBOL) {
                size_t adjusted_start = (state == IN_STRING && next_state == START) ? token_start + 1 : token_start;
                size_t length = lexer->position - adjusted_start;
                create_token(lexer, state, adjusted_start, length, line);
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
        if (state == IN_ERROR) {
            if (old_state == IN_STRING && c == '\n') {
                log_error_with_offset(ERROR_PHASE_LEXER, ERROR_LEXER_NEWLINE_IN_STRING, lexer->filename, line,
                                      lexer->position, "['%s'] : A string cannot contain a new line", __func__);
                return ERROR_LEXER_NEWLINE_IN_STRING;
            } else if (old_state == IN_STRING && c == '\0') {
                log_error_with_offset(ERROR_PHASE_LEXER, ERROR_LEXER_EOF_IN_STRING, lexer->filename, line,
                                      lexer->position, "['%s'] : A string cannot contain file EOF", __func__);
                return ERROR_LEXER_EOF_IN_STRING;
            } else if (c == '\0') {
                log_error_with_offset(ERROR_PHASE_LEXER, ERROR_LEXER_UNEXPECTED_EOF, lexer->filename, line,
                                      lexer->position, "['%s'] : Unexpected EOF", __func__);
                return ERROR_LEXER_UNEXPECTED_EOF;
            } else {
                log_error_with_offset(ERROR_PHASE_LEXER, ERROR_LEXER_NEWLINE_IN_STRING, lexer->filename, line,
                                      lexer->position, "['%s'] : Illegal symbol > '%c'", __func__, c);
                return ERROR_LEXER_ILLEGAL_SYMBOL; // Unreachable since log_with_offset "panics"
            }
        }

        lexer->position++;

        if (c == '\0') break; // EOF
    }

    log_message(LOG_LEVEL_DEBUG, ERROR_NONE, "Lexer processed %d tokens\n", token_count);

    return ERROR_NONE;
}











