#include "compiler.h"

typedef struct {
    int parsed;
    int parsed_num;
    char jack_files[32][128];
    char jack_vm_files[32][128];
    int num_of_files;
    char* vm_filename;
    FILE* vm_ptr;
} CompilerState;


int InitCompiler(CompilerState* state) {
    memset(state->jack_files, 0, sizeof state->jack_files);
    memset(state->jack_vm_files, 0, sizeof state->jack_vm_files);
    state->num_of_files = 0;
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

    char* retStr = malloc(newLen + 1);
    if (retStr == NULL) {
        return NULL;
    }

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
        printf("Error: Could not open directory %s\n", dir_name);
        return 0;
    }

    struct dirent* de;
    while((de == readdir(dir)) != NULL) {
        if (strcmp("jack", get_file_ext(de->d_name)) == 0) {
            char* file_path = NULL;
            char* vm_file_path = NULL;

            //Allocating memory for the file path
            size_t file_path_len = strlen(name_buf) + strlen(de->d_name) + 2; // +2 for '/' and null terminator

            file_path = malloc(file_path_len);
            if (!file_path) {
                fprintf(stderr, "Error: Could not allocate memory for file path\n");
                return 0;
            }

            snprintf(file_path, file_path_len, "%s/%s", name_buf, de->d_name);

            //Allocating memory for the vm file path
            size_t vm_file_path_len = strlen(name_buf) + strlen(de->d_name) + 4; // +4 for ".vm", '/' and null terminator
            vm_file_path = malloc(vm_file_path_len);
            if (!vm_file_path) {
                fprintf(stderr, "Error: Could not allocate memory for vm file path\n");
                free(file_path);
                return 0;
            }

            snprintf(vm_file_path, vm_file_path_len, "%s/%s.vm", name_buf, remove_ext(de->d_name));
            
            strcpy(state->jack_files[state->num_of_files], file_path);
            strcpy(state->jack_vm_files[state->num_of_files], vm_file_path);

            // Free the dyanmically allocated memory
            free(file_path);
            free(vm_file_path);
        }
    }

    closedir(dir);
    return 1;
}
