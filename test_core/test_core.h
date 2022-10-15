#ifndef INCLUDE_TEST_CORE_H
#define INCLUDE_TEST_CORE_H
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void start();

void _assert(bool condition, size_t line, const char* file, char* f1, char* f3, ...);

#define EVAL_TYPE_FORMATER(exp) (_Generic((exp), \
        char: "%d", \
        short: "%d", \
        int: "%d", \
        long: "%ld", \
        long long: "%lld", \
        unsigned char: "%u", \
        unsigned short: "%u", \
        unsigned int: "%u", \
        unsigned long: "%lu", \
        unsigned long long: "%llu", \
        float: "%f", \
        double: "%f", \
        long double: "%lf", \
        default: "%p" \
    ))

#define assert_equal(x, y) do {\
    typeof(x) evalx = x;\
    typeof(y) evaly = y;\
    _assert(evalx == evaly, __LINE__, __FILE__,\
        (EVAL_TYPE_FORMATER(evalx)), EVAL_TYPE_FORMATER(evaly), evalx, evaly\
    );\
} while (0)

#define FORMAT_STRING(str) str ? "\"": "", str ? str : "NULL", str ? "\"" : ""
#define assert_str_equal(x, y) do {\
    char* evalx = x;\
    char* evaly = y;\
    _assert(evalx == evaly || (evalx && evaly && strcmp(evalx, evaly) == 0), __LINE__, __FILE__,\
        "%s%s%s", "%s%s%s", FORMAT_STRING(evalx), FORMAT_STRING(evaly)\
    );\
} while (0)

#define assert_true(x) do {\
    typeof(x) evalx = x;\
    _assert(evalx, __LINE__, __FILE__,\
        EVAL_TYPE_FORMATER(evalx), "true", evalx\
    );\
} while (0)

void end();

#endif