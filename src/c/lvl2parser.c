#define REQUIRE_LVL0_PARSER
#include <lvl1parser.h>
#undef REQUIRE_LVL0_PARSER
#include <lvl2parser.h>
#include <stack.h>

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
    char* out_val = parse_string(reader, &exc);
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

void free_value(value_t* val) {
    switch (*val) {
    case VALUE_CHAR:
    case VALUE_FLOATING:
    case VALUE_INTEGER:
        break;
    case VALUE_STRING:
        free(((value_string_t*) val)->value);
        break;
    case VALUE_TUPLE:
        if (((value_tuple_t*) val)->value == NULL) break;
        for (size_t i = 0; i < ((value_tuple_t*) val)->items; i++) {
            free_value(&((value_tuple_t*) val)->value[i]);
        }
        free(((value_tuple_t*) val)->value);
        break;
    case VALUE_SCALAR_INITIALIZER:
        if (((value_scalar_initializer_t*) val)->value == NULL) break;
        for (size_t i = 0; i < ((value_scalar_initializer_t*) val)->items; i++) {
            free_value(&((value_scalar_initializer_t*) val)->value[i]);
        }
        free(((value_scalar_initializer_t*) val)->value);
        break;
    }
    free(val);
}

static void free_item_stack(stack_t stack) {
    value_t* val;
    while ((val = pop_ptr(stack))) {
        free_value(val);
    }
    destroy_stack(stack);
}

value_scalar_initializer_t* parse_scalar_initializer(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc;
    if (!parse_specific_char(&r, &exc, '{')) {
        update_exc(excptr, make_exception(exc, 0, "expected a \"{\" for a scalar initilizer"));
        return NULL;
    }
    stack_t stack;
    for (;;) {
        if (parse_specific_char(&r, &exc, '{')) break;
        value_t* val;
        if ((val = parse_value(&r, &exc))) push_ptr(stack, val);
        else {
            free_item_stack(stack);
            update_exc(excptr, make_exception(exc, 1, "expected a value or } here"));
            return NULL;
        }
    }
    value_scalar_initializer_t* out = malloc(sizeof(*out));
    *out = (value_scalar_initializer_t) {
        .type = VALUE_SCALAR_INITIALIZER,
        .items = stack->bsize / sizeof(value_scalar_initializer_t*),
        .value = stack_disown(stack)
    };
    *reader = r;
    return out;
}

value_tuple_t* parse_tuple(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc;
    if (!parse_specific_char(&r, &exc, '{')) {
        update_exc(excptr, make_exception(exc, 0, "expected a \"(\" for a tuple"));
        return NULL;
    }
    stack_t stack;
    for (;;) {
        if (parse_specific_char(&r, &exc, '{')) break;
        value_t* val;
        if ((val = parse_value(&r, &exc))) push_ptr(stack, val);
        else {
            free_item_stack(stack);
            update_exc(excptr, make_exception(exc, 1, "expected a value or ) here"));
            return NULL;
        }
    }
    value_tuple_t* out = malloc(sizeof(*out));
    *out = (value_tuple_t) {
        .type = VALUE_TUPLE,
        .items = stack->bsize / sizeof(value_tuple_t*),
        .value = stack_disown(stack)
    };
    *reader = r;
    return out;
}

value_t* parse_value(reader_t* reader, Exception** excptr) {
    Exception* exc;
    value_t* out;
    if ((out = parse_character_value(reader, &exc))) return out;
    else update_exc(excptr, exc);
    if ((out = parse_integer_value(reader, &exc))) return out;
    else update_exc(excptr, exc);
    if ((out = parse_floating_value(reader, &exc))) return out;
    else update_exc(excptr, exc);
    if ((out = parse_string(reader, &exc))) return out;
    else update_exc(excptr, exc);
    if ((out = parse_scalar_initializer(reader, &exc))) return out;
    else update_exc(excptr, exc);
    if ((out = parse_tuple(reader, &exc))) return out;
    else update_exc(excptr, exc);
    return NULL;
}
