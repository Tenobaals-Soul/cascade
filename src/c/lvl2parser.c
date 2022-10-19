#define REQUIRE_LVL0_PARSER
#include <lvl1parser.h>
#undef REQUIRE_LVL0_PARSER
#include <lvl2parser.h>
#include <stack.h>

typedef struct Exception Exception;

void skip_whitespace(reader_t* reader);

value_t* parse_number_value(reader_t* reader, Exception** excptr) {
    Exception* exc = NULL;
    struct number out_val = parse_number(reader, &exc);
    if (out_val.type == NNONE) {
        update_exc(excptr, exc);
        return NULL;
    }
    if (out_val.type == NINTEGER) {
        value_integer_t* out_wrapper = malloc(sizeof(value_integer_t));
        update_exc(excptr, NULL);
        out_wrapper->type = VALUE_INTEGER;
        out_wrapper->value = out_val.i;
        return &out_wrapper->type;
    }
    else {
        value_floating_t* out_wrapper = malloc(sizeof(value_floating_t));
        update_exc(excptr, NULL);
        out_wrapper->type = VALUE_FLOATING;
        out_wrapper->value = out_val.f;
        return &out_wrapper->type;
    }
}

value_t* parse_character_value(reader_t* reader, Exception** excptr) {
    Exception* exc = NULL;
    char out_val = parse_character(reader, &exc);
    value_char_t* out_wrapper = calloc(1, sizeof(*out_wrapper));
    if (exc) {
        update_exc(excptr, exc);
        return NULL;
    }
    else {
        update_exc(excptr, NULL);
        out_wrapper->type = VALUE_CHAR;
        out_wrapper->value = out_val;
        return &out_wrapper->type;
    }
}

value_t* parse_string_value(reader_t* reader, Exception** excptr) {
    Exception* exc = NULL;
    char* out_val = parse_string(reader, &exc);
    value_string_t* out_wrapper = calloc(1, sizeof(*out_wrapper));
    if (exc) {
        update_exc(excptr, exc);
        return NULL;
    }
    else {
        update_exc(excptr, NULL);
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
            free_value(((value_tuple_t*) val)->value[i]);
        }
        free(((value_tuple_t*) val)->value);
        break;
    case VALUE_SCALAR_INITIALIZER:
        if (((value_scalar_initializer_t*) val)->value == NULL) break;
        for (size_t i = 0; i < ((value_scalar_initializer_t*) val)->items; i++) {
            free_value(((value_scalar_initializer_t*) val)->value[i]);
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

static value_t** parse_value_list(reader_t* reader, Exception** excptr, char start, char end, size_t* len) {
    reader_t r = *reader;
    Exception* exc = NULL;
    if (!parse_specific_char(&r, &exc, start)) {
        update_exc(excptr, make_exception(exc, 0, r, "expected a \'%c\'", start));
        return NULL;
    }
    stack_t stack;
    init_stack(stack);
    value_t* val;
    skip_whitespace(&r);
    if (parse_specific_char(&r, &exc, end)) goto skipall;
    if ((val = parse_value(&r, &exc))) {
        push_ptr(stack, val);
    }
    else {
        free_item_stack(stack);
        update_exc(excptr, make_exception(exc, 1, r, "expected a value here"));
        return NULL;
    }
    for (;;) {
        skip_whitespace(&r);
        if (parse_specific_char(&r, &exc, end)) break;
        if (parse_specific_char(&r, &exc, ',')) {
            skip_whitespace(&r);
            if ((val = parse_value(&r, &exc))) {
                push_ptr(stack, val);
            }
            else {
                free_item_stack(stack);
                update_exc(excptr, make_exception(exc, 1, r, "expected a value here"));
                return NULL;
            }
        }
        else {
            free_item_stack(stack);
            update_exc(excptr, make_exception(exc, 1, r, "expected a , or %c here", end));
            return NULL;
        }
    }
skipall:
    *len = stack->bsize / sizeof(value_t*);
    return (value_t**) stack_disown(stack);
}

value_t* parse_scalar_initializer(reader_t* reader, Exception** excptr) {
    size_t len;
    value_t** values = parse_value_list(reader, excptr, '{', '}', &len);
    if (values == NULL) return NULL;
    value_scalar_initializer_t* sinit = malloc(sizeof(*sinit));
    sinit->type = VALUE_SCALAR_INITIALIZER;
    sinit->items = len;
    sinit->value = values;
    return (value_t*) sinit;
}

value_t* parse_tuple(reader_t* reader, Exception** excptr) {
    size_t len;
    value_t** values = parse_value_list(reader, excptr, '(', ')', &len);
    if (values == NULL) return NULL;
    value_tuple_t* tuple = malloc(sizeof(*tuple));
    tuple->type = VALUE_TUPLE;
    tuple->items = len;
    tuple->value = values;
    return (value_t*) tuple;
}

value_t* parse_value(reader_t* reader, Exception** excptr) {
    Exception* temp_exc = NULL;
    Exception* exc = NULL;
    value_t* out;
    if ((out = parse_character_value(reader, &exc))) {
        if (temp_exc) free_exception(temp_exc);
        return out;
    }
    else update_exc(&temp_exc, exc);
    exc = NULL;
    if ((out = parse_number_value(reader, &exc))) {
        if (temp_exc) free_exception(temp_exc);
        return out;
    }
    else update_exc(&temp_exc, exc);
    exc = NULL;
    if ((out = parse_string_value(reader, &exc))) {
        if (temp_exc) free_exception(temp_exc);
        return out;
    }
    else update_exc(&temp_exc, exc);
    exc = NULL;
    if ((out = parse_scalar_initializer(reader, &exc))) {
        if (temp_exc) free_exception(temp_exc);
        return out;
    }
    else update_exc(&temp_exc, exc);
    exc = NULL;
    if ((out = parse_tuple(reader, &exc))) {
        if (temp_exc) free_exception(temp_exc);
        return out;
    }
    else update_exc(&temp_exc, exc);
    if(excptr) *excptr = temp_exc;
    else free_exception(temp_exc);
    return NULL;
}

void skip_whitespace(reader_t* reader) {
    reader_t r = *reader;
    for (;;) {
        switch (parse_char(&r, NULL)) {
        case ' ':
            break;
        case '\n':
            r.loff = r.cur;
            r.line++;
            break;
        case 0:
            *reader = r;
            return;
        default:
            r.cur--;
            *reader = r;
            return;
        }
    }
}

/*
value_t* parse_expression(reader_t* reader, Exception* excptr) {
    stack_t vastack;
    stack_t opstack;
}
*/