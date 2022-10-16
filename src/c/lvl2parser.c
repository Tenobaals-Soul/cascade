#include <lvl1parser.h>
#include <lvl2parser.h>

typedef struct Exception Exception;

value_t* parse_integer_value(reader_t* reader, Exception** excptr) {
    Exception* exc;
    uintmax_t out_val = parse_integer(reader, &exc);
    value_integer_t* out_wrapper = malloc(sizeof(value_integer_t));
    if (exc) {
        if (excptr) *excptr = exc;
        else free_exception(exc);
        return NULL;
    }
    else {
        *excptr = NULL;
        out_wrapper->type = VALUE_INTEGER;
        out_wrapper->value = out_val;
        return &out_wrapper->type;
    }
}

value_t* parse_floating_value(reader_t* reader, Exception** excptr) {
    Exception* exc;
    double out_val = parse_floating(reader, &exc);
    value_floating_t* out_wrapper = malloc(sizeof(*out_wrapper));
    if (exc) {
        if (excptr) *excptr = exc;
        else free_exception(exc);
        return NULL;
    }
    else {
        *excptr = NULL;
        out_wrapper->type = VALUE_FLOATING;
        out_wrapper->value = out_val;
        return &out_wrapper->type;
    }
}

value_t* parse_character_value(reader_t* reader, Exception** excptr) {
    Exception* exc;
    char out_val = parse_character(reader, &exc);
    value_char_t* out_wrapper = malloc(sizeof(*out_wrapper));
    if (exc) {
        if (excptr) *excptr = exc;
        else free_exception(exc);
        return NULL;
    }
    else {
        *excptr = NULL;
        out_wrapper->type = VALUE_CHAR;
        out_wrapper->value = out_val;
        return &out_wrapper->type;
    }
}

value_t* parse_string_value(reader_t* reader, Exception** excptr) {
    Exception* exc;
    char* out_val = parse_character(reader, &exc);
    value_string_t* out_wrapper = malloc(sizeof(*out_wrapper));
    if (exc) {
        if (excptr) *excptr = exc;
        else free_exception(exc);
        return NULL;
    }
    else {
        *excptr = NULL;
        out_wrapper->type = VALUE_STRING;
        out_wrapper->value = out_val;
        return &out_wrapper->type;
    }
}

value_scalar_initializer_t* parse_scalar_initializer(reader_t* reader, Exception** excptr);

value_tuple_t* parse_tuple(reader_t* reader, Exception** excptr);

value_t* parse_value(reader_t* reader, Exception** excptr);
