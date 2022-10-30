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

void test_value_parsing() {
    struct Exception* exc = NULL;
    value_char_t* vc = (void*) parse_value(mr("'a'e"), &exc);
    if (vc == NULL) { print_exception(exc); exc = NULL; }
    else {
        assert_equal((int) vc->type, VALUE_CHAR);
        assert_equal(vc->value, 'a');
    }
    value_integer_t* vi = (void*) parse_value(mr("420e"), &exc);
    if (vi == NULL) { print_exception(exc); exc = NULL; }
    else {
        assert_equal((int) vi->type, VALUE_INTEGER);
        assert_equal(vi->value, (uintmax_t) 420);
    }
    value_floating_t* vf = (void*) parse_value(mr("12.5"), &exc);
    if (vf == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) vf->type, VALUE_FLOATING);
        assert_equal(vf->value, 12.5);
    }
    value_string_t* vs = (void*) parse_value(mr("\"hello\" + "), &exc);
    if (vs == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) vs->type, VALUE_STRING);
        assert_str_equal(vs->value, "hello");
    }
    value_tuple_t* vt = (void*) parse_value(mr("(\"hello\", 2)"), &exc);
    if (vt == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) vt->type, VALUE_TUPLE);
        assert_equal((int) *vt->value[0], VALUE_STRING);
        assert_equal((int) *vt->value[1], VALUE_INTEGER);
    }
    value_scalar_initializer_t* vv = (void*) parse_value(mr("{\"hello\", 2}"), &exc);
    if (vv == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) vv->type, VALUE_SCALAR_INITIALIZER);
        assert_equal((int) *vv->value[0], VALUE_STRING);
        assert_equal((int) *vv->value[1], VALUE_INTEGER);
    }
    value_variable_t* vn = (void*) parse_value(mr("Tree.EMPTY"), &exc);
    if (vn == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) vn->type, VALUE_VARIABLE);
        assert_true(memcmp(vn->name, "Tree\0EMPTY\0", sizeof("Tree\0EMPTY\0")) == 0);
    }
    vv = (void*) parse_value(mr("Tree.NODE{NONE, 0, NONE}"), &exc);
    if (vv == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) vv->type, VALUE_SCALAR_INITIALIZER);
        assert_equal((int) vv->items, 3);
        assert_equal((int) *vv->value[0], VALUE_VARIABLE);
        assert_equal((int) *vv->value[1], VALUE_INTEGER);
        assert_equal((int) *vv->value[2], VALUE_VARIABLE);
    }
}

void test_expression_parsing() {
    struct Exception* exc;
    value_operation_t* val;
    val = (void*) parse_expression(mr("5 + 7"), &exc);
    if (val == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) val->type, VALUE_OPERATION);
    }
    
    val = (void*) parse_expression(mr("5 + 7 * 5"), &exc);
    if (val == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) val->type, VALUE_OPERATION);
        assert_str_equal(val->operator_name, "+");
    }

    val = (void*) parse_expression(mr("(5 + 7) * 5"), &exc);
    if (val == NULL) { print_exception(exc); exc = NULL; assert_true(false); }
    else {
        assert_equal((int) val->type, VALUE_OPERATION);
        assert_str_equal(val->operator_name, "*");
    }
}

void test_structure_parsing() {
    type_t* type = parse_type(mr("string"), NULL);
    assert_str_equal(type->name, "string");
    assert_equal((int) type->generic_len, 0);
    type = parse_type(mr("Dictionary<string, string>"), NULL);
    assert_str_equal(type->name, "Dictionary");
    assert_equal((int) type->generic_len, 2);
    assert_str_equal(type->generic_val[0]->name, "string");
    assert_str_equal(type->generic_val[1]->name, "string");
}

int main() {
    start();
    test_value_parsing();
    test_expression_parsing();
    end();
}