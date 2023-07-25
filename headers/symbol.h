#ifndef SYMBOL_H
#define SYMBOL_H

#include "vector.h"
#include "headers/safer.h"

typedef struct Symbol Symbol;
typedef struct SymbolTable SymbolTable;

typedef enum {
    KIND_STATIC,
    KIND_FIELD,
    KIND_ARG,
    KIND_VAR,
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

SymbolTable* create_table(Scope scope, SymbolTable *parent);
void destroy_table(SymbolTable *table);
void define(SymbolTable *table, char *name, char *type, Kind kind);
int varCount(SymbolTable *table, Kind kind);
Kind kindOf(SymbolTable *table, char *name);
char* typeOf(SymbolTable *table, char *name);
int indexOf(SymbolTable *table, char *name);
Symbol* symbol_new(char *name, char *type, Kind kind, int index);
void symbol_table_add(SymbolTable *table, const char* name, const char* type, Kind kind);
void destroy_symbol(Symbol *symbol);

SymbolTable* getParent(SymbolTable *table);

#endif // SYMBOL_H