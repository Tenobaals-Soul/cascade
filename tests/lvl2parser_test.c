#if !defined (TEST)
#define TEST
#endif
#if !defined(DEBUG)
#define DEBUG
#endif
#include <lvl2parser.h>
#include <test_core.h>
#include <stdio.h>

reader_t* mr(char* t) {
    reader_t* r = malloc(sizeof(reader_t));
    r[0] = (reader_t) {t, 0, 0, 0};
    return r;
}

int main() {
    start();
    value_char_t* vc = (void*) parse_value(mr("'a'e"), NULL);
    assert_equal((int) vc->type, VALUE_CHAR);
    assert_equal(vc->value, 'a');
    value_integer_t* vi = (void*) parse_value(mr("420e"), NULL);
    assert_equal((int) vi->type, VALUE_INTEGER);
    assert_equal(vi->value, (uintmax_t) 420);
    value_floating_t* vf = (void*) parse_value(mr("12.5"), NULL);
    assert_equal((int) vf->type, VALUE_FLOATING);
    assert_equal(vf->value, 12.5);
    value_string_t* vs = (void*) parse_value(mr("\"hello\" + "), NULL);
    assert_equal((int) vs->type, VALUE_STRING);
    assert_str_equal(vs->value, "hello");
    end();
}