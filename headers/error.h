typedef enum {
    ERROR_NONE,
    ERROR_INVALID_INPUT,
    ERROR_INVALID_OUTPUT,
    ERROR_MEMORY_ALLOCATION,
    ERROR_OPEN_DIRECTORY,
    ERROR_BUFFER_FULL,
    ERROR_BUFFER_EMPTY,
    ERROR_NULL_POINTER,
    ERROR_FILE_READ,
    ERROR_FILE_OPEN,
    ERROR_FILE_WRITE,
    ERROR_FILE_CLOSE,
    ERROR_LEXER,
    ERROR_LEXER_NEWLINE_IN_STRING,
    ERROR_LEXER_EOF_IN_STRING,
    ERROR_LEXER_UNEXPECTED_EOF,
    ERROR_LEXER_ILLEGAL_SYMBOL,
} ErrorCode;




typedef struct {
    ErrorCode code;
    char *msg;
    const char *file;
    int line;
} Error;

Error get_error(void);
void set_error(Error error);