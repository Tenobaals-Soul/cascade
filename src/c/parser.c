#include <parser.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stack.h>
#include <stddef.h>
#include <stdbool.h>

#define make_conditional_exception(caused_by, parsed_symbols, format, ...)\
(caused_by != NULL ? make_exception(caused_by, parsed_symbols, format##__VA_ARGS__) : NULL)

#define update_exc(dest, val) do { if (dest) *dest = val; else free_exception(val); } while(0)

typedef struct Exception {
    char* val;
    struct Exception* caused_by;
    size_t parsed_symbols;
} Exception;

void free_exception(Exception* exc) {
    Exception* next;
    while (exc) {
        next = exc->caused_by;
        free(exc);
        exc = next;
    }
}

#ifdef __GNUC__
__attribute__((__format__(__printf__, 3, 4)))
#endif
Exception* make_exception(Exception* caused_by, size_t parsed_symbols, const char* format, ...) {
    va_list l;
    va_start(l, format);
    int len = vsnprintf(NULL, 0, format, l);
    va_end(l);
    char* buffer = malloc(len + 1);
    va_start(l, format);
    vsprintf(buffer, format, l);
    va_end(l);
    Exception* exp = malloc(sizeof(Exception));
    exp->caused_by = caused_by;
    exp->parsed_symbols = parsed_symbols;
    exp->val = buffer;
    return exp;
}

char parse_char(reader_t* reader, Exception** excptr) {
    char out = reader->text[reader->cur];
    if (out == 0) update_exc(excptr, make_exception(NULL, 0, "EOF"));
    else reader->cur++;
    return out;
}

char parse_specific_char(reader_t* reader, Exception** excptr, char expect) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char out = parse_char(&r, &exc);
    update_exc(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (out == expect) {
        *reader = r;
        return out;
    }
    else {
        update_exc(excptr, make_exception(NULL, 0, "expected '%c', not '%c'", expect, out));
        return 0;
    }
}

char parse_alpha(reader_t* reader, Exception** excptr) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    update_exc(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (isalpha(out)) {
        *reader = temp;
        return out;
    }
    else {
        update_exc(excptr, make_exception(NULL, 0, "'%c' is not a alphabetical character", out));
        return 0;
    }
}

char parse_punct(reader_t* reader, Exception** excptr) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    update_exc(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (ispunct(out)) {
        *reader = temp;
        return out;
    }
    else {
        update_exc(excptr, make_exception(NULL, 0, "'%c' is not a punctuation character", out));
        return 0;
    }
}

char parse_digit(reader_t* reader, Exception** excptr) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    update_exc(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (isdigit(out)) {
        *reader = temp;
        return out;
    }
    else {
        update_exc(excptr, make_exception(NULL, 0, "'%c' is not a digit", out));
        return 0;
    }
}

uintmax_t parse_integer(reader_t* reader, Exception** excptr) {
    char* startptr = reader->text + reader->cur;
    char* errptr;
    uintmax_t out = strtoumax(startptr, &errptr, 0);
    if (errptr == reader->text + reader->cur) {
        update_exc(excptr, make_exception(NULL, 0, "not a number"));
        return 0;
    }
    if (errno == ERANGE) {
        update_exc(excptr, make_exception(NULL, 1, "number too big"));
        return 0;
    }
    return out;
}

double parse_floating(reader_t* reader, Exception** excptr) {
    char* errptr = reader->text + reader->cur;
    double out = strtold(errptr, &errptr);
    if (errptr == reader->text + reader->cur) {
        update_exc(excptr, make_exception(NULL, 0, "not a number"));
        return 0;
    }
    if (errno == ERANGE) {
        update_exc(excptr, make_exception(NULL, 1, "number too big"));
        return 0;
    }
    return out;
}

char* parse_identifier(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    Exception* exp;
    if ((c = parse_alpha(reader, &exp)) ||
        (c = parse_specific_char(reader, &exp, '_')));
    else {
        update_exc(excptr, make_exception(exp, 0, "not an identifier"));
        return NULL;
    }
    push_chr(stack, c);
    for (;;) {
        if ((c = parse_alpha(reader, NULL)) ||
            (c = parse_specific_char(reader, NULL, '_')) ||
            (c = parse_digit(reader, NULL))) {
            push_chr(stack, c);
        }
        else break;
    }
    return stack_disown(stack);
}

char parse_operator_char(reader_t* reader, Exception** excptr) {
    char c;
    if ((c = parse_alpha(reader, NULL))) return c;
    if ((c = parse_punct(reader, NULL))) return c;
    if ((c = parse_specific_char(reader, NULL, '_'))) return c;
    update_exc(excptr, make_exception(NULL, 0, "'%c' is not a valid character for an operator", c));
    return 0;
}

char* parse_operator(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    Exception* exc;
    if ((c = parse_operator_char(reader, &exc)));
    else {
        update_exc(excptr, make_exception(exc, 0, "not an identifier"));
        return NULL;
    }
    push_chr(stack, c);
    for (;;) {
        if ((c = parse_operator_char(reader, NULL)) ||
            (c = parse_digit(reader, NULL))) {
            push_chr(stack, c);
        }
        else break;
    }
    return stack_disown(stack);
}

bool parse_keyword(reader_t* reader, Exception** excptr, const char* expect) {
    reader_t r;
    for (size_t i = 0; expect[i]; i++) {
        Exception* exc = NULL;
        if (parse_specific_char(&r, &exc, expect[i]) == 0) {
            update_exc(excptr, make_exception(exc, 0, "expected \"%s\"", expect));
            return false;
        }
    }
    *reader = r;
    return true;
}

char* parse_string(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    if (parse_specific_char(&r, &exc, '\"') == 0) {
        update_exc(excptr, make_exception(exc, 0, "expected a string"));
        return NULL;
    }
    stack_t stack;
    init_stack(stack);
    for (;;) {
        char c;
        Exception* exc;
        if ((c = parse_char(&r, &exc)) == 0) {
            update_exc(excptr, make_exception(exc, 1, "no closing \" was found"));
            destroy_stack(stack);
            return NULL;
        }
        else if (c == '\"') break;
        push_chr(stack, c);
    }
    *reader = r;
    return stack_disown(stack);
}