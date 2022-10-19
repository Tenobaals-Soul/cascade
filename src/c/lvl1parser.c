#define REQUIRE_LVL0_PARSER
#include <lvl1parser.h>
#undef REQUIRE_LVL0_PARSER
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <inttypes.h>
#include <errno.h>
#include <stack.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory.h>

typedef struct Exception {
    char* val;
    struct Exception* caused_by;
    size_t parsed_symbols;
    char* text_pos;
} Exception;

static void print_single_exception(Exception* exc) {
    char obuf[16];
    size_t len;
    for (len = 0; exc->text_pos[len] && exc->text_pos[len] != '\n' && len < 16; len++);
    if (len > 15) {
        memcpy(obuf, exc->text_pos, 12);
        strcpy(obuf + 12, "...");
    }
    else {
        memcpy(obuf, exc->text_pos, len);
        obuf[len] = 0;
    }
    printf("%s. At: %s", exc->val, obuf);
}

void print_exception(Exception* exc) {
    print_single_exception(exc);
    printf("\n");
    while (exc->caused_by) {
        exc = exc->caused_by;
        printf("    ");
        print_single_exception(exc);
        printf("\n");
    }
}

void update_exc(Exception** dest, Exception* val) {
    if (dest != NULL) {
        if (*dest == NULL) {
            *dest = val;
            return;
        }
        if (val == NULL) {
            free_exception(*dest);
            *dest = NULL;
            return;
        }
        if ((*dest)->parsed_symbols <= val->parsed_symbols) {
            free_exception(*dest);
            *dest = val;
            return;
        }
        free_exception(val);
    }
    else free_exception(val);
}

void free_exception(Exception* exc) {
    Exception* next;
    while (exc) {
        next = exc->caused_by;
        free(exc->val);
        free(exc);
        exc = next;
    }
}

#ifdef __GNUC__
__attribute__((__format__(__printf__, 4, 5)))
#endif
Exception* make_exception(Exception* caused_by, size_t parsed_symbols, reader_t r, const char* format, ...) {
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
    exc->text_pos = r.text + r.cur;
    exc->val = buffer;
    return exc;
}

char parse_char(reader_t* reader, Exception** excptr) {
    char out = reader->text[reader->cur];
    if (out == 0) update_exc(excptr, make_exception(NULL, 0, *reader, "EOF"));
    else reader->cur++;
    return out;
}

char parse_specific_char(reader_t* reader, Exception** excptr, char expect) {
    reader_t r = *reader;
    Exception* exc = NULL;
    char out = parse_char(&r, &exc);
    if (out == 0) {
        exc = make_exception(exc, 0, r, "no digit was found");
        update_exc(excptr, exc);
        return 0;
    }
    if (out != expect) {
        exc = make_exception(NULL, 0, r, "expected '%c', not '%c'", expect, out);
        update_exc(excptr, exc);
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
        update_exc(excptr, make_exception(exc, 0, r, "no alphabetic character was found"));
        return 0;
    }
    if (!isalpha(out)) {
        update_exc(excptr, make_exception(NULL, 0, r, "'%c' is not an alphabetical character", out));
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
        update_exc(excptr, make_exception(exc, 0, r, "no punctuation character was found"));
        return 0;
    }
    if (!ispunct(out)) {
        update_exc(excptr, make_exception(NULL, 0, r, "'%c' is not a punctuation character", out));
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
        update_exc(excptr, make_exception(exc, 0, r, "no digit was found"));
        return 0;
    }
    if (!isdigit(out)) {
        update_exc(excptr, make_exception(NULL, 0, r, "'%c' is not a digit", out));
        return 0;
    }
    else {
        *reader = r;
        update_exc(excptr, NULL);
        return out;
    }
}

struct number parse_number(reader_t* reader, struct Exception** excptr) {
    char* startptr = reader->text + reader->cur;
    char* errptri;
    char* errptrf;
    uintmax_t iout = strtoumax(startptr, &errptri, 0);
    int ierr = errno;
    double fout = strtod(startptr, &errptrf);
    int ferr = errno;
    char* errptr, err;
    struct number to_ret;
    if (errptri >= errptrf) {
        errptr = errptri;
        err = ierr;
        to_ret.type = NINTEGER;
        to_ret.i = iout;
    }
    else {
        errptr = errptrf;
        err = ferr;
        to_ret.type = NFLOATING;
        to_ret.f = fout;
    }
    if (errptr == reader->text + reader->cur) {
        update_exc(excptr, make_exception(NULL, 0, *reader, "not a number"));
        return (struct number) {0};
    }
    if (err == ERANGE) {
        update_exc(excptr, make_exception(NULL, 1, *reader, "number too big"));
        return (struct number) {0};
    }
    update_exc(excptr, NULL);
    reader->cur += errptr - startptr;
    return to_ret;
}

char* parse_identifier(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    Exception* exc = NULL;
    if ((c = parse_alpha(reader, &exc)) ||
        (c = parse_specific_char(reader, &exc, '_')));
    else {
        update_exc(excptr, make_exception(exc, 0, *reader, "not an identifier"));
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
    update_exc(excptr, make_exception(NULL, 0, *reader, "'%c' is not a valid character for an operator", c));
    return 0;
}

char* parse_operator(reader_t* reader, Exception** excptr) {
    stack_t stack;
    init_stack(stack);
    char c;
    Exception* exc = NULL;
    if ((c = parse_operator_char(reader, &exc)));
    else {
        update_exc(excptr, make_exception(exc, 0, *reader, "not an identifier"));
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

bool parse_keyword(reader_t* reader, Exception** excptr, const char* expect) {
    reader_t r;
    for (size_t i = 0; expect[i]; i++) {
        Exception* exc = NULL;
        if (parse_specific_char(&r, &exc, expect[i]) == 0) {
            update_exc(excptr, make_exception(exc, 0, r, "expected \"%s\"", expect));
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
        update_exc(excptr, make_exception(exc, 0, r, "expected a character"));
        return 0;
    }
    if ((c = parse_char(&r, &exc)) == 0) { // fix later to support excape sequences
        update_exc(excptr, make_exception(exc, 1, r, "expected a character"));
        return 0;
    }
    if (parse_specific_char(&r, &exc, '\'') == 0) {
        update_exc(excptr, make_exception(exc, 1, r, "expected a closing '"));
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
        update_exc(excptr, make_exception(exc, 0, r, "expected a string"));
        return NULL;
    }
    stack_t stack;
    init_stack(stack);
    for (;;) {
        char c;
        if ((c = parse_char(&r, &exc)) == 0 || c == '\n') {
            update_exc(excptr, make_exception(exc, 1, r, "no closing \" was found"));
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