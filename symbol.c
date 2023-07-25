#include <stdlib.h>
#include <string.h>
#include "headers/symbol.h"

Symbol* symbol_new(char* name, char* type, Kind kind, int index) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = strdup(type);
    symbol->kind = kind;
    symbol->index = index;
    return symbol;
}

void symbol_free(Symbol* symbol) {
    free(symbol->name);
    free(symbol->type);
    free(symbol);
}

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

void destroy_table(SymbolTable* table) {
    for (int i = 0; i < vector_size(table->symbols); i++) {
        Symbol* symbol = vector_get(table->symbols, i);
        symbol_free(symbol);
    }
    vector_destroy(table->symbols);
    free(table);
}