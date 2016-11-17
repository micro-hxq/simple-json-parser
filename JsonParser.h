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
    json_object
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
    invalid_unicode_surrogate
};

struct Json_value {
    Json_t type;
    union {
        double number;
        struct {
            char* pstr;
            std::size_t len;
        };
    };
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
State       parseString(Context *pc, Json_value *pv);


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



} // namespace json
} // namespace wt

#endif //JSONPARSER_JSONPARSER_H
