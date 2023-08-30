# JACK Compiler

A jack compiler following traditional compiler design.

## Features
___
### Lexer /Tokenizer

State machine based tokenizing 

(*example state*)
```c
[COMMENT_START] = {  
        [C_white] = START,  
        [C_newline] = START,  
        [C_alpha] = IN_ID,  
        [C_digit] = IN_NUM,  
        [C_double_quote] = IN_STRING,  
        [C_slash] = IN_COMMENT_SINGLE,  
        [C_star] = IN_COMMENT_MULTI,  
        [C_symbol] = IN_SYMBOL,  
        [C_other] = IN_ERROR,  
        [C_eof] = IN_ERROR,  
}
```

JACK language is ASCII bounded, so anything greater than the standard ASCII range can be treated as an error state. Means our state machine is relatively small. (location: `src/lexer/refac_lexer.c`)
### AST Generation

Location : `src/parser/refac_parser.c`

A recursive descent parser, since JACK is an LL(1) language.  Implementation followed the grammar representation given [here](https://www.cs.huji.ac.il/course/2002/nand2tet/oldsite/docs/ch_9_compiler_I.pdf).  The range of ASTNode's are defined in `src/include/ast.h`

(*parser function for a subroutine body*)

```c
ASTNode* parse_subroutine_body(Parser* parser) {  
  
    ASTNode* node = init_ast_node(NODE_SUBROUTINE_BODY, parser->arena);  
    node->filename = arena_strdup(parser->arena, parser->currentToken->filename);  
    node->line = parser->currentToken->line;  
    node->byte_offset = *(size_t*)vector_get(parser->line_offsets, node->line  - 1);  
  
    log_message(LOG_LEVEL_DEBUG,ERROR_NONE, "Parsing subroutine body. Current Token : %s, Line : %d\n",  
        token_type_to_string(parser->currentToken->type), parser->currentToken->line);  
  
    expect_and_consume(parser, TOKEN_TYPE_OPEN_BRACE);  
  
    // Parse any variable declarations  
    while (parser->currentToken->type == TOKEN_TYPE_VAR) {  
        ASTNode* varDec = parse_var_dec(parser);  
        vector_push(node->data.subroutineBody->varDecs, varDec);  
    }  
    // Parse the statements  
    node->data.subroutineBody->statements = parse_statements(parser);  
  
    expect_and_consume(parser, TOKEN_TYPE_CLOSE_BRACE);  
  
    return node;  
}
```

### Code Generation

A standardised visitor pattern was followed for all the following phases, making our AST traversal relatively phase agnostic.  This was enabled by a simple jump table per phase, dependent on the `nodeType` field of `ASTNode`.
#### Symbol Table Building

Locations : `src/symbol/symbol.c`, `src/ast/ast.c`

We represent different scope levels via our symbol table (class fields , local variables etc.). Types are supported and are either JACK stdlib types or user-defined. types. Type-checking is performed in the analysis phase.  The standard library is also supported, it is added to the global symbol table in `src/symbol/symbol.c`

(*handler function for subroutine declaration - during the build phase*)
```c
void build_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {  
    SymbolTable* subroutWe type-checking ineTable;  
    Symbol* subSymbol;  
  
    switch(node->data.subroutineDec->subroutineType) {  
        case CONSTRUCTOR:  
            subroutineTable = create_table(SCOPE_CONSTRUCTOR, visitor->currentTable, visitor->arena);  
            subSymbol = symbol_table_add(visitor->currentTable, node->data.subroutineDec->subroutineName,  
                node->data.subroutineDec->returnType, KIND_CONSTRUCTOR);  
            break;  
        case METHOD:  
            subroutineTable = create_table(SCOPE_METHOD, visitor->currentTable, visitor->arena);  
            subSymbol =symbol_table_add(visitor->currentTable, node->data.subroutineDec->subroutineName,  
                node->data.subroutineDec->returnType, KIND_METHOD);  
            break;  
        case FUNCTION:  
            subroutineTable = create_table(SCOPE_FUNCTION, visitor->currentTable, visitor->arena);  
            subSymbol = symbol_table_add(visitor->currentTable, node->data.subroutineDec->subroutineName,  
                node->data.subroutineDec->returnType, KIND_FUNCTION);  
            break;  
        default:  
            log_error_with_offset(ERROR_PHASE_SEMANTIC, ERROR_SEMANTIC_INVALID_SUBROUTINE, node->filename, node->line,  
                                  node->byte_offset, "['%s'] : Invalid subroutine type", __func__);  
            exit(EXIT_FAILURE);  
    }  
    subSymbol->childTable = subroutineTable;  
    push_table(visitor, subroutineTable);  
    ast_node_accept(visitor, node->data.subroutineDec->parameters);  
    ast_node_accept(visitor, node->data.subroutineDec->body);  
    pop_table(visitor);  
}
```
#### Analysis

Locations : same as before.

We support type-checking, type folding (some larger expressions become very nested, ASTNode-wise) and propagation, array semantics, variable scoping and declarations, control flow semantics.

(*handler function for subroutine declaration - analysis phase*)

```c
void analyze_subroutine_dec_node(ASTVisitor* visitor, ASTNode* node) {  
    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable,  
                                            node->data.subroutineDec->subroutineName, LOOKUP_LOCAL);  
    if(!subSymbol || (subSymbol->kind != KIND_METHOD && subSymbol->kind != KIND_CONSTRUCTOR  
            && subSymbol->kind != KIND_FUNCTION)) {  
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_KIND , node->filename, node->line,  
                              node->byte_offset, "['%s'] : Undefined subroutine > '%s'", __func__, node->data.subroutineDec->subroutineName );  
    }  
    push_table(visitor, subSymbol->childTable);  
    ast_node_accept(visitor, node->data.subroutineDec->parameters);  
    ast_node_accept(visitor, node->data.subroutineDec->body);  
    pop_table(visitor);  
}
```

#### Actual Code generation

Locations : same as before + `src/ast/ast_writer.c`

Code generation follows definitions given [here](https://drive.google.com/file/d/1CYOcXKxfAwRHaOERvoyuNKSwdlxMo_e3/view). Writer functions are externalised for coherence, but in general the implementation follows the language specification.

(*handler function for subroutine declaration - generation phase*)

```c
void generate_sub_dec_node(ASTVisitor* visitor, ASTNode* node) {  
  
    Symbol* subSymbol = symbol_table_lookup(visitor->currentTable, node->data.subroutineDec->subroutineName, LOOKUP_LOCAL);  
    if (!subSymbol || (subSymbol->kind != KIND_METHOD && subSymbol->kind != KIND_CONSTRUCTOR && subSymbol->kind != KIND_FUNCTION)) {  
        log_error_with_offset(ERROR_PHASE_SEMANTIC,ERROR_SEMANTIC_INVALID_KIND , node->filename, node->line,  
                              node->byte_offset, "['%s'] : Undefined subroutine > '%s'", __func__, node->data.subroutineDec->subroutineName );  
        return;  
    }  
    char* functionLabel = arena_sprintf(visitor->arena, "%s.%s", visitor->currentClassName, node->data.subroutineDec->subroutineNamewhen);  
    vector localSymbols = get_symbols_of_kind(subSymbol->childTable, KIND_VAR);  
    int numLocals = vector_size(localSymbols);  
    write_function(visitor->vmFile, functionLabel, numLocals);  
  
    if (node->data.subroutineDec->subroutineType == CONSTRUCTOR) {  
        vector fieldSymbols = get_symbols_of_kind(subSymbol->table, KIND_FIELD);  
        int numFields = vector_size(fieldSymbols);  
        write_push(visitor->vmFile, SEG_CONST, numFields);  
        write_call(visitor->vmFile, "Memory.alloc", 1);  
        write_pop(visitor->vmFile, SEG_POINTER, 0);  // set the `this` pointer  
    }  
  
    if (node->data.subroutineDec->subroutineType == METHOD) {  
        write_push(visitor->vmFile, SEG_ARG, 0);  
        write_pop(visitor->vmFile, SEG_POINTER, 0);  // set the `this` pointer  
    }  
  
    // manage scopes  
    push_table(visitor, subSymbol->childTable);  
    ast_node_accept(visitor, node->data.subroutineDec->body);  
    pop_table(visitor);  
}
```
### Error Handling 

Due to sometimes limited access to a debugger, a pretty print logger/error handler was implemented.
Errors are reported like this + are logged in the log file. Hints are statically defined bc lazy but could extend via context aware reporting.

```
[Warning][Semantic:Semantic Invalid Expression]: In file 'Ball.jack' , line 109:
	['analyze_subroutine_call_node'] : Subroutine > 'hie', has not been declared yet 
source : do hie();
hint : Check the expression for correctness.
path: ../compiler/src/jack_files/Pong/Ball.jack
[Warning][Semantic:Semantic Undeclared Symbol]: In file 'PongGame.jack' , line 27:
	['analyze_let_statement_node'] : This variable is undeclared > 'bas'
source : let bas = Bat.new(230, 229, batWidth, 7);
hint : Declare the symbol before using it.
path: ../compiler/src/jack_files/Pong/PongGame.jack
[Warning][Semantic:Semantic Invalid Expression]: In file 'PongGame.jack' , line 45:
	['analyze_subroutine_call_node'] : Subroutine > 'dispos', has not been declared yet 
source : do ball.dispos();
hint : Check the expression for correctness.
path: ../compiler/src/jack_files/Pong/PongGame.jack
========== Error Summary ==========
Total Errors: 3
Total Warnings: 3
Compilation completed with errors and/or warnings.
===================================
```

Successful compilation looks like this 

```
========== Error Summary ==========
Total Errors: 0
Total Warnings: 0
Compilation was successful.
===================================
```

### Arena Allocator

Location: `src/util/arena.c`

To make memory management slightly easier, a hand rolled memory arena was used. It is a lazy allocator, reserving pages upfront, but commits smaller sections upon request.  It aligns allocations to the system's word size (a micro-optimisation for cache performance given that JACK programs are relatively small but good practice). Arenas are used as vague lifetime specifiers in the application, see `src/compiler/refac_compiler.c`, but its usage needs improvement/(automation?), it's rather clunky at the moment.
## Extensions/ Improvements

 - Context aware error handling
 - More graceful error handling
 - Better integration of memory arenas - see Rust lifetime specifiers 
 - Different compile targets - (with not too much work, you could make JACK compile to C, although JACK stdlib functionality may be hard to replicate)