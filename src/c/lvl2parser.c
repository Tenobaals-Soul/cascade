#define REQUIRE_LVL0_PARSER
#include <lvl1parser.h>
#undef REQUIRE_LVL0_PARSER
#include <lvl2parser.h>
#include <stack.h>
#include <string.h>
#include <limits.h>
#include <list.h>

#define ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))

typedef struct Exception Exception;

const empty_statement_t empty_statement_guard = { EMPTY_STATEMENT };

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
    if (val == NULL) return;
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
    case VALUE_OPERATION:
        free_value(((value_operation_t*) val)->left);
        free_value(((value_operation_t*) val)->right);
        free(((value_operation_t*) val)->operator_name);
        break;
    case VALUE_VARIABLE:
        free(((value_variable_t*) val)->name);
    }
    free(val);
}

static void free_value_stack(stack_t stack) {
    value_t* val;
    while ((val = pop_ptr(stack))) {
        free_value(val);
    }
    destroy_stack(stack);
}

static value_t** parse_expression_list(reader_t* reader, Exception** excptr, char start, char end, size_t* len) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    if (!parse_specific_char(&r, &exc, start)) {
        update_exc(excptr, make_exception(exc, 0, r, "expected a \'%c\'", start));
        return NULL;
    }
    stack_t stack;
    init_stack(stack);
    value_t* val;
    skip_whitespace(&r);
    if (parse_specific_char(&r, &exc, end)) goto skipall;
    if ((val = parse_expression(&r, &exc))) {
        push_ptr(stack, val);
    }
    else {
        free_value_stack(stack);
        update_exc(excptr, make_exception(exc, 1, r, "expected a value here"));
        return NULL;
    }
    for (;;) {
        skip_whitespace(&r);
        if (parse_specific_char(&r, &exc, end)) break;
        skip_whitespace(&r);
        if (parse_specific_char(&r, &exc, ',')) {
            skip_whitespace(&r);
            if ((val = parse_expression(&r, &exc))) {
                push_ptr(stack, val);
            }
            else {
                free_value_stack(stack);
                update_exc(excptr, make_exception(exc, 1, r, "expected a value here"));
                return NULL;
            }
        }
        else {
            free_value_stack(stack);
            update_exc(excptr, make_exception(exc, 1, r, "expected a , or %c here", end));
            return NULL;
        }
    }
skipall:
    *len = stack->bsize / sizeof(value_t*);
    *reader = r;
    return (value_t**) stack_disown(stack);
}

value_t* parse_scalar_initializer(reader_t* reader, Exception** excptr) {
    size_t len;
    reader_t r = *reader;
    char* name = parse_name(&r, NULL);
    value_t** values = parse_expression_list(&r, excptr, '{', '}', &len);
    if (values == NULL) {
        return NULL;
    }
    value_scalar_initializer_t* sinit = malloc(sizeof(*sinit));
    sinit->name = name;
    sinit->type = VALUE_SCALAR_INITIALIZER;
    sinit->items = len;
    sinit->value = values;
    return (value_t*) sinit;
}

value_t* parse_tuple(reader_t* reader, Exception** excptr) {
    size_t len;
    value_t** values = parse_expression_list(reader, excptr, '(', ')', &len);
    if (values == NULL) return NULL;
    value_tuple_t* tuple = malloc(sizeof(*tuple));
    tuple->type = VALUE_TUPLE;
    tuple->items = len;
    tuple->value = values;
    return (value_t*) tuple;
}

value_t* parse_name_value(reader_t* reader, Exception** excptr) {
    name_t name = parse_name(reader, excptr);
    if (name == NULL) return NULL;
    value_variable_t* val = malloc(sizeof(*val));
    val->type = VALUE_VARIABLE;
    val->name = name;
    return (value_t*) val;
}

