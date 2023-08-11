#include <stdlib.h>
#include <string.h>
#include "symbol.h"

Kind string_to_kind(const char* kind_str) {
    if (strcmp(kind_str, "KIND_FUNCTION") == 0) {
        return KIND_FUNCTION;
    } else if (strcmp(kind_str, "KIND_METHOD") == 0) {
        return KIND_METHOD;
    } else if (strcmp(kind_str, "KIND_CONSTRUCTOR") == 0) {
        return KIND_CONSTRUCTOR;
    }else {
        log_error(ERROR_SEMANTIC_INVALID_KIND, __FILE__, __LINE__, "Invalid kind string");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Create a symbol object
 * 
 * @param name 
 * @param type 
 * @param kind 
 * @return Symbol* 
 */
Symbol* symbol_new(const char* name, Type* type, Kind kind, SymbolTable* table) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->kind = kind;
    symbol->table = table;
    return symbol;
}

void symbol_free(Symbol* symbol) {
    free(symbol->name);
    if (symbol->type->basicType == TYPE_USER_DEFINED) {
        free(symbol->type->userDefinedType);
    }
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
    table->children = vector_create();


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
Symbol* symbol_table_add(SymbolTable* table, const char* name, const char* type, Kind kind) {
    Type symbolType;
    if(strcmp(type, "int") == 0) {
        symbolType.basicType = TYPE_INT;
        symbolType.userDefinedType = NULL;
    } else if (strcmp(type, "char") == 0) {
        symbolType.basicType = TYPE_CHAR;
        symbolType.userDefinedType = NULL;
    } else if (strcmp(type, "boolean") == 0) {
        symbolType.basicType = TYPE_BOOLEAN;
        symbolType.userDefinedType = NULL;
    } else if (strcmp(type, "String") == 0) {
        symbolType.basicType = TYPE_STRING;
        symbolType.userDefinedType = NULL;
    } else if (strcmp(type, "void") == 0 ){
        symbolType.basicType = TYPE_VOID;
        symbolType.userDefinedType = NULL;
    } else {
        symbolType.basicType = TYPE_USER_DEFINED;
        symbolType.userDefinedType = strdup(type);
    }

    Symbol* symbol = symbol_new(name, &symbolType, kind, table);
    symbol->index = table->counts[kind];
    table->counts[kind]++;
    vector_push(table->symbols, symbol);
    return symbol;
}

SymbolTable* add_child_table(SymbolTable* parent, Scope scope) {
    SymbolTable* child = create_table(scope, parent);
    vector_push(parent->children, child);
    return child;
}


/**
 * @brief Returns the number of variables of the given kind.
 *
 * @param table
 * @param name
 * @return Symbol*
 */

Symbol* symbol_table_lookup(SymbolTable* table, char* name, Depth depth) {
    // Current table lookup
    for (int i = 0; i < vector_size(table->symbols); ++i) {
        Symbol* symbol = (Symbol*)vector_get(table->symbols, i);
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
    }

    if (depth == LOOKUP_LOCAL) {
        return NULL;
    }

    if (depth == LOOKUP_CLASS && (table->scope == SCOPE_METHOD || table->scope == SCOPE_FUNCTION || table->scope == SCOPE_CONSTRUCTOR)) {
        // Look only in the parent (class scope)
        if (table->parent) {
            return symbol_table_lookup(table->parent, name, LOOKUP_CLASS);
        }
        return NULL;
    }

    // Parent table lookup
    if (table->parent != NULL) {
        return symbol_table_lookup(table->parent, name, LOOKUP_GLOBAL);
    }

    // Sibling lookup (other children of parent)
    if (depth == LOOKUP_GLOBAL && table->parent && table->parent->children) {
        for (int i = 0; i < vector_size(table->parent->children); ++i) {
            SymbolTable* siblingTable = (SymbolTable*)vector_get(table->parent->children, i);
            if (siblingTable != table) {
                Symbol* siblingSymbol = symbol_table_lookup(siblingTable, name, LOOKUP_GLOBAL);
                if (siblingSymbol) {
                    return siblingSymbol;
                }
            }
        }
    }

    return NULL;
}

const char* type_to_str(Type type) {
    switch (type.basicType) {
        case TYPE_INT:
            return "int";
        case TYPE_CHAR:
            return "char";
        case TYPE_BOOLEAN:
            return "boolean";
        case TYPE_STRING:
            return "string";
        case TYPE_NULL:
            return "null";
        case TYPE_USER_DEFINED:
            return type.userDefinedType;
        default:
            log_error(ERROR_INVALID_INPUT, __FILE__, __LINE__, "How did we get here");
            exit(EXIT_FAILURE);
    }
}

vector get_symbols_of_kind(SymbolTable* table, Kind kind) {
    vector symbols = vector_create();
    for(int i = 0 ; i < vector_size(table->symbols); i++) {
        Symbol* symbol = vector_get(table->symbols, i);
        if (symbol->kind == kind) {
            vector_push(symbols, symbol);
        }
    }
    return symbols;
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

vector parse_jack_stdlib_from_json(const char* json_content) {
    cJSON *root = cJSON_Parse(json_content);
    if (root == NULL) {
        // Handle parsing error
        log_error(ERROR_JSON_PARSING, __FILE__, __LINE__, cJSON_GetErrorPtr());
        exit(EXIT_FAILURE);
    }

    vector std_classes = vector_create();

    // Iterate over each class in the JSON
    cJSON *class_item = NULL;
    cJSON_ArrayForEach(class_item, root) {
        ClassInfo *class_info = safer_malloc(sizeof(ClassInfo));

        // The class name is the key for this item
        class_info->name = strdup(class_item->string);
        class_info->functions = vector_create();

        cJSON *functions_array = cJSON_GetObjectItem(class_item, "functions");
        cJSON *function_item = NULL;
        cJSON_ArrayForEach(function_item, functions_array) {
            FunctionInfo* function_info = parse_function_from_json(function_item);
            vector_push(class_info->functions, function_info);
        }

        cJSON *methods_array = cJSON_GetObjectItem(class_item, "methods");
        cJSON *method_item = NULL;
        cJSON_ArrayForEach(method_item, methods_array) {
            FunctionInfo* method_info = parse_function_from_json(method_item);
            vector_push(class_info->functions, method_info);
        }

        vector_push(std_classes, class_info);
    }

    cJSON_Delete(root);

    return std_classes;
}

FunctionInfo* parse_function_from_json(cJSON* function_json) {
    FunctionInfo *function_info = safer_malloc(sizeof(FunctionInfo));
    function_info->name = strdup(cJSON_GetObjectItem(function_json, "name")->valuestring);
    function_info->return_type = strdup(cJSON_GetObjectItem(function_json, "return_type")->valuestring);
    function_info->kind = string_to_kind(cJSON_GetObjectItem(function_json, "kind")->valuestring);
    function_info->parameters = vector_create();

    cJSON *params = cJSON_GetObjectItem(function_json, "parameters");
    cJSON *param_item = NULL;
    cJSON_ArrayForEach(param_item, params) {
        ParameterInfo *param_info = safer_malloc(sizeof(ParameterInfo));
        param_info->name = strdup(cJSON_GetObjectItem(param_item, "name")->valuestring);
        param_info->type = strdup(cJSON_GetObjectItem(param_item, "type")->valuestring);
        vector_push(function_info->parameters, param_info);
    }

    return function_info;
}

void add_stdlib_table(SymbolTable* global_table, vector jack_os_classes) {
    for (int i = 0; i < vector_size(jack_os_classes); i++) {
        ClassInfo* class_info = vector_get(jack_os_classes, i);

        // Add class to global table and create a new table for the class
        Symbol* class_symbol = symbol_table_add(global_table, class_info->name, class_info->name, KIND_CLASS);
        SymbolTable* class_table = class_symbol->childTable;

        for (int j = 0; j < vector_size(class_info->functions); j++) {
            FunctionInfo* func_info = vector_get(class_info->functions, j);

            // Add function/method/constructor to class's table and create a new table for it
            (void) symbol_table_add(class_table, func_info->name, func_info->return_type, func_info->kind);
            SymbolTable* func_table = create_table_for_func(func_info->kind, class_table);  // Helper function to determine scope and create table

            for (int k = 0; k < vector_size(func_info->parameters); k++) {
                ParameterInfo* param_info = vector_get(func_info->parameters, k);
                (void) symbol_table_add(func_table, param_info->name, param_info->type, KIND_ARG);
            }
        }
    }
}

SymbolTable* create_table_for_func(Kind kind, SymbolTable* parent_table) {
    switch(kind) {
        case KIND_CONSTRUCTOR:
            return create_table(SCOPE_CONSTRUCTOR, parent_table);
        case KIND_METHOD:
            return create_table(SCOPE_METHOD, parent_table);
        case KIND_FUNCTION:
            return create_table(SCOPE_FUNCTION, parent_table);
        default:
            log_error(ERROR_SEMANTIC_INVALID_SCOPE, __FILE__, __LINE__, "Invalid function kind");
            exit(EXIT_FAILURE);
    }
}



