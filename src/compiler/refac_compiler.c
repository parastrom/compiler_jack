// #include "compiler.h"
#include "refac_compiler.h"
#include "arena.h"
#include "ast.h"
#include "logger.h"
#include "vector.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CompilerState *init_compiler() {
  Arena *arena = init_arena(128);
  CompilerState *state = arena_alloc(arena, sizeof(CompilerState));
  state->jack_files = vector_create();
  state->jack_vm_files = vector_create();
  state->arena = arena;
  state->num_of_files = 0;
  state->vm_filename = NULL;
  state->vm_ptr = NULL;
  state->global_table = create_table(SCOPE_GLOBAL, NULL, arena);
  return state;
}

const char *get_file_ext(const char *file_name) {
  const char *dot = strrchr(file_name, '.');
  if (!dot || dot == file_name)
    return "";
  return dot + 1;
}

// Allocates memory for the file name
const char *remove_ext(const char *myStr) {
  if (myStr == NULL) {
    return NULL;
  }

  size_t len = strlen(myStr);
  const char *lastExt = strrchr(myStr, '.');
  if (lastExt == NULL) {
    return myStr;
  }

  size_t extLen = len - (lastExt - myStr);
  size_t newLen = len - extLen;

  char *retStr = safer_malloc(newLen + 1);

  memcpy(retStr, myStr, newLen);
  retStr[newLen] = '\0';

  return retStr;
}

int find_jack_files(const char *dir_name, CompilerState *state) {

  char name_buf[128];
  memset(name_buf, 0, sizeof name_buf);

  // Point directly to the Pong1 subdirectory
  snprintf(name_buf, sizeof(name_buf), "%s/Pong", dir_name);

  DIR *dir = opendir(name_buf);
  if (dir == NULL) {
    perror("Error opening directory");
    log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_OPEN_DIRECTORY, __FILE__, __LINE__,
                            "['%s'] : Could not open directory > '%s'", __func__, name_buf);
    return 0;
  }

  struct dirent *de;
  while ((de = readdir(dir)) != NULL) {
    if (de->d_type == DT_REG && strcmp("jack", get_file_ext(de->d_name)) == 0) {
      char *file_path = NULL;
      char *vm_file_path = NULL;

      size_t file_path_len = strlen(name_buf) + strlen(de->d_name) +
                             2; // +2 for '/' and null terminator
      file_path = malloc(file_path_len);
      if (!file_path) {
          log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for file path", __func__);
        closedir(dir);
        return 0;
      }
      snprintf(file_path, file_path_len, "%s/%s", name_buf, de->d_name);

      size_t vm_file_path_len = strlen(name_buf) + strlen(de->d_name) +
                                4; // +4 for ".vm", '/' and null terminator
      vm_file_path = malloc(vm_file_path_len);
      if (!vm_file_path) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_MEMORY_ALLOCATION, __FILE__, __LINE__,
                            "['%s'] : Failed to allocate memory for vm file path", __func__);
        free(file_path);
        closedir(dir);
        return 0;
      }

      snprintf(file_path, file_path_len, "%s/%s", name_buf, de->d_name);
      vector_push(state->jack_files, file_path); // Push to the vector

      char *no_ext_name = remove_ext(de->d_name);
      snprintf(vm_file_path, vm_file_path_len, "%s/%s.vm", name_buf,
               no_ext_name);
      vector_push(state->jack_vm_files, vm_file_path); // Push to the vector

      free(no_ext_name);
      state->num_of_files++;
    }
  }

  closedir(dir);
  return 1;
}

int compile(CompilerState *state) {

  initialize_eq_classes();
  initialize_logger_arena();

  char buf[256];
  snprintf(buf, sizeof(buf), "%s/stdlib.json", JACK_FILES_DIR);
  const char *stdlib_json = read_file_into_string(buf);
  find_jack_files(JACK_FILES_DIR, state);

  log_message(LOG_LEVEL_INFO, ERROR_NONE, "Finished finding files\n");
  vector jack_os_classes = parse_jack_stdlib_from_json(stdlib_json, state->arena);
  log_message(LOG_LEVEL_INFO, ERROR_NONE, "Finished parsing stdlib_json\n");
  add_stdlib_table(state->global_table, jack_os_classes);

  ASTNode *program_node = init_ast_node(NODE_PROGRAM, state->arena);

  for (int i = 0; i < state->num_of_files; i++) {
    Arena *fileArena = init_arena(16);
    Lexer *lexer = init_lexer(vector_get(state->jack_files, i), fileArena);
    Parser *parser = init_parser(lexer->queue, lexer->line_starts, state->arena);
    ASTNode *class_node = parse_class(parser);
    vector_push(program_node->data.program->classes, class_node);

    destroy_lexer(lexer);
    destroy_arena(fileArena);
    destroy_parser(parser);
  }

  ASTVisitor *visitor = init_ast_visitor(state->arena, BUILD, state->global_table);
  ast_node_accept(visitor, program_node);
  log_message(LOG_LEVEL_INFO, ERROR_NONE, "Finished building\n");
  visitor->phase = ANALYZE;
  ast_node_accept(visitor, program_node);

  if(error_count() == 0) {
      visitor->phase = GENERATE;

      for(int i = 0 ; i < state->num_of_files; ++i) {
          ASTNode* class_node = vector_get(program_node->data.program->classes, i);
          visitor->vmFile =fopen((char*) vector_get(state->jack_vm_files, i), "w");
          if (visitor->vmFile == NULL) {
              log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_FILE_OPEN, __FILE__, __LINE__,"['%s'] : Failed to open/create VM file > '%s'", __func__,(char*) vector_get(state->jack_vm_files, i));
          }
          ast_node_accept(visitor, class_node);
          fclose((visitor->vmFile));
      }
  }

  //

  print_all_errors();
  print_error_summary();

  // clean up
  close_log_file();
  destroy_ast_node(program_node);
  destroy_arena(state->arena);

  return 1;
}
