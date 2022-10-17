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
    r[0] = (reader_t) {t, 0};
    return r;
}

int main() {
    value_char_t vc = { 0 };
    vc.type = VALUE_CHAR;
    vc.value = 'a';
    assert_memeq(parse_value(mr("'a'e"), NULL), &vc, sizeof(vc));
    value_integer_t vi = { 0 };
    vi.type = VALUE_INTEGER;
    vi.value = 420;
    assert_memeq(parse_value(mr("420e"), NULL), &vi, sizeof(vi));
    value_floating_t vf = { 0 };
    vf.type = VALUE_FLOATING;
    vf.value = 12.5;
    value_floating_t* v = parse_value(mr("12.5"), NULL);
    assert_memeq(v, &vf, sizeof(vf));
    value_string_t* parsed_string = (value_string_t*) parse_value(mr("\"hello\" + "), NULL);
    assert_equal((int) parsed_string->type, VALUE_STRING);
    assert_str_equal(parsed_string->value, "hello");
}