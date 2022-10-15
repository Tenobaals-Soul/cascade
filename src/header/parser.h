#ifndef INCLUDE_PARSER_H
#define INCLUDE_PARSER_H
#include <parser.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stack.h>
#include <stddef.h>

typedef struct reader_t {
    char* text;
    size_t cur;
} reader_t;

struct Exception;

enum value_type {
    VALUE_INTEGER,
    VALUE_FLOATING,
    VALUE_STRING,
    VALUE_CHAR,
    VALUE_LIST,
    VALUE_NULL
};

typedef enum value_type value_base;

typedef struct value_integer_t {
    value_base type;
    uintmax_t value;
} value_integer_t;

typedef struct value_floating_t {
    value_base type;
    long double value;
} value_floating_t;

typedef struct value_string_t {
    value_base type;
    char* value;
} value_string_t;

typedef struct value_char_t {
    value_base type;
    char value;
} value_char_t;

typedef struct value_list_t {
    value_base type;
    value_base* value;
} value_list_t;

#ifdef TEST
char parse_char(reader_t* reader, struct Exception** excptr);
char parse_specific_char(reader_t* reader, struct Exception** excptr, char expect);
char parse_alpha(reader_t* reader, struct Exception** excptr);
char parse_punct(reader_t* reader, struct Exception** excptr);
char parse_digit(reader_t* reader, struct Exception** excptr);
uintmax_t parse_integer(reader_t* reader, struct Exception** excptr);
double parse_floating(reader_t* reader, struct Exception** excptr);
char* parse_identifier(reader_t* reader, struct Exception** excptr);
char parse_operator_char(reader_t* reader, struct Exception** excptr);
char* parse_operator(reader_t* reader, struct Exception** excptr);
bool parse_keyword(reader_t* reader, struct Exception** excptr, const char* expect);
char* parse_string(reader_t* reader, struct Exception** excptr);
#endif

#endif