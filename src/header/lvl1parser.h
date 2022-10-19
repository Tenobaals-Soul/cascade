#ifndef INCLUDE_LVL1PARSER_H
#define INCLUDE_LVL1PARSER_H
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct reader_t {
    char* text;
    size_t cur;
    size_t line;
    size_t loff;
} reader_t;

struct Exception;

struct number {
    enum type {
        NNONE, NINTEGER, NFLOATING
    } type;
    union {
        uintmax_t i;
        double f;
    };
};

#if !defined(TYPES_ONLY)
#if defined(TEST) || defined(REQUIRE_LVL0_PARSER)
char parse_char(reader_t* reader, struct Exception** excptr);
char parse_specific_char(reader_t* reader, struct Exception** excptr, char expect);
char parse_alpha(reader_t* reader, struct Exception** excptr);
char parse_punct(reader_t* reader, struct Exception** excptr);
char parse_digit(reader_t* reader, struct Exception** excptr);
void update_exc(struct Exception** dest, struct Exception* val);
void free_exception(struct Exception* exc);
#ifdef __GNUC__
__attribute__((__format__(__printf__, 4, 5)))
#endif
struct Exception* make_exception(struct Exception* caused_by, size_t parsed_symbols, reader_t r, const char* format, ...);
#endif
void free_exception(struct Exception* exc);
void print_exception(struct Exception* exc);
struct number parse_number(reader_t* reader, struct Exception** excptr);
char parse_character(reader_t* reader, struct Exception** excptr);
char* parse_identifier(reader_t* reader, struct Exception** excptr);
char parse_operator_char(reader_t* reader, struct Exception** excptr);
char* parse_operator(reader_t* reader, struct Exception** excptr);
bool parse_keyword(reader_t* reader, struct Exception** excptr, const char* expect);
char* parse_string(reader_t* reader, struct Exception** excptr);
#endif

#endif