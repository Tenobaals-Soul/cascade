#if !defined (TEST)
#define TEST
#endif
#if !defined(DEBUG)
#define DEBUG
#endif
#include <test_core.h>
#include <lvl1parser.h>
#include <stdlib.h>

reader_t* mr(char* t) {
    reader_t* r = malloc(sizeof(reader_t));
    r[0] = (reader_t) {t, 0};
    return r;
}

int main() {
    start();
    assert_equal(parse_char(mr("hello world"), NULL), 'h');
    assert_equal(parse_specific_char(mr("231"), NULL, '_'), 0);
    assert_equal(parse_specific_char(mr("_231"), NULL, '_'), '_');
    assert_equal(parse_alpha(mr("_231"), NULL), 0);
    assert_equal(parse_alpha(mr("hello"), NULL), 'h');
    assert_equal(parse_digit(mr("hello world"), NULL), 0);
    assert_equal(parse_digit(mr("123"), NULL), '1');
    assert_equal(parse_punct(mr("hello world"), NULL), 0);
    assert_equal(parse_punct(mr("/"), NULL), '/');
    assert_equal(parse_floating(mr("0.5"), NULL), 0.5);
    assert_equal(parse_floating(mr("r0.5"), NULL), 0);
    assert_equal(parse_character(mr("'a'"), NULL), 'a');
    assert_equal(parse_character(mr("a'"), NULL), 0);
    assert_str_equal(parse_identifier(mr("hello world"), NULL), "hello");
    assert_str_equal(parse_operator(mr("<swap> 2"), NULL), "<swap>");
    assert_str_equal(parse_operator(mr("2times 2"), NULL), NULL);
    assert_str_equal(parse_string(mr("\"hello world\""), NULL), "hello world");
    assert_str_equal(parse_string(mr("hello world"), NULL), NULL);
    end();
}