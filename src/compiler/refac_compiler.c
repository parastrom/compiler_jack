//#include "compiler.h"
#include "logger.h"
#include "refac_lexer.h"
#include "refac_parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "safer.h"

typedef struct {
    int parsed;
    int parsed_num;
    char jack_files[32][128];
    char jack_vm_files[32][128];
    size_t num_of_files;
    char* vm_filename;
    FILE* vm_ptr;
    SymbolTable* global_table;
} CompilerState;


int InitCompiler(CompilerState* state) {
    memset(state->jack_files, 0, sizeof state->jack_files);
    memset(state->jack_vm_files, 0, sizeof state->jack_vm_files);
    state->global_table = create_table(SCOPE_GLOBAL, NULL);
    state->num_of_files = 0;

    log_message(LOG_LEVEL_INFO, "Compiler initialized\n");
    return 1;
}

const char* get_file_ext(const char* file_name) {
    const char* dot = strrchr(file_name, '.');
    if(!dot || dot == file_name) return "";
    return dot + 1;
}

// Allocates memory for the file name
const char* remove_ext(const char* myStr) {
    if( myStr == NULL) {
        return NULL;
    }

    size_t len = strlen(myStr);
    const char* lastExt = strrchr(myStr, '.');
    if (lastExt == NULL) {
        return myStr;
    }

    size_t extLen = len - (lastExt - myStr);
    size_t newLen = len - extLen;

    char* retStr = safer_malloc(newLen + 1);

    memcpy(retStr, myStr, newLen);
    retStr[newLen] = '\0';

    return retStr;
}

int find_jack_files(const char* dir_name, CompilerState* state) {
    
    char name_buf[128];
    memset(name_buf, 0, sizeof name_buf);

    snprintf(name_buf, sizeof(name_buf), "./%s", dir_name);
    DIR* dir = opendir(name_buf);
    if (dir == NULL) {
        log_error(ERROR_OPEN_DIRECTORY, __FILE__, __LINE__, "Could not open the given directory : %s\n", name_buf);
        return 0;
    }

    struct dirent* de;
    while((de == readdir(dir)) != NULL) {
        if (strcmp("jack", get_file_ext(de->d_name)) == 0) {
            char* file_path = NULL;
            char* vm_file_path = NULL;

            size_t file_path_len = strlen(name_buf) + strlen(de->d_name) + 2; // +2 for '/' and null terminator

            file_path = malloc(file_path_len);
            if (!file_path) {
                log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for file path\n");
                closedir(dir); // Close the directory before returning
                return 0;
            }

            snprintf(file_path, file_path_len, "%s/%s", name_buf, de->d_name);

            size_t vm_file_path_len = strlen(name_buf) + strlen(de->d_name) + 4; // +4 for ".vm", '/' and null terminator
            vm_file_path = malloc(vm_file_path_len);
            if (!vm_file_path) {
                log_error(ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__, "Could not allocate memory for vm file path\n");
                free(file_path);
                closedir(dir);  // Close the directory before returning
                return 0;
            }

            snprintf(vm_file_path, vm_file_path_len, "%s/%s.vm", name_buf, remove_ext(de->d_name)); //Leaks memory from remove_ext
            
            strcpy(state->jack_files[state->num_of_files++], file_path);
            strcpy(state->jack_vm_files[state->num_of_files++], vm_file_path);

            // Free the dynamically allocated memory
            free(file_path);
            free(vm_file_path);
        }
    }

    closedir(dir);
    return 1;
}



int compile(CompilerState* state) {

    initialize_eq_classes();

    vector stdlib_classes = jack_stdlib_setup();
    add_stdlib_table(state->global_table, stdlib_classes);

    ASTNode* program_node = init_program();
    for (size_t i = 0; i < state->num_of_files; i++) {
        Lexer* lexer = init_lexer(state->jack_files[i]);
        Parser* parser = init_parser(lexer);
        ClassNode* class_node = parse_class(parser);
        vector_push(program_node->data.program->classes, class_node);
    }


    ASTVisitor* visitor = safer_malloc(sizeof(ASTVisitor));
    visitor->visit_ast_node = &visit_ast_node;
    visitor->currentTable = state->global_table;

    ASTVisitor sym_table_builder;

    sym_table_builder.visit_class_node = visit_class_node;
    sym_table_builder.visit_class_var_dec_node = visit_class_var_dec_node;
    sym_table_builder.visit_subroutine_dec_node = visit_subroutine_dec_node;
    sym_table_builder.visit_parameter_list_node = visit_parameter_list_node;
    sym_table_builder.visit_subroutine_body_node = visit_subroutine_body_node;
    sym_table_builder.visit_var_dec_node = visit_var_dec_node;
    sym_table_builder.currentTable = state->global_table;

    for(int i = 0 ; i < vector_size(program_node->data.program->classes); i++) {
        // Use the visitor to visit each class node
        ASTNode* class_node = vector_get(program_node->data.program->classes, i);
        ast_node_accept(class_node, visitor);
    }
}