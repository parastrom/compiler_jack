#ifndef SYMBOL_H
#define SYMBOL_H
#include "vector.h"
#include "headers/safer.h"

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
    SCOPE_CLASS, 
    SCOPE_FUNCTION,
    SCOPE_METHOD,
    SCOPE_CONSTRUCTOR,
    SUBSCOPE
} Scope;

struct Symbol {
    char *name;
    char *type;
    Kind kind;
    int index;
};
struct SymbolTable {
    vector symbols;
    int counts[KIND_NONE];
    Scope scope;
    SymbolTable *parent;
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

SymbolTable* create_table(Scope scope, SymbolTable *parent);
void destroy_table(SymbolTable *table);
void define(SymbolTable *table, char *name, char *type, Kind kind);
int varCount(SymbolTable *table, Kind kind);
Kind kindOf(SymbolTable *table, char *name);
char* typeOf(SymbolTable *table, char *name);
int indexOf(SymbolTable *table, char *name);
Symbol* symbol_new(const char *name, const char *type, Kind kind);
void symbol_table_add(SymbolTable *table, const char* name, const char* type, Kind kind);
Symbol* symbol_table_lookup(SymbolTable* table, char* name);
void destroy_symbol(Symbol *symbol);

SymbolTable* getParent(SymbolTable *table);

ParameterInfo* create_parameter(const char* name, const char* type);
FunctionInfo* create_function(const char* name, const char* return_type, Kind kind);
void add_function(ClassInfo* class_info, FunctionInfo* function);
void add_parameter(FunctionInfo* function, ParameterInfo* parameter);
vector jack_stdlib_setup();
ClassInfo* math_stdlib_setup();
ClassInfo* string_stdlib_setup();
ClassInfo* array_stdlib_setup();
ClassInfo* output_stdlib_setup();
ClassInfo* screen_stdlib_setup();
ClassInfo* kb_stdlib_setup();
ClassInfo* mem_stdlib_setup();
ClassInfo* sys_stdlib_setup();
void add_stdlib_table(SymbolTable* table, vector jack_os_classes);


#endif // SYMBOL_H