value_t* parse_value(reader_t* reader, Exception** excptr) {
    value_t* (*parse_func[])(reader_t*, Exception**) = {
        parse_character_value, parse_number_value, parse_string_value,
        parse_scalar_initializer, parse_name_value, parse_tuple
    };
    Exception* temp_exc = NULL;
    value_t* out;
    for (size_t i = 0; i < sizeof(parse_func) / sizeof(parse_func[0]); i++) {
        Exception* exc = NULL;
        if ((out = parse_func[i](reader, &exc))) {
            if (temp_exc) free_exception(temp_exc);
            return out;
        }
        else update_exc(&temp_exc, exc);
    }
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

enum mode {
    SEARCH_VALUE, SEARCH_OPERATOR
};

struct expression_parsing_locals {
    reader_t r;
    Exception* exc;
    stack_t vastack;
    stack_t opstack;
    enum mode mode;
    size_t values_parsed;
};

bool parse_expression_value(struct expression_parsing_locals* l) {
    value_t* val = parse_value(&l->r, &l->exc);
    if (val) {
        push_ptr(l->vastack, val);
        l->values_parsed++;
    }
    else {
        free_value_stack(l->vastack);
        while (peek_ptr(l->opstack)) {
            free(pop_ptr(l->opstack));
        }
        l->exc = make_exception(l->exc, l->values_parsed, l->r,
            "could not parse expression because a %s was expected",
            l->mode == SEARCH_VALUE ? "value" : "operator");
        return false;
    }
    l->mode = SEARCH_OPERATOR;
    return true;
}

int priority(const char* op) {
    if (op == NULL) return INT_MIN;
    if (strcmp(op, "&&") == 0) return 0;
    if (strcmp(op, "||") == 0) return 0;

    if (strcmp(op, "==") == 0) return 0;
    if (strcmp(op, "!=") == 0) return 0;

    if (strcmp(op, "<=") == 0) return 1;
    if (strcmp(op, ">=") == 0) return 1;
    if (strcmp(op, "<") == 0) return 2;
    if (strcmp(op, ">") == 0) return 2;

    if (strcmp(op, "+") == 0) return 3;
    if (strcmp(op, "-") == 0) return 3;


    if (strcmp(op, "*") == 0) return 4;
    if (strcmp(op, "/") == 0) return 4;

    if (strcmp(op, "|") == 0) return 5;

    if (strcmp(op, "&") == 0) return 6;

    if (strcmp(op, "<<") == 0) return 7;
    if (strcmp(op, ">>") == 0) return 7;
    else return INT_MAX;
}

void pushback(stack_t vastack, stack_t opstack, char* op) {
    while (peek_ptr(opstack) && priority(op) <=
    priority((((value_operation_t*) peek_ptr(opstack))->operator_name))) {
        value_operation_t* opw = pop_ptr(opstack);
        opw->right = pop_ptr(vastack);
        opw->left = pop_ptr(vastack);
        push_ptr(vastack, opw);
    }
}

bool parse_expression_operator(struct expression_parsing_locals* l) {
    char* op = parse_operator(&l->r, &l->exc);
    if (op != NULL) {
        value_operation_t* opwrapper = malloc(sizeof(value_operation_t));
        opwrapper->type = VALUE_OPERATION;
        opwrapper->operator_name = op;
        pushback(l->vastack, l->opstack, op);
        push_ptr(l->opstack, opwrapper);
        l->values_parsed++;
        l->mode = SEARCH_VALUE;
        return true;
    }
    return false;
}

name_t parse_name(reader_t* reader, struct Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    char* segment = parse_identifier(&r, &exc);
    if (segment == NULL) {
        update_exc(excptr, make_exception(exc, 0, r,
            "a name has to start with an identifier"));
        return NULL;
    }
    stack_t name;
    init_stack(name);
    push_str(name, segment);
    for (;;) {
        skip_whitespace(&r);
        if (parse_specific_char(&r, NULL, '.') == 0) break;
        skip_whitespace(&r);
        segment = parse_identifier(&r, &exc);
        if (segment == NULL) {
            update_exc(excptr, make_exception(exc, 1, r,
                "expected a identifier"));
            destroy_stack(name);
            return NULL;
        }
        push_str(name, segment);
    }
    *reader = r;
    return stack_disown(name);
}

#ifdef __GNUC__
__attribute__((__const__))
#endif
int name_len(name_t name) {
    size_t len;
    for (len = 0; name[len] && name[len + 1]; len++);
    return len;
}

void name_inner_len(name_t name, int out[name_len(name)]) {
    int cur = 0;
    for (size_t i = 0; name[i] && name[i + 1]; i++) {
        size_t s = i;
        for (; name[i]; i++);
        out[cur] = i - s;
    }
}

char* name_next(name_t* name) {
    char* current = *name;
    for (; (*name)[0]; (*name)++);
    return current;
}

value_t* parse_expression(reader_t* reader, Exception** excptr) {
    struct expression_parsing_locals l;
    l.r = *reader;
    l.exc = NULL;
    init_stack(l.vastack);
    init_stack(l.opstack);
    l.mode = SEARCH_VALUE;
    l.values_parsed = 0;
    for (;;) {
        skip_whitespace(&l.r);
        if (l.mode == SEARCH_VALUE) {
            if (!parse_expression_value(&l)) {
                update_exc(excptr, l.exc);
                return NULL;
            }
        }
        else {
            if (!parse_expression_operator(&l)) {
                pushback(l.vastack, l.opstack, NULL);
                value_t* val = pop_ptr(l.vastack);
                destroy_stack(l.vastack);
                destroy_stack(l.opstack);
                *reader = l.r;
                return val;
            }
        }
    }
}

void free_type(type_t* type) {
    if (type->generic_val) {
        for (size_t i = 0; i < type->generic_len; i++) {
            free_type(type->generic_val[i]);
        }
        free(type->generic_val);
    }
    free(type);
}

type_t* parse_type(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    char* name = parse_identifier(&r, &exc);
    if (name == NULL) {
        update_exc(excptr, make_exception(exc, 0, r, "expected a type name"));
        return NULL;
    }
    type_t* out = malloc(sizeof(type_t));
    out->name = name;
    out->generic_len = 0;
    list_t generics;
    list_init(generics, sizeof(type_t*));
    skip_whitespace(&r);
    if (parse_specific_char(&r, NULL, '<')) {
        type_t* inner = parse_type(&r, &exc);
        for (;;) {
            if (inner == NULL) {
                update_exc(excptr,make_exception(exc, 1, r,
                    "expected a type"));
                goto cleanup;
            }
            append_ptr(generics, inner);
            skip_whitespace(&r);
            if (parse_specific_char(&r, NULL, ',') == 0) break;
            inner = parse_type(&r, &exc);
        }
        skip_whitespace(&r);
        if (parse_specific_char(&r, NULL, '>') == 0) {
            update_exc(excptr, make_exception(exc, 1, r, "expected >"));
            goto cleanup;
        }
    }
    out->generic_len = generics->wsize;
    out->generic_val = list_disown(generics);
    *reader = r;
    return out;

    cleanup:
    out->generic_len = generics->wsize;
    out->generic_val = list_disown(generics);
    free_type(out);
    return NULL;
}

statement_t* parse_empty_statement(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    if (parse_specific_char(&r, &exc, ';')) {
        *reader = r;
        return &empty_statement_guard;
    }
    update_exc(excptr, make_exception(exc, 0, r,
        "empty statement must be only \";\""));
    return NULL;
}

bool try_parse_typed_assign(reader_t* reader, type_t** type, name_t* name) {
    reader_t r = *reader;
    *type = parse_type(&r, NULL);
    if (*type == NULL)
        return false;
    *name = parse_name(&r, NULL);
    if (*name == NULL) {
        free_type(type);
        return false;
    }
    *reader = r;
    return true;
}

bool try_parse_untyped_assign(reader_t* reader, name_t* name) {
    reader_t r = *reader;
    *name = parse_name(&r, NULL);
    if (*name == NULL)
        return false;
    *reader = r;
    return true;
}

statement_t* parse_assign_statement(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    type_t* type = parse_type(&r, NULL);
    name_t name = parse_name(&r, NULL);
    assign_statement_t stm;
    stm.type = ASSIGN_STATEMENT;
    if (try_parse_typed_assign(&r, &type, &name)) {
        stm.rntype = type;
    }
    else if (try_parse_untyped_assign(&r, &name)) {
        stm.rntype = NULL;
    }
    else {
        update_exc(excptr, make_exception(NULL, 0, r,
            "expected a destination"));
        return NULL;
    }
    skip_whitespace(&r);
    if (!parse_specific_char(&r, &exc, '=')) {
        free_type(type);
        free(name);
        update_exc(excptr, make_exception(exc, 1, r, "excpected a \"=\""));
        return NULL;
    }
    skip_whitespace(&r);
    stm.dest = name;
    stm.val = parse_value(&r, &exc);
    if (stm.val == NULL) {
        free_type(type);
        free(name);
        update_exc(excptr, make_exception(exc, 1, r, "excpected a expression"));
        return NULL;
    }
    *reader = r;
    assign_statement_t* out = malloc(sizeof(assign_statement_t));
    *out = stm;
    return (statement_t*) out;
}

statement_t* parse_block(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    list_t stmlist;
    skip_whitespace(&r);
    if (!parse_specific_char(&r, &exc, '{')) {
        update_exc(excptr, make_exception(exc, 0, r,
            "exprected \"{\" at the front of a block"));
        return NULL;
    }
    statement_t* stm;
    while ((stm = parse_statement(&r, NULL))) {
        append_ptr(stmlist, stm);
    }
    skip_whitespace(&r);
    if (!parse_specific_char(&r, &exc, '}')) {
        update_exc(excptr, make_exception(exc, 0, r,
            "exprected \"}\" at the end of a block"));
        for (size_t i = 0; i < stmlist->wsize; i++) {
            stm = get_ptr(stmlist, i);
            free_statement(stm);
        }
        list_destroy(stmlist);
        return NULL;
    }
    block_statement_t* block = malloc(sizeof(block_statement_t));
    block->type = BLOCK_STATEMENT;
    block->count = stmlist->wsize;
    block->statements = stack_disown(stmlist);
    *reader = r;
    return (statement_t*) block;
}

statement_t* parse_return(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    if (!parse_keyword(&r, &exc, "return")) {
        update_exc(excptr, make_exception(exc, 0, r, "expected \"return\""));
        return NULL;
    }
    skip_whitespace(&r);
    value_t* val = parse_value(&r, &exc);
    if (val == NULL) {
        update_exc(excptr, make_exception(exc, 1, r,
            "could not parse an expression"));
        return NULL;
    }
    skip_whitespace(&r);
    if (!parse_specific_char(&r, &exc, ';')) {
        free_value(val);
        update_exc(excptr, make_exception(exc, 1, r, "expected a \";\""));
        return NULL;
    }
    return_statement_t* stm = malloc(sizeof(return_statement_t));
    stm->type = RETURN_STATEMENT;
    stm->val = val;
    *reader = r;
    return (statement_t*) val;
}

statement_t* parse_if(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    skip_whitespace(&r);
    if (!parse_keyword(&r, &exc, "return")) {
        update_exc(excptr, make_exception(exc, 0, r, "expected \"if\""));
        return NULL;
    }
    skip_whitespace(&r);
    if (!parse_keyword(&r, &exc, "(")) {
        update_exc(excptr, make_exception(exc, 1, r, "expected \"(\""));
        return NULL;
    }
    value_t* condition = parse_value(&r, &exc);
    if (condition == NULL) {
        update_exc(excptr, make_exception(exc, 2, r,
            "could not parse an expression"));
        return NULL;
    }
    if (!parse_keyword(&r, &exc, ")")) {
        free(condition);
        update_exc(excptr, make_exception(exc, 3, r, "expected \")\""));
        return NULL;
    }
    statement_t* iftrue = parse_statement(&r, &exc);
    if (iftrue == NULL) {
        free_value(condition);
        update_exc(excptr, make_exception(exc, 2, r,
            "could not parse a statement"));
        return NULL;
    }
    statement_t* iffalse = NULL;
    if (parse_keyword(&r, &exc, "else")) {
        iffalse = parse_statement(&r, &exc);
        if (iffalse == NULL) {
            free_value(condition);
            free_statement(iftrue);
            update_exc(excptr, make_exception(exc, 1, r,
                "could not parse a statement"));
            return NULL;
        }
    }
    if_statement_t* out = malloc(sizeof(if_statement_t));
    out->type = IF_STATEMENT;
    out->cond = condition;
    out->tstm = iftrue;
    out->fstm = iffalse;
    return out;
}

statement_t* parse_statement(reader_t* reader, Exception** excptr) {
    statement_t* (*parse_func[])(reader_t*, Exception**) = {
        parse_empty_statement, parse_assign_statement, parse_block,
        parse_return, parse_if, parse_for, parse_while, parse_do_while
    };
    Exception* temp_exc = NULL;
    statement_t* out;
    for (size_t i = 0; i < ARRLEN(parse_func); i++) {
        Exception* exc = NULL;
        if ((out = parse_func[i](reader, &exc))) {
            if (temp_exc) free_exception(temp_exc);
            return out;
        }
        else update_exc(&temp_exc, exc);
    }
    if(excptr) *excptr = temp_exc;
    else free_exception(temp_exc);
    return NULL;
}