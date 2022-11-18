#ifndef INCLUDE_LVL2PARSER_H
#define INCLUDE_LVL2PARSER_H
#include <inttypes.h>
#include <lvl1parser.h>
#include <list.h>

typedef enum value_type {
    VALUE_INTEGER,
    VALUE_FLOATING,
    VALUE_STRING,
    VALUE_CHAR,
    VALUE_SCALAR_INITIALIZER,
    VALUE_TUPLE,
    VALUE_OPERATION,
    VALUE_VARIABLE
} value_t;

typedef char* name_t;

typedef struct value_integer_t {
    value_t type;
    uintmax_t value;
} value_integer_t;

typedef struct value_floating_t {
    value_t type;
    long double value;
} value_floating_t;

typedef struct value_string_t {
    value_t type;
    char* value;
} value_string_t;

typedef struct value_char_t {
    value_t type;
    char value;
} value_char_t;

typedef struct value_scalar_initializer_t {
    value_t type;
    char* name;
    value_t** value;
    size_t items;
} value_scalar_initializer_t;

typedef struct value_tuple_t {
    value_t type;
    value_t** value;
    size_t items;
} value_tuple_t;

typedef struct value_operation_t {
    value_t type;
    char* operator_name;
    value_t* left;
    value_t* right;
} value_operation_t;

typedef struct value_variable_t {
    value_t type;
    name_t name;
} value_variable_t;

typedef struct type_t {
    char* name;
    size_t generic_len;
    struct type_t** generic_val;
} type_t;

value_t* parse_scalar_initializer(reader_t* reader, struct Exception** excptr);
value_t* parse_tuple(reader_t* reader, struct Exception** excptr);
value_t* parse_value(reader_t* reader, struct Exception** excptr);
name_t parse_name(reader_t* reader, struct Exception** excptr);
#ifdef __GNUC__
__attribute__((__const__))
#endif
int name_len(name_t name);
void name_inner_len(name_t name, int out[name_len(name)]);

/**
 * usage:
 * name_t do_not_change = "Tree\0\EMPTY\0";
 * name_t iterator = do_not_change;
 * while ((segment = name_next(&iterator))) {
 *     // do stuff
 * }
 * // do_not_change == "Tree\0\EMPTY\0";
 * // name_t iterator == "\0";
 */
char* name_next(name_t* name);

value_t* parse_expression(reader_t* reader, struct Exception** excptr);

type_t* parse_type(reader_t* reader, struct Exception** excptr);

typedef enum statement_type {
    EMPTY_STATEMENT,
    ASSIGN_STATEMENT,
    BLOCK_STATEMENT,
    RETURN_STATEMENT,
    IF_STATEMENT,
    FOR_STATEMENT,
    WHILE_STATEMENT,
    DO_WHILE_STATEMENT
} statement_t;

typedef struct empty_statement_t {
    statement_t type;
} empty_statement_t;

extern const empty_statement_t empty_statement_guard;

typedef struct assign_statement_t {
    statement_t type;
    name_t dest;
    value_t* val;
} assign_statement_t;

typedef struct block_statement_t {
    statement_t type;
    statement_t** statements;
    size_t count;
} block_statement_t;

typedef struct return_statement_t {
    statement_t type;
    value_t* val;
} return_statement_t;

typedef struct if_statement_t {
    statement_t type;
    value_t* val;
    statement_t* nstm;
    statement_t* estm;
} if_statement_t;

typedef struct for_statement_t {
    statement_t type;
    list_t stm1;
    list_t val2;
    list_t stm3;
    statement_t* nstm;
} for_statement_t;

typedef struct while_statement_t {
    statement_t type;
    value_t* val;
    statement_t* nstm;
} while_statement_t;

typedef struct do_while_statement_t {
    statement_t type;
    value_t* val;
    statement_t* nstm;
} do_while_statement_t;

statement_t* parse_statement(reader_t* reader, struct Exception** excptr);

#endif