#ifndef INCLUDE_LVL2PARSER_H
#define INCLUDE_LVL2PARSER_H
#include <inttypes.h>
#define TYPES_ONLY
#include <lvl1parser.h>
#undef TYPES_ONLY

enum value_type {
    VALUE_INTEGER,
    VALUE_FLOATING,
    VALUE_STRING,
    VALUE_CHAR,
    VALUE_SCALAR_INITIALIZER,
    VALUE_TUPLE
};

typedef enum value_type value_t;

typedef struct value_integer_s {
    value_t type;
    uintmax_t value;
} value_integer_t;

typedef struct value_floating_s {
    value_t type;
    long double value;
} value_floating_t;

typedef struct value_string_s {
    value_t type;
    char* value;
} value_string_t;

typedef struct value_char_s {
    value_t type;
    char value;
} value_char_t;

typedef struct value_scalar_initializer_s {
    value_t type;
    value_t* value;
    size_t items;
} value_scalar_initializer_t;

typedef struct value_tuple_s {
    value_t type;
    value_t* value;
    size_t items;
} value_tuple_t;

value_scalar_initializer_t* parse_scalar_initializer(reader_t* reader, struct Exception** excptr);
value_tuple_t* parse_tuple(reader_t* reader, struct Exception** excptr);
value_t* parse_value(reader_t* reader, struct Exception** excptr);

#endif