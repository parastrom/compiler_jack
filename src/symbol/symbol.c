#include <stdlib.h>
#include <string.h>
#include "symbol.h"

/**
 * @brief Create a symbol object
 * 
 * @param name 
 * @param type 
 * @param kind 
 * @return Symbol* 
 */
Symbol* symbol_new(const char* name, const char* type, Kind kind) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = strdup(type);
    symbol->kind = kind;
    return symbol;
}

void symbol_free(Symbol* symbol) {
    free(symbol->name);
    free(symbol->type);
    free(symbol);
}


/**
 * @brief Create a table object
 * 
 * @param scope 
 * @param parent 
 * @return SymbolTable* 
 */
SymbolTable* create_table(Scope scope, SymbolTable* parent) {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    table->scope = scope;
    table->parent = parent;
    table->symbols = NULL;
    for (int i = 0; i < KIND_NONE; i++) {
        table->counts[i] = 0;
    }
    table->symbols = vector_create();

    return table;
}


/**
 * @brief Adds a symbol to the symbol table.
 * 
 * @param table 
 * @param name 
 * @param type 
 * @param kind 
 */
void symbol_table_add(SymbolTable* table, const char* name, const char* type, Kind kind) {
    Symbol* symbol = symbol_new(name, type, kind);
    symbol->index = table->counts[kind];
    table->counts[kind]++;
    vector_push(table->symbols, symbol);
}


/**
 * @brief Returns the number of variables of the given kind.
 * 
 * @param table 
 * @param name 
 * @return Symbol* 
 */
Symbol* symbol_table_lookup(SymbolTable* table, char* name) {
    for (int i = 0; i < vector_size(table->symbols); ++i) {
        Symbol* symbol = (Symbol*)vector_get(table->symbols, i);
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
    }
    // If not found in the current table, check the parent table
    if (table->parent != NULL) {
        return symbol_table_lookup(table->parent, name);
    }
    return NULL;
}


/**
 * @brief Destructor for SymbolTable
 * 
 * @param table 
 */
void destroy_table(SymbolTable* table) {
    for (int i = 0; i < vector_size(table->symbols); i++) {
        Symbol* symbol = vector_get(table->symbols, i);
        symbol_free(symbol);
    }
    vector_destroy(table->symbols);
    free(table);
}


/**
 * @brief Create a parameter object
 * 
 * @param name name of the parameter
 * @param type type of the parameter
 * @return ParameterInfo* 
 */
ParameterInfo* create_parameter(const char* name, const char* type) {
    ParameterInfo* parameter = safer_malloc(sizeof(ParameterInfo));
    parameter->name = strdup(name);
    parameter->type = strdup(type);
    return parameter;
}

/**
 * @brief Create a function object
 * 
 * @param name name of the function
 * @param return_type return type of the function
 * @return FunctionInfo* 
 */
FunctionInfo* create_function(const char* name, const char* return_type, Kind kind) {
    FunctionInfo* function = safer_malloc(sizeof(FunctionInfo));
    function->name = strdup(name);
    function->return_type = strdup(return_type);
    function->kind = kind;
    function->parameters = vector_create();
    return function;
}


void add_function(ClassInfo* class_info, FunctionInfo* function) {
    vector_push(class_info->functions, function);
}

void add_parameter(FunctionInfo* function, ParameterInfo* parameter) {
    vector_push(function->parameters, parameter);
}

vector jack_stdlib_setup() {

    vector std_classes = vector_create();
    vector_push(std_classes, math_stdlib_setup());
    vector_push(std_classes, string_stdlib_setup());
    vector_push(std_classes, array_stdlib_setup());
    vector_push(std_classes, output_stdlib_setup());
    vector_push(std_classes, screen_stdlib_setup());
    vector_push(std_classes, kb_stdlib_setup());
    vector_push(std_classes, mem_stdlib_setup());
    vector_push(std_classes, sys_stdlib_setup());

    return std_classes;

}

