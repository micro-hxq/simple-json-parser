//
// Created by WT on 2016/10/23.
//

#include <stdio.h>
#include <string.h>
#include <string>
#include "JsonParser.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        ++test_count;\
        if(equality)\
            ++test_pass;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format ", actual: " format "\n",\
                     __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

#define EXPECT_EQ_CAST_INT(expect, actual) \
    EXPECT_EQ_INT(wt::json::toIntType(expect), wt::json::toIntType(actual))

#define EXPECT_EQ_DOUBLE(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lf")

#define EXPECT_EQ_STRING(expect, actual, length) \
    EXPECT_EQ_BASE((::strncmp(expect, actual, length) == 0), expect, actual, "%s")

#define EXPECT_EQ_SIZE_T(expect, actual)    \
    EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")

#define TEST_EXPRESSION(statexpr, valuexpr, jsonexpr) \
    do {\
        wt::json::Json_value v;\
        v.type = wt::json::Json_t::json_null;\
        EXPECT_EQ_CAST_INT(statexpr, wt::json::parse(&v, jsonexpr));\
        EXPECT_EQ_CAST_INT(valuexpr, getType(v));\
    } while(0)

#define TEST_ERROR(error, jsonexpr) \
    do {\
        wt::json::Json_value v;\
        v.type = wt::json::Json_t::json_null;\
        EXPECT_EQ_CAST_INT(error, wt::json::parse(&v, jsonexpr));\
    } while(0)

#define TEST_NUMBER(expect, jsonexpr) \
    do {\
        wt::json::Json_value v;\
        EXPECT_EQ_CAST_INT(wt::json::State::ok, wt::json::parse(&v, jsonexpr));\
        EXPECT_EQ_CAST_INT(wt::json::Json_t::json_number, wt::json::getType(v));\
        EXPECT_EQ_DOUBLE(expect, wt::json::getNumber(v));\
    } while(0)


using namespace wt::json;

static void test_parse_null()
{
//    Json_value v;
//    v.value = Json_t ::json_true;
//    EXPECT_EQ_CAST_INT(State::ok, parse(&v, "null"));
//    EXPECT_EQ_CAST_INT(Json_t::json_null, getType(v));
    TEST_EXPRESSION(State::ok, Json_t::json_null, "null");
}

static void test_parse_false()
{
//    Json_value v;
//    v.value = Json_t::json_null;
//    EXPECT_EQ_CAST_INT(State::ok, parse(&v, "false"));
//    EXPECT_EQ_CAST_INT(Json_t::json_false, getType(v));
    TEST_EXPRESSION(State::ok, Json_t::json_false, "false");
}

static void test_parse_true()
{
//    Json_value v;
//    v.value = Json_t::json_null;
//    EXPECT_EQ_CAST_INT(State::ok, parse(&v, "true"));
//    EXPECT_EQ_CAST_INT(Json_t::json_true, getType(v));
    TEST_EXPRESSION(State::ok, Json_t::json_true, "true");
}

static void test_parse_expect_value()
{
//    Json_value v;
//    v.value = Json_t::json_null;
//    EXPECT_EQ_CAST_INT(State::except_value, parse(&v, ""));
    TEST_EXPRESSION(State::except_value, Json_t::json_null, "");
    TEST_EXPRESSION(State::except_value, Json_t::json_null, "\t\t\n ");
}

static void test_parse_root_not_singular()
{
    TEST_ERROR(State::root_not_singular, " null true");
}

static void test_parse_invalid_value()
{
//    Json_value v;
//    v.value = Json_t::json_null;
//    EXPECT_EQ_CAST_INT(State::invalid_value, parse(&v, "qq"));
    TEST_ERROR(State::invalid_value, "qq");
}

