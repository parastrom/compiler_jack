#include "symbol.h"
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

Kind string_to_kind(const char* kind_str) {
    if (strcmp(kind_str, "KIND_FUNCTION") == 0) {
        return KIND_FUNCTION;
    } else if (strcmp(kind_str, "KIND_METHOD") == 0) {
        return KIND_METHOD;
    } else if (strcmp(kind_str, "KIND_CONSTRUCTOR") == 0) {
        return KIND_CONSTRUCTOR;
    }else {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_SEMANTIC_INVALID_KIND, __FILE__, __LINE__,
                            "Invalid string passed to ['%s']", __func__);
        return KIND_NONE;
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
    Symbol* symbol = (Symbol*) arena_alloc(table->arena, sizeof(Symbol));
    symbol->name = arena_alloc(table->arena, strlen(name) + 1);
    strcpy(symbol->name, name);
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
SymbolTable* create_table(Scope scope, SymbolTable* parent, Arena* arena) {
    SymbolTable* table = (SymbolTable*) arena_alloc(arena, sizeof(SymbolTable));
    table->arena = arena;
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
    Type* symbolType  = (Type*) arena_alloc(table->arena, sizeof(Type));
    if(strcmp(type, "int") == 0) {
        symbolType->basicType = TYPE_INT;
        symbolType->userDefinedType = NULL;
    } else if (strcmp(type, "char") == 0) {
        symbolType->basicType = TYPE_CHAR;
        symbolType->userDefinedType = NULL;
    } else if (strcmp(type, "boolean") == 0) {
        symbolType->basicType = TYPE_BOOLEAN;
        symbolType->userDefinedType = NULL;
    } else if (strcmp(type, "String") == 0) {
        symbolType->basicType = TYPE_STRING;
        symbolType->userDefinedType = NULL;
    } else if (strcmp(type, "void") == 0 ){
        symbolType->basicType = TYPE_VOID;
        symbolType->userDefinedType = NULL;
    } else {
        symbolType->basicType = TYPE_USER_DEFINED;
        symbolType->userDefinedType = strdup(type);
    }

    Symbol* symbol = symbol_new(name, symbolType, kind, table);
    symbol->index = table->counts[kind];
    table->counts[kind]++;
    vector_push(table->symbols, symbol);
    return symbol;
}

SymbolTable* add_child_table(SymbolTable* parent, Scope scope) {
    SymbolTable* child = create_table(scope, parent, parent->arena);
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

    if (!table) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_NULL_POINTER, __FILE__, __LINE__,
                            "['%s] : SymbolTable pointer was null", __func__);
    }

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

    if (depth == LOOKUP_GLOBAL && !table->parent) { // Global table
        for (int i = 0; i < vector_size(table->children); ++i) {
            SymbolTable* childTable = (SymbolTable*)vector_get(table->children, i);
            Symbol* childSymbol = symbol_table_lookup(childTable, name, LOOKUP_LOCAL);
            if (childSymbol) {
                return childSymbol;
            }
        }
    }

    return NULL;
}

const char* type_to_str(Type* type) {
    switch (type->basicType) {
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
            return type->userDefinedType;
        default:
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_INVALID_INPUT, __FILE__, __LINE__,
                            "Invalid type passed to ['%s']", __func__);
            return NULL;
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
    vector_destroy(table->symbols);
}

FunctionInfo* parse_function_from_json(cJSON* function_json, Arena* arena) {
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

vector parse_jack_stdlib_from_json(const char* json_content, Arena* arena) {
    cJSON *root = cJSON_Parse(json_content);
    if (root == NULL) {
        log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_JSON_PARSING, __FILE__, __LINE__,
                            "['%s'] : %s", __func__, cJSON_GetErrorPtr());
        exit(EXIT_FAILURE);
    }

    vector std_classes = vector_create();

    // Iterate over each class in the JSON
    cJSON *class_item = NULL;
    cJSON_ArrayForEach(class_item, root) {
        ClassInfo *class_info = safer_malloc(sizeof(ClassInfo));


        cJSON *class_name = cJSON_GetObjectItem(class_item, "name");
        if (class_name == NULL || class_name->type != cJSON_String) {
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_JSON_STRUCTURE, __FILE__, __LINE__,
                            "['%s'] : Expected 'name' field in class object ", __func__);
            exit(EXIT_FAILURE);
        }
        class_info->name = strdup(class_name->valuestring);
        class_info->functions = vector_create();

        cJSON *functions_array = cJSON_GetObjectItem(class_item, "functions");
        cJSON *function_item = NULL;
        cJSON_ArrayForEach(function_item, functions_array) {
            FunctionInfo* function_info = parse_function_from_json(function_item, arena);
            vector_push(class_info->functions, function_info);
        }

        cJSON *methods_array = cJSON_GetObjectItem(class_item, "methods");
        cJSON *method_item = NULL;
        cJSON_ArrayForEach(method_item, methods_array) {
            FunctionInfo* method_info = parse_function_from_json(method_item, arena);
            vector_push(class_info->functions, method_info);
        }

        vector_push(std_classes, class_info);
    }

    cJSON_Delete(root);

    return std_classes;
}


SymbolTable* create_table_for_func(Kind kind, SymbolTable* parent_table) {
    switch(kind) {
        case KIND_CONSTRUCTOR:
            return create_table(SCOPE_CONSTRUCTOR, parent_table, parent_table->arena);
        case KIND_METHOD:
            return create_table(SCOPE_METHOD, parent_table, parent_table->arena);
        case KIND_FUNCTION:
            return create_table(SCOPE_FUNCTION, parent_table, parent_table->arena);
        default:
            log_error_no_offset(ERROR_PHASE_INTERNAL, ERROR_SEMANTIC_INVALID_SCOPE, __FILE__, __LINE__,
                            "['%s'] : Invalid function type in stdlib.json ", __func__);
            return NULL;
    }
}



void add_stdlib_table(SymbolTable* global_table, vector jack_os_classes) {
    for (int i = 0; i < vector_size(jack_os_classes); i++) {
        ClassInfo* classInfo = vector_get(jack_os_classes, i);


        Symbol* classSymbol = symbol_table_add(global_table, classInfo->name, classInfo->name, KIND_CLASS);
        SymbolTable* childTable = add_child_table(global_table, SCOPE_CLASS);
        classSymbol->childTable = childTable;

        for (int j = 0; j < vector_size(classInfo->functions); j++) {
            FunctionInfo* funcInfo = vector_get(classInfo->functions, j);


            Symbol* funcSymbol =  symbol_table_add(childTable, funcInfo->name, funcInfo->return_type, funcInfo->kind);
            SymbolTable* funcTable = create_table_for_func(funcInfo->kind, childTable);
            funcSymbol->childTable = funcTable;

            for (int k = 0; k < vector_size(funcInfo->parameters); k++) {
                ParameterInfo* param_info = vector_get(funcInfo->parameters, k);
                symbol_table_add(funcTable, param_info->name, param_info->type, KIND_ARG);
            }
        }
    }
}





