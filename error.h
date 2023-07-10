typedef enum {
    ERROR_NONE,
    ERROR_INVALID_INPUT,
    ERROR_INVALID_OUTPUT,
    ERROR_MEMORY_ALLOCATION,
    ERROR_OPEN_DIRECTORY
} ErrorCode;


typedef struct {
    ErrorCode code;
    char *msg;
    const char *file;
    int line;
} Error;

Error get_error(void);
void set_error(Error error);