static void test_number()
{

    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");

    TEST_ERROR(State::number_too_big, "1e1000");
    TEST_ERROR(State::number_too_big, "1e309");
    TEST_ERROR(State::root_not_singular, "0123");
    TEST_ERROR(State::invalid_value, "1.");
    TEST_ERROR(State::invalid_value, ".1");
    TEST_ERROR(State::invalid_value, "+10");
    TEST_ERROR(State::invalid_value, "inf");

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");
    TEST_NUMBER( 4.9406564584124654e-324, "4.9406564584124654e-324");
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER( 2.2250738585072009e-308, "2.2250738585072009e-308");
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER( 2.2250738585072014e-308, "2.2250738585072014e-308");
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER( 1.7976931348623157e+308, "1.7976931348623157e+308");
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

static void test_access_string()
{
    Json_value v;
    initJson(&v);
    setString(&v, "", 0);
    EXPECT_EQ_STRING("", getString(v), getStringLength(v));
    setString(&v, "CLion", 5);
    EXPECT_EQ_STRING("CLion", getString(v), getStringLength(v));
    freeJson(&v);
}

static void test_parse_string()
{
    Json_value v;
    v.type = Json_t::json_string;
    EXPECT_EQ_CAST_INT(State::ok, parse(&v, "\"hello\\tworld\""));
    EXPECT_EQ_STRING("hello\tworld", getString(v), sizeof("hello\tworld"));
    setNumber(&v, 10.5);
    EXPECT_EQ_DOUBLE(10.5, getNumber(v));
}

static void test_parse_unicode()
{
    Json_value v;
    v.type = Json_t::json_string;
    EXPECT_EQ_CAST_INT(State::ok, parse(&v, "\"hell\u006f\u0020 worl\u0064\""));
    EXPECT_EQ_STRING("hello  world", getString(v), 12);
}

static void test_parse_array()
{
    Json_value v;
    initJson(&v);
    EXPECT_EQ_INT(State::ok, parse(&v, "[ ]"));
    EXPECT_EQ_INT(Json_t::json_array, getType(v));
    EXPECT_EQ_SIZE_T(0, getArraySize(v));
    freeJson(&v);

    initJson(&v);

    EXPECT_EQ_INT(State::ok, parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(Json_t::json_array, getType(v));
    EXPECT_EQ_SIZE_T(5, getArraySize(v));
    EXPECT_EQ_INT(Json_t::json_null, getType(*getArrayElem(v, 0)));
    EXPECT_EQ_INT(Json_t::json_false, getType(*getArrayElem(v, 1)));
    EXPECT_EQ_INT(Json_t::json_true, getType(*getArrayElem(v, 2)));
    EXPECT_EQ_INT(Json_t::json_number, getType(*getArrayElem(v, 3)));
    EXPECT_EQ_INT(Json_t::json_string, getType(*getArrayElem(v, 4)));
    EXPECT_EQ_DOUBLE(123, getNumber(*getArrayElem(v, 3)));
    EXPECT_EQ_STRING("abc", getString(*getArrayElem(v, 4)), getStringLength(*getArrayElem(v, 4)));
    freeJson(&v);

    initJson(&v);
    EXPECT_EQ_INT(State::ok, parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(Json_t::json_array, getType(v));
    EXPECT_EQ_SIZE_T(4, getArraySize(v));
    for (int i = 0; i < 4; i++) {
        Json_value* a = getArrayElem(v, i);
        EXPECT_EQ_INT(Json_t::json_array, getType(*a));
        EXPECT_EQ_SIZE_T(i, getArraySize(*a));
        for (int j = 0; j < i; j++) {
            Json_value* e = getArrayElem(*a, j);
            EXPECT_EQ_INT(Json_t::json_number, getType(*e));
            EXPECT_EQ_DOUBLE((double)j, getNumber(*e));
        }
    }
    freeJson(&v);
}

static void test_parse_miss_key()
{
    TEST_ERROR(State::miss_key, "{:1,");
    TEST_ERROR(State::miss_key, "{1:1,");
    TEST_ERROR(State::miss_key, "{true:1,");
    TEST_ERROR(State::miss_key, "{false:1,");
    TEST_ERROR(State::miss_key, "{null:1,");
    TEST_ERROR(State::miss_key, "{[]:1,");
    TEST_ERROR(State::miss_key, "{{}:1,");
    TEST_ERROR(State::miss_key, "{\"a\":1,");
}

static void test_parse_miss_colon()
{
    TEST_ERROR(State::miss_colon, "{\"a\"}");
    TEST_ERROR(State::miss_colon, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket()
{
    TEST_ERROR(State::miss_comma_or_curly_bracket, "{\"a\":1");
    TEST_ERROR(State::miss_comma_or_curly_bracket, "{\"a\":1]");
    TEST_ERROR(State::miss_comma_or_curly_bracket, "{\"a\":1 \"b\"");
    TEST_ERROR(State::miss_comma_or_curly_bracket, "{\"a\":{}");
}

static void test_parse_object()
{
    Json_value v;

    initJson(&v);
    EXPECT_EQ_INT(State::ok, parse(&v, " { } "));
    EXPECT_EQ_INT(Json_t::json_object, getType(v));
    EXPECT_EQ_SIZE_T(0, getObjectsize(v));
    freeJson(&v);

    initJson(&v);
    EXPECT_EQ_INT(State::ok, parse(&v,
    " { "
            "\"n\" : null , "
            "\"f\" : false , "
            "\"t\" : true , "
            "\"i\" : 123 , "
            "\"s\" : \"abc\", "
            "\"a\" : [ 1, 2, 3 ],"
            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
            " } "
    ));
    EXPECT_EQ_INT(Json_t::json_object, getType(v));
    EXPECT_EQ_SIZE_T(7, getObjectsize(v));
    EXPECT_EQ_STRING("n", getObjectKey(v, 0), getObjectKeyLen(v, 0));
    EXPECT_EQ_INT(Json_t::json_null,   getType(*getObjectValue(v, 0)));
    EXPECT_EQ_STRING("f", getObjectKey(v, 1), getObjectKeyLen(v, 1));
    EXPECT_EQ_INT(Json_t::json_false,  getType(*getObjectValue(v, 1)));
    EXPECT_EQ_STRING("t", getObjectKey(v, 2), getObjectKeyLen(v, 2));
    EXPECT_EQ_INT(Json_t::json_true, getType(*getObjectValue(v, 2)));
    EXPECT_EQ_STRING("i", getObjectKey(v, 3), getObjectKeyLen(v, 3));
    EXPECT_EQ_INT(Json_t::json_number, getType(*getObjectValue(v, 3)));
    EXPECT_EQ_DOUBLE(123.0, getNumber(*getObjectValue(v, 3)));
    EXPECT_EQ_STRING("s", getObjectKey(v, 4), getObjectKeyLen(v, 4));
    EXPECT_EQ_INT(Json_t::json_string, getType(*getObjectValue(v, 4)));
    EXPECT_EQ_STRING("abc", getString(*getObjectValue(v, 4)), getStringLength(*getObjectValue(v, 4)));
    EXPECT_EQ_STRING("a", getObjectKey(v, 5), getObjectKeyLen(v, 5));
    EXPECT_EQ_INT(Json_t::json_array, getType(*getObjectValue(v, 5)));
    EXPECT_EQ_SIZE_T(3, getArraySize(*getObjectValue(v, 5)));
    for (size_t i = 0; i < 3; i++)
    {
        Json_value* e = getArrayElem(*getObjectValue(v, 5), i);
        EXPECT_EQ_INT(Json_t::json_number, getType(*e));
        EXPECT_EQ_DOUBLE(i + 1.0, getNumber(*e));
    }
    EXPECT_EQ_STRING("o", getObjectKey(v, 6), getObjectKeyLen(v, 6));
    {
        Json_value* o = getObjectValue(v, 6);
        EXPECT_EQ_INT(Json_t::json_object, getType(*o));
        for (size_t i = 0; i < 3; i++) {
            Json_value* ov = getObjectValue(*o, i);
            EXPECT_EQ_SIZE_T(1, getObjectKeyLen(*o, i));
            EXPECT_EQ_INT(Json_t::json_number, getType(*ov));
            EXPECT_EQ_DOUBLE(i + 1.0, getNumber(*ov));
        }
    }
    freeJson(&v);
}
static void test_parse()
{
    test_parse_null();
    test_parse_false();
    test_parse_true();
    test_parse_expect_value();
    test_parse_root_not_singular();
    test_parse_invalid_value();
    test_number();
    test_access_string();
    test_parse_string();
    test_parse_unicode();
    test_parse_array();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
    test_parse_object();
}

int main()
{
    test_parse();
    printf("%d/%d (%4.2f)%% passed\n", test_pass, test_count, test_pass*100.0/test_count);
    return main_ret;
}