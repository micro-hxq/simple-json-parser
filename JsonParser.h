//
// Created by WT on 2016/10/23.
//

#ifndef JSONPARSER_JSONPARSER_H
#define JSONPARSER_JSONPARSER_H

#include <type_traits>
#include <string>


#define initJson(pv)        do { (pv)->type = Json_t::json_null; } while(0)

namespace wt {
namespace json {

enum class Json_t {
    json_null,
    json_false,
    json_true,
    json_number,
    json_string,
    json_array,
    json_object,
};

enum class State {
    ok,
    except_value,
    invalid_value,
    root_not_singular,
    number_too_big,
    miss_quotation_mark,
    invalid_string_escape,
    invalid_unicode_hex,
    invalid_unicode_surrogate,
    miss_comma_or_square_bracket,
    miss_key,
    miss_colon,
    miss_comma_or_curly_bracket
};

struct Json_member;

struct Json_value {
    Json_t type;
    union {
        double number;
        struct {
            char* pstr;
            size_t len;
        };
        struct {
            Json_value* pa;
            size_t asize;
        };
        struct {
            Json_member* po;
            size_t osize;
        };
    };
};

struct Json_member {
    char*       pk;
    size_t      klen;
    Json_value  val;
};

struct Context {
    const char* pJson;
    char*       pStack;
    std::size_t size, top;
};


template <typename E>
constexpr auto toIntType(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

State       parse(Json_value* pv, const std::string& js);
void        parseWhiteSpace(Context* pc);
State       parseNull(Context *pc, Json_value *pv);
State       parseFalse(Context *pc, Json_value *pv);
State       parseTrue(Context *pc, Json_value *pv);
State       parseValue(Context* pc, Json_value* pv);
State       parseNumber(Context *pc, Json_value *pv);
State       parseStringRaw(Context *pc, char** str, size_t *len);
State       parseString(Context *pc, Json_value *pv);
State       parseArray(Context *pc, Json_value *pv);
State       parseObject(Context *pc, Json_value *pv);

void        freeJson(Json_value *pv);
void*       contextPush(Context *pc, std::size_t size);
void*       contextPop(Context *pc, std::size_t  size);

void        setBool(Json_value *pv, bool flag);
bool        getBool(const Json_value &v);

void        setNumber(Json_value *pv, double val);
double      getNumber(const Json_value &v);

void        setString(Json_value *pv, const char* str, std::size_t size);
const char* getString(const Json_value &v);
std::size_t getStringLength(const Json_value &v);
Json_t      getType(const Json_value& v);

const char* parseHex4(const char* p, unsigned* u);
void        encodeUTF8(Context *pc, unsigned u);

size_t      getArraySize(const Json_value &pv);
Json_value* getArrayElem(const Json_value &pv, size_t index);

size_t      getObjectsize(const Json_value &pv);
const char* getObjectKey(const Json_value &pv, size_t index);
size_t      getObjectKeyLen(const Json_value &pv, size_t index);
Json_value* getObjectValue(const Json_value &pv, size_t index);

} // namespace json
} // namespace wt

#endif //JSONPARSER_JSONPARSER_H
