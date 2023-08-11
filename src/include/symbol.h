#ifndef SYMBOL_H
#define SYMBOL_H
#include "vector.h"
#include "safer.h"
#include "cJSON.h"

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;
typedef struct ParameterInfo ParameterInfo;
typedef struct FunctionInfo FunctionInfo;
typedef struct ClassInfo ClassInfo;

typedef enum {
    KIND_STATIC,
    KIND_FIELD,
    KIND_ARG,
    KIND_VAR,
    KIND_CLASS,
    KIND_CONSTRUCTOR,
    KIND_FUNCTION,
    KIND_METHOD,
    KIND_NONE
} Kind;

typedef enum {
    SCOPE_GLOBAL,
    SCOPE_CLASS, 
    SCOPE_FUNCTION,
    SCOPE_METHOD,
    SCOPE_CONSTRUCTOR,
    SUBSCOPE
} Scope;

typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_BOOLEAN,
    TYPE_STRING,
    TYPE_NULL,
    TYPE_VOID,
    TYPE_USER_DEFINED
} BasicType;

typedef struct Type {
    BasicType basicType;
    char* userDefinedType;  // NULL unless basicType == TYPE_USER_DEFINED
} Type;

typedef enum {
    LOOKUP_LOCAL,
    LOOKUP_CLASS,
    LOOKUP_GLOBAL
} Depth;

struct Symbol {
    char *name;
    Type* type;
    Kind kind;
    int index;
    SymbolTable* table;
    SymbolTable* childTable; // Only assigned/relevant when kind == KIND_CLASS
};

struct SymbolTable {
    vector symbols;
    int counts[KIND_NONE];
    Scope scope;
    vector children;
    SymbolTable *parent;
    Arena* arena;
};

struct ParameterInfo {
    char* name;
    char* type;
};

struct FunctionInfo {
    char* name;
    char* return_type;
    vector parameters;
    Kind kind;
};

struct ClassInfo {
    char* name;
    vector functions;
};

SymbolTable* create_table(Scope scope, SymbolTable *parent, Arena* arena);
void destroy_table(SymbolTable *table);
Symbol* symbol_new(const char *name,  Type* type, Kind kind, SymbolTable* table);
Symbol* symbol_table_add(SymbolTable *table, const char* name, const char* type, Kind kind);
Symbol* symbol_table_lookup(SymbolTable* table, char* name, Depth depth);
vector get_symbols_of_kind(SymbolTable* table, Kind kind);
const char* type_to_str(Type type);
void destroy_symbol(Symbol *symbol);
SymbolTable* create_and_link_table(Scope scope, SymbolTable* parent);
SymbolTable* getParent(SymbolTable *table);
SymbolTable* add_child_table(SymbolTable* parent, Scope scope);

vector parse_jack_stdlib_from_json(const char* json_content, Arena* arena);
void add_stdlib_table(SymbolTable* global_table, vector jack_os_classes);


#endif // SYMBOL_H