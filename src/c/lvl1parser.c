#include <lvl1parser.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stack.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct Exception {
    char* val;
    struct Exception* caused_by;
    size_t parsed_symbols;
} Exception;

void update_exc(Exception** dest, Exception* val) {
    if (dest != NULL) {
        if (*dest == NULL) {
            *dest = val;
        }
        else if ((*dest)->parsed_symbols <= val->parsed_symbols) {
            free_exception(*dest);
            *dest = val;
        }
        else free_exception(val);
    }
    else free_exception(val);
}

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
    Exception* exc = malloc(sizeof(Exception));
    exc->caused_by = caused_by;
    exc->parsed_symbols = parsed_symbols;
    exc->val = buffer;
    return exc;
}

char parse_char(reader_t* reader, Exception** excptr) {
    char out = reader->text[reader->cur];
    if (out == 0) update_exc(excptr, make_exception(NULL, 0, "EOF"));
    else reader->cur++;
    return out;
}

char parse_specific_char(reader_t* reader, Exception** excptr, char excect) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char out = parse_char(&r, &exc);
    if (out == 0) {
        update_exc(excptr, make_exception(exc, 0, "no digit was found"));
        return 0;
    }
    if (out != excect) {
        update_exc(excptr, make_exception(NULL, 0, "excected '%c', not '%c'", excect, out));
        return 0;
    }
    else {
        *reader = r;
        update_exc(excptr, NULL);
        return out;
    }
}

char parse_alpha(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char out = parse_char(&r, &exc);
    if (out == 0) {
        update_exc(excptr, make_exception(exc, 0, "no alphabetic character was found"));
        return 0;
    }
    if (!isalpha(out)) {
        update_exc(excptr, make_exception(NULL, 0, "'%c' is not an alphabetical character", out));
        return 0;
    }
    else {
        *reader = r;
        update_exc(excptr, NULL);
        return out;
    }
}

char parse_punct(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char out = parse_char(&r, &exc);
    if (out == 0) {
        update_exc(excptr, make_exception(exc, 0, "no punctuation character was found"));
        return 0;
    }
    if (!ispunct(out)) {
        update_exc(excptr, make_exception(NULL, 0, "'%c' is not a punctuation character", out));
        return 0;
    }
    else {
        *reader = r;
        update_exc(excptr, NULL);
        return out;
    }
}

char parse_digit(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char out = parse_char(&r, &exc);
    if (out == 0) {
        update_exc(excptr, make_exception(exc, 0, "no digit was found"));
        return 0;
    }
    if (!isdigit(out)) {
        update_exc(excptr, make_exception(NULL, 0, "'%c' is not a digit", out));
        return 0;
    }
    else {
        *reader = r;
        update_exc(excptr, NULL);
        return out;
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
    update_exc(excptr, NULL);
    return out;
}

double parse_floating(reader_t* reader, Exception** excptr) {
    char* errptr = reader->text + reader->cur;
    double out = strtod(errptr, &errptr);
    if (errptr == reader->text + reader->cur) {
        update_exc(excptr, make_exception(NULL, 0, "not a number"));
        return 0;
    }
    if (errno == ERANGE) {
        update_exc(excptr, make_exception(NULL, 1, "number too big"));
        return 0;
    }
    update_exc(excptr, NULL);
    return out;
}

char* parse_identifier(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    Exception* exc = NULL;
    if ((c = parse_alpha(reader, &exc)) ||
        (c = parse_specific_char(reader, &exc, '_')));
    else {
        update_exc(excptr, make_exception(exc, 0, "not an identifier"));
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
    update_exc(excptr, NULL);
    return stack_disown(stack);
}

char parse_operator_char(reader_t* reader, Exception** excptr) {
    char c;
    update_exc(excptr, NULL);
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
    Exception* exc = NULL;
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
    update_exc(excptr, NULL);
    return stack_disown(stack);
}

bool parse_keyword(reader_t* reader, Exception** excptr, const char* excect) {
    reader_t r;
    for (size_t i = 0; excect[i]; i++) {
        Exception* exc = NULL;
        if (parse_specific_char(&r, &exc, excect[i]) == 0) {
            update_exc(excptr, make_exception(exc, 0, "excected \"%s\"", excect));
            return false;
        }
    }
    *reader = r;
    update_exc(excptr, NULL);
    return true;
}

char parse_character(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char c;
    if (parse_specific_char(&r, &exc, '\'') == 0) {
        update_exc(excptr, make_exception(exc, 0, "excected a character"));
        return 0;
    }
    if ((c = parse_char(&r, &exc)) == 0) { // fix later to support excape sequences
        update_exc(excptr, make_exception(exc, 1, "excected a character"));
        return 0;
    }
    if (parse_specific_char(&r, &exc, '\'') == 0) {
        update_exc(excptr, make_exception(exc, 1, "excected a closing '"));
        return 0;
    }
    update_exc(excptr, NULL);
    *reader = r;
    return c;
}

char* parse_string(reader_t* reader, Exception** excptr) {
    reader_t r = *reader;
    Exception* exc = NULL;
    if (parse_specific_char(&r, &exc, '\"') == 0) {
        update_exc(excptr, make_exception(exc, 0, "excected a string"));
        return NULL;
    }
    stack_t stack;
    init_stack(stack);
    for (;;) {
        char c;
        if ((c = parse_char(&r, &exc)) == 0) {
            update_exc(excptr, make_exception(exc, 1, "no closing \" was found"));
            destroy_stack(stack);
            return NULL;
        }
        else if (c == '\"') break;
        push_chr(stack, c);
    }
    update_exc(excptr, NULL);
    *reader = r;
    return stack_disown(stack);
}