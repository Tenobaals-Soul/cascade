#include <parser.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stack.h>

#define make_conditional_exception(caused_by, parsed_chars, format, ...)\
(caused_by != NULL ? make_exception(caused_by, parsed_chars, format##__VA_ARGS__) : NULL)

#define safe_set(dest, val) do { if (dest) *dest = val; } while(0)

typedef struct Exception {
    char* val;
    Exception* caused_by;
    size_t parsed_chars;
} Exception;

typedef struct reader_t {
    const char* text;
    size_t cur;
} reader_t;

void free_exception(Exception* exc) {
    Exception* next;
    while (exc) {
        next = exc->caused_by;
        free(exc);
        exc = next;
    }
}

Exception* make_exception(Exception* caused_by, size_t parsed_chars, const char* format, ...) __attribute_format_arg__(3);
Exception* make_exception(Exception* caused_by, size_t parsed_chars, const char* format, ...) {
    va_list l;
    va_start(l, format);
    int len = vsnprintf(NULL, 0, format, l);
    va_end(l);
    char* buffer = malloc(len + 1);
    va_list l;
    va_start(l, format);
    vsprintf(buffer, format, l);
    va_end(l);
    return buffer;
}

char parse_char(reader_t* reader, Exception** excptr) {
    char out = reader->text[reader->cur];
    if (out == '\0') *excptr = make_exception(NULL, 0, "EOF");
    else reader->cur++;
    return out;
}

char parse_specific_char(reader_t* reader, Exception** excptr, char expect) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    safe_set(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (out == expect) {
        *reader = temp;
        return out;
    }
    else {
        safe_set(excptr, make_exception(NULL, 0, "expected '%c', not '%c'", expect, out));
        return 0;
    }
}

char parse_alpha(reader_t* reader, Exception** excptr) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    safe_set(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (isalpha(out)) {
        *reader = temp;
        return out;
    }
    else {
        safe_set(excptr, make_exception(NULL, 0, "'%c' is not a alphabetical character", out));
        return 0;
    }
}

char parse_punct(reader_t* reader, Exception** excptr) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    safe_set(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (ispunct(out)) {
        *reader = temp;
        return out;
    }
    else {
        safe_set(excptr, make_exception(NULL, 0, "'%c' is not a punctuation character", out));
        return 0;
    }
}

char parse_digit(reader_t* reader, Exception** excptr) {
    reader_t temp = *reader;
    Exception* exc = NULL;
    char out = parse_char(&temp, &exc);
    safe_set(excptr, make_conditional_exception(exc, 0, "no digit was found"));
    if (isdigit(out)) {
        *reader = temp;
        return out;
    }
    else {
        safe_set(excptr, make_exception(NULL, 0, "'%c' is not a digit", out));
        return 0;
    }
}

uint64_t parse_integer(reader_t* reader, Exception** excptr) {
    char* errptr = reader->text + reader->cur;
    uint64_t out = strtoumax(errptr, &errptr, 0);
    if (*errptr == reader->text + reader->cur) {
        safe_set(excptr, make_exception(NULL, 0, "not a number"));
        return 0;
    }
    if (errno == ERANGE) {
        safe_set(excptr, make_exception(NULL, (size_t) *errptr - (size_t) reader->text + reader->cur, "number too big"));
        return 0;
    }
    return out;
}

double parse_floating(reader_t* reader, Exception** excptr) {
    char* errptr = reader->text + reader->cur;
    double out = strtod(errptr, &errptr);
    if (*errptr == reader->text + reader->cur) {
        safe_set(excptr, make_exception(NULL, 0, "not a number"));
        return 0;
    }
    return out;
}

char* parse_identifier(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    if ((c = parse_alpha(reader, NULL)) || (c = parse_specific_char(reader, NULL, '_')));
    else {
        safe_set(excptr, make_exception(NULL, 0, "not an identifier"));
        return NULL;
    }
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
    safe_set(excptr, make_exception(NULL, 0, "'%c' is not a valid character for an operator", c));
    return 0;
}

char* parse_operator(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    if ((c = parse_operator_char(reader, NULL)));
    else {
        safe_set(excptr, make_exception(NULL, 0, "not an identifier"));
        return NULL;
    }
    for (;;) {
        if ((c = parse_operator_char(reader, NULL)) ||
            (c = parse_digit(reader, NULL))) {
            push_chr(stack, c);
        }
        else break;
    }
    return stack_disown(stack);
}