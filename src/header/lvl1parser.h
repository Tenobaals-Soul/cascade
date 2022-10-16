#ifndef INCLUDE_LVL1PARSER_H
#define INCLUDE_LVL1PARSER_H
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct reader_t {
    char* text;
    size_t cur;
} reader_t;

struct Exception;

#if !defined(TYPES_ONLY)
#if defined(TEST) || defined(REQUIRE_LVL0_PARSER)
char parse_char(reader_t* reader, struct Exception** excptr);
char parse_specific_char(reader_t* reader, struct Exception** excptr, char expect);
char parse_alpha(reader_t* reader, struct Exception** excptr);
char parse_punct(reader_t* reader, struct Exception** excptr);
char parse_digit(reader_t* reader, struct Exception** excptr);
#endif
void free_exception(struct Exception* exc);
uintmax_t parse_integer(reader_t* reader, struct Exception** excptr);
double parse_floating(reader_t* reader, struct Exception** excptr);
char parse_character(reader_t* reader, struct Exception** excptr);
char* parse_identifier(reader_t* reader, struct Exception** excptr);
char parse_operator_char(reader_t* reader, struct Exception** excptr);
char* parse_operator(reader_t* reader, struct Exception** excptr);
bool parse_keyword(reader_t* reader, struct Exception** excptr, const char* expect);
char* parse_string(reader_t* reader, struct Exception** excptr);
#endif

#endif