ClassInfo* math_stdlib_setup() {
    ClassInfo* math_info = safer_malloc(sizeof(ClassInfo));
    math_info->name = strdup("Math");
    math_info->functions = vector_create();

    FunctionInfo* math_abs = create_function("abs", "int", KIND_FUNCTION);
    add_parameter(math_abs, create_parameter("n", "int"));
    add_function(math_info, math_abs);


    FunctionInfo* math_multiply = create_function("multiply", "int", KIND_FUNCTION);
    add_parameter(math_multiply, create_parameter("x", "int"));
    add_parameter(math_multiply, create_parameter("y", "int"));
    add_function(math_info, math_multiply);

    FunctionInfo* math_divide = create_function("divide", "int", KIND_FUNCTION);
    add_parameter(math_divide, create_parameter("x", "int"));
    add_parameter(math_divide, create_parameter("y", "int"));
    add_function(math_info, math_divide);

    FunctionInfo* math_min = create_function("min", "int", KIND_FUNCTION);
    add_parameter(math_min, create_parameter("x", "int"));
    add_parameter(math_min, create_parameter("y", "int"));
    add_function(math_info, math_min);

    FunctionInfo* math_max = create_function("max", "int", KIND_FUNCTION);
    add_parameter(math_max, create_parameter("x", "int"));
    add_parameter(math_max, create_parameter("y", "int"));
    add_function(math_info, math_max);

    FunctionInfo* math_sqrt = create_function("sqrt", "int", KIND_FUNCTION);
    add_parameter(math_sqrt, create_parameter("x", "int"));
    add_function(math_info, math_sqrt);

    return math_info;
}

ClassInfo* string_stdlib_setup() {
    ClassInfo* string_info = safer_malloc(sizeof(ClassInfo));
    string_info->name = strdup("String");
    string_info->functions = vector_create();

    FunctionInfo* string_new_int = create_function("new", "String", KIND_CONSTRUCTOR);
    add_parameter(string_new_int, create_parameter("maxLen", "int"));
    add_function(string_info, string_new_int);

    FunctionInfo* string_append = create_function("dispose", "void", KIND_METHOD);
    add_function(string_info, string_append);

    FunctionInfo* string_length = create_function("length", "int", KIND_METHOD);
    add_function(string_info, string_length);

    FunctionInfo* string_charAt = create_function("charAt", "char", KIND_METHOD);
    add_parameter(string_charAt, create_parameter("i", "int"));
    add_function(string_info, string_charAt);

    FunctionInfo* string_setCharAt = create_function("setCharAt", "void", KIND_METHOD);
    add_parameter(string_setCharAt, create_parameter("i", "int"));
    add_parameter(string_setCharAt, create_parameter("c", "char"));
    add_function(string_info, string_setCharAt);

    FunctionInfo* string_appendChar = create_function("appendChar", "String", KIND_METHOD);
    add_parameter(string_appendChar, create_parameter("c", "char"));
    add_function(string_info, string_appendChar);

    FunctionInfo* string_eraseLastChar = create_function("eraseLastChar", "void", KIND_METHOD);
    add_function(string_info, string_eraseLastChar);

    FunctionInfo* string_intVal = create_function("intValue", "int", KIND_METHOD);
    add_function(string_info, string_intVal);

    FunctionInfo* string_setIntVal = create_function("setInt", "void", KIND_METHOD);
    add_parameter(string_setIntVal, create_parameter("n", "int"));
    add_function(string_info, string_setIntVal);

    FunctionInfo* string_backSpace = create_function("backSpace", "char", KIND_FUNCTION);
    add_function(string_info, string_backSpace);

    FunctionInfo* string_doubleQuote = create_function("doubleQuote", "char", KIND_FUNCTION);
    add_function(string_info, string_doubleQuote);

    FunctionInfo* string_newLine = create_function("newLine", "char", KIND_FUNCTION);
    add_function(string_info, string_newLine);

    return string_info;
}

