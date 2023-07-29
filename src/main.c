#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "refac_parser.h"
#include "print_ast.h"

#ifndef JACK_FILES_DIR
    #define JACK_FILES_DIR "./jack_files"  // Fallback if JACK_FILES_DIR is not defined in CMakeLists.txt
#endif

#define PATH_LEN_MAX 1024

int main() {
    const char* jackFilesDir = JACK_FILES_DIR;
    printf("JACK files directory: %s\n", jackFilesDir);

    // Open the directory
    DIR* dir = opendir(jackFilesDir);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", jackFilesDir);
        return 1;
    }
    
    initialize_eq_classes();
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the path to the JACK file
        char filePath[PATH_LEN_MAX];
        snprintf(filePath, sizeof(filePath), "%s/%s", jackFilesDir, entry->d_name);

        // Tokenize the JACK file
        Lexer* lexer = init_lexer(filePath);
        if (lexer == NULL) {
            printf("Failed to initialize lexer for file: %s\n", filePath);
            continue;
        }

        Parser* parser = init_parser(lexer);
        ClassNode const* classNode = parse_class(parser);
        print_class(classNode);

        // Print the AST
        printf("AST for file: %s\n", filePath);
        if(classNode != NULL) {
            log_message(LOG_LEVEL_INFO, "AST for file: %s\n", filePath);
        } else {
            printf("Failed to parse file: %s\n", filePath);
        }

        break;
        //destroy_lexer(lexer);
    }

    // Close the directory
    closedir(dir);
    close_log_file();

    return 0;
}
