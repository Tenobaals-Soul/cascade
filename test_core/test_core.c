#define NO_MACRO_OVERRIDE
#include <stdio.h>
#include <test_core.h>
#include <stdarg.h>

static unsigned int assert_counter = 0, pass_counter = 0;

void start() {}

void end() {
    if (assert_counter > pass_counter) {
        printf("\033[91mtest failed [%d/%d] asserts passed.\033[0m\n", pass_counter, assert_counter);
    }
    else {
        printf("\033[92mtest passed [%d/%d] asserts passed.\033[0m\n", pass_counter, assert_counter);
    }
}

void _assert(bool condition, size_t line, const char* file, char* f1, char* f3, ...) {
    assert_counter++;
    if (condition) {
        printf("\033[92massert %4u passed at %s:%lu\033[0m\n", assert_counter, file, (unsigned long) line);
        pass_counter++;
    }
    else {
        printf("\033[91massert %4u failed at %s:%lu\033[0m\n", assert_counter, file, (unsigned long) line);
        char* f2 = " != ";
        char* format = malloc(strlen(f1) + strlen(f2) + strlen(f3) + 1);
        strcpy(format, f1);
        strcat(format, f2);
        strcat(format, f3);
        va_list l;
        va_start(l, format);
        printf("    ");
        vprintf(format, l);
        printf("\n");
        va_end(l);
    }
}