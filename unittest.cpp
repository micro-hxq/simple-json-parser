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
}

int main()
{
    test_parse();
    printf("%d/%d (%4.2f)%% passed\n", test_pass, test_count, test_pass*100.0/test_count);
    return main_ret;
}