ClassInfo* array_stdlib_setup() {
    ClassInfo* array_info = safer_malloc(sizeof(ClassInfo));
    array_info->name = strdup("Array");
    array_info->functions = vector_create();

    FunctionInfo* array_new = create_function("new", "Array", KIND_FUNCTION);
    add_parameter(array_new, create_parameter("size", "int"));
    add_function(array_info, array_new);

    FunctionInfo* array_dispose = create_function("dispose", "void", KIND_METHOD);
    add_function(array_info, array_dispose);

    return array_info;
}


ClassInfo* output_stdlib_setup() {
    ClassInfo* output_info = safer_malloc(sizeof(ClassInfo));
    output_info->name = strdup("Output");
    output_info->functions = vector_create();

    FunctionInfo* output_move_cursor = create_function("moveCursor", "void", KIND_FUNCTION);
    add_parameter(output_move_cursor, create_parameter("row", "int"));
    add_parameter(output_move_cursor, create_parameter("col", "int"));
    add_function(output_info, output_move_cursor);

    FunctionInfo* output_print_char = create_function("printChar", "void", KIND_FUNCTION);
    add_parameter(output_print_char, create_parameter("c", "char"));
    add_function(output_info, output_print_char);

    FunctionInfo* output_print_string = create_function("printString", "void", KIND_FUNCTION);
    add_parameter(output_print_string, create_parameter("s", "String"));
    add_function(output_info, output_print_string);

    FunctionInfo* output_print_int = create_function("printInt", "void", KIND_FUNCTION);
    add_parameter(output_print_int, create_parameter("n", "int"));
    add_function(output_info, output_print_int);

    FunctionInfo* output_println = create_function("println", "void", KIND_FUNCTION);
    add_function(output_info, output_println);

    FunctionInfo* output_backspace = create_function("backSpace", "void", KIND_FUNCTION);
    add_function(output_info, output_backspace);

    return output_info;
}

ClassInfo* screen_stdlib_setup() {

    ClassInfo* screen_info = safer_malloc(sizeof(ClassInfo));
    screen_info->name = strdup("Screen");
    screen_info->functions = vector_create();

    FunctionInfo* clear_screen = create_function("clearScreen", "void", KIND_FUNCTION);
    add_function(screen_info, clear_screen);

    FunctionInfo* set_color = create_function("setColor", "void", KIND_FUNCTION);
    add_parameter(set_color, create_parameter("b", "boolean"));
    add_function(screen_info, set_color);

    FunctionInfo* draw_pixel = create_function("drawPixel", "void", KIND_FUNCTION);
    add_parameter(draw_pixel, create_parameter("x", "int"));
    add_parameter(draw_pixel, create_parameter("y", "int"));
    add_function(screen_info, draw_pixel);

    FunctionInfo* draw_line = create_function("drawLine", "void", KIND_FUNCTION);
    add_parameter(draw_line, create_parameter("x1", "int"));
    add_parameter(draw_line, create_parameter("y1", "int"));
    add_parameter(draw_line, create_parameter("x2", "int"));
    add_parameter(draw_line, create_parameter("y2", "int"));
    add_function(screen_info, draw_line);

    FunctionInfo* draw_rect = create_function("drawRect", "void", KIND_FUNCTION);
    add_parameter(draw_rect, create_parameter("x", "int"));
    add_parameter(draw_rect, create_parameter("y", "int"));
    add_parameter(draw_rect, create_parameter("w", "int"));
    add_parameter(draw_rect, create_parameter("h", "int"));
    add_function(screen_info, draw_rect);

    FunctionInfo* draw_circle = create_function("drawCircle", "void", KIND_FUNCTION);
    add_parameter(draw_circle, create_parameter("x", "int"));
    add_parameter(draw_circle, create_parameter("y", "int"));
    add_parameter(draw_circle, create_parameter("r", "int"));
    add_function(screen_info, draw_circle);
}


ClassInfo* kb_stdlib_setup() {

    ClassInfo* keyboard_info = safer_malloc(sizeof(ClassInfo));
    keyboard_info->name = strdup("Keyboard");
    keyboard_info->functions = vector_create();

    FunctionInfo* keyPressed = create_function("keyPressed", "char", KIND_FUNCTION);
    add_function(keyboard_info, keyPressed);

    FunctionInfo* readChar = create_function("readChar", "char", KIND_FUNCTION);
    add_function(keyboard_info, readChar);

    FunctionInfo* readLine = create_function("readLine", "String", KIND_FUNCTION);
    add_parameter(readLine, create_parameter("message", "String"));
    add_function(keyboard_info, readLine);

    FunctionInfo* readInt = create_function("readInt", "int", KIND_FUNCTION);
    add_parameter(readInt, create_parameter("message", "String"));
    add_function(keyboard_info, readInt);

    return keyboard_info;
}

ClassInfo* mem_stdlib_setup() {

    ClassInfo* mem_info = safer_malloc(sizeof(ClassInfo));
    mem_info->name = strdup("Memory");
    mem_info->functions = vector_create();
    
    FunctionInfo* peek = create_function("alloc", "int", KIND_FUNCTION);
    add_parameter(peek, create_parameter("address", "int"));
    add_function(mem_info, peek);

   FunctionInfo* poke = create_function("poke", "void", KIND_FUNCTION);
    add_parameter(poke, create_parameter("address", "int"));
    add_parameter(poke, create_parameter("value", "int"));
    add_function(mem_info, poke);

    FunctionInfo* alloc = create_function("alloc", "Array", KIND_FUNCTION);
    add_parameter(alloc, create_parameter("size", "int"));
    add_function(mem_info, alloc);

    FunctionInfo* deAlloc = create_function("deAlloc", "void", KIND_FUNCTION);
    add_parameter(deAlloc, create_parameter("array", "Array"));
    add_function(mem_info, deAlloc);

    return mem_info;
}

ClassInfo* sys_stdlib_setup() {
    ClassInfo* sys_info = safer_malloc(sizeof(ClassInfo));
    sys_info->name = strdup("Sys");
    sys_info->functions = vector_create();

    FunctionInfo* halt = create_function("halt", "void", KIND_FUNCTION);
    add_function(sys_info, halt);

    FunctionInfo* error = create_function("error", "void", KIND_FUNCTION);
    add_parameter(error, create_parameter("errorCode", "int"));
    add_function(sys_info, error);
    
    FunctionInfo* wait = create_function("wait", "void", KIND_FUNCTION);
    add_parameter(wait, create_parameter("ms", "int"));
    add_function(sys_info, wait);

    return sys_info;

}

void add_stdlib_table(SymbolTable* table, vector jack_os_classes) {
    for (int i = 0; i < vector_size(jack_os_classes); i++) {

        ClassInfo* class_info = vector_get(jack_os_classes, i);
        
        symbol_table_add(table, class_info->name, class_info->name, KIND_CLASS);

        for (int j = 0; j < vector_size(class_info->functions); j++) {
            FunctionInfo* func_info = vector_get(class_info->functions, j);

            symbol_table_add(table, func_info->name, func_info->return_type, func_info->kind);

            SymbolTable* func_table;

            switch(func_info->kind) {
                case KIND_CONSTRUCTOR:
                    func_table = create_table(SCOPE_CONSTRUCTOR, table);
                    break;
                case KIND_METHOD:
                    func_table = create_table(SCOPE_METHOD, table);
                    break;
                case KIND_FUNCTION:
                    func_table =  create_table(SCOPE_FUNCTION, table);
                    break;
                default:
                    log_error(ERROR_SEMANTIC_INVALID_SCOPE, __FILE__, __LINE__, "Invalid function kind");
                    exit(EXIT_FAILURE);
            }

            for (int k = 0; k < vector_size(func_info->parameters); k++) {
                ParameterInfo* param_info = vector_get(func_info->parameters, k);
                symbol_table_add(func_table, param_info->name, param_info->type, KIND_ARG);
            }
        }
    }
}


