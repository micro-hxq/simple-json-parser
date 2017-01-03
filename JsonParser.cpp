//
// Created by WT on 2016/10/23.
//

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "JsonParser.h"

#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

#ifndef PARSE_STACK_SIZE
#define PARSE_STACK_SIZE 256
#endif

#define PUSHC(pc, ch)       do { *static_cast<char*>(contextPush(pc, sizeof(char))) = (ch); } while(0)
#define STRING_ERROR(ret)   do { pc->top = head; return ret; } while(0)

wt::json::State wt::json::parse(wt::json::Json_value *pv, const std::string &js)
{
    assert(pv != nullptr);
    wt::json::Context c;
    c.pJson = js.c_str();
    c.pStack = nullptr;
    c.size = 0;
    c.top = 0;
    initJson(pv);
    parseWhiteSpace(&c);
    State state;
    if((state = parseValue(&c, pv)) == State::ok)
    {
        parseWhiteSpace(&c);
        if(*c.pJson != '\0')
        {
            pv->type = Json_t::json_null;
            return State::root_not_singular;
        }
    }
    assert(c.top == 0);
    free(c.pStack);
    return state;
}

void wt::json::parseWhiteSpace(wt::json::Context *pc)
{
    const char *pCur = pc->pJson;
    while(*pCur == ' ' || *pCur == '\t' || *pCur == '\r' || *pCur == '\n')
        ++pCur;
    pc->pJson = pCur;
}

wt::json::State wt::json::parseNull(wt::json::Context *pc, wt::json::Json_value *pv)
{
    if(pc->pJson[0] != 'n' || pc->pJson[1] != 'u' || pc->pJson[2] != 'l' || pc->pJson[3] != 'l')
        return State::invalid_value;
    pc->pJson += 4;
    pv->type = Json_t::json_null;
    return State ::ok;
}

wt::json::State wt::json::parseFalse(wt::json::Context *pc, wt::json::Json_value *pv)
{
    if(pc->pJson[0] != 'f' || pc->pJson[1] != 'a' || pc->pJson[2] != 'l' ||
            pc->pJson[3] != 's' || pc->pJson[4] != 'e')
        return State::invalid_value;
    pc->pJson += 5;
    pv->type = Json_t::json_false;
    return State::ok;
}

wt::json::State wt::json::parseTrue(wt::json::Context *pc, wt::json::Json_value *pv)
{
    if(pc->pJson[0] != 't' || pc->pJson[1] != 'r' || pc->pJson[2] != 'u' || pc->pJson[3] != 'e')
        return State::invalid_value;
    pc->pJson += 4;
    pv->type = Json_t::json_true;
    return State::ok;
}

wt::json::State wt::json::parseValue(wt::json::Context *pc, wt::json::Json_value *pv)
{
    switch(*pc->pJson)
    {
        case 'n':   return parseNull(pc, pv);
        case 'f':   return parseFalse(pc, pv);
        case 't':   return parseTrue(pc, pv);
        case '\"':  return parseString(pc, pv);
        case '[':   return parseArray(pc, pv);
        case '{':   return parseObject(pc, pv);
        case '\0':  return State::except_value;
        default:    return parseNumber(pc, pv);
    }
}

wt::json::State wt::json::parseNumber(wt::json::Context *pc, wt::json::Json_value *pv)
{
    const char *pCur = pc->pJson;
    if(*pCur == '-')    ++pCur;
    if(*pCur == '0')    ++pCur;
    else
    {
        if(!ISDIGIT1TO9(*pCur))
            return State::invalid_value;
        for(++pCur; ISDIGIT(*pCur); ++pCur);
    }
    if(*pCur == '.')
    {
        ++pCur;
        if(!ISDIGIT(*pCur))
            return State::invalid_value;
        for(++pCur; ISDIGIT(*pCur); ++pCur);
    }
    if(*pCur == 'E' || *pCur == 'e')
    {
        ++pCur;
        if(*pCur == '+' || *pCur == '-')    ++pCur;
        if(!ISDIGIT(*pCur))
            return State::invalid_value;
        for(++pCur; ISDIGIT(*pCur); ++pCur);
    }
    pv->number = std::strtod(pc->pJson, nullptr);
    if(pv->number == HUGE_VAL || pv->number == -HUGE_VAL)
        return State::number_too_big;
    pv->type = Json_t::json_number;
    pc->pJson = pCur;
    return State::ok;
}

wt::json::State wt::json::parseStringRaw(Context *pc, char **str, size_t *len)
{
    size_t head = pc->top;
    const char* pCur = pc->pJson;
    unsigned u = 0;
    assert(*pCur == '\"');
    ++pCur;
    for(;;)
    {
        char ch = *pCur++;
        switch(ch)
        {
            case '\"':
                *len = pc->top - head;
                *str = static_cast<char*>(contextPop(pc, *len));
                pc->pJson = pCur;
                return State::ok;
            case '\\':
                switch(*pCur++)
                {
                    case '\"': PUSHC(pc, '\"'); break;
                    case '\\': PUSHC(pc, '\\'); break;
                    case '/':  PUSHC(pc, '/');  break;
                    case 'b':  PUSHC(pc, '\b'); break;
                    case 'f':  PUSHC(pc, '\f'); break;
                    case 'n':  PUSHC(pc, '\n'); break;
                    case 'r':  PUSHC(pc, '\r'); break;
                    case 't':  PUSHC(pc, '\t'); break;
                    case 'u':
                        if(!(pCur = parseHex4(pCur, &u)))
                            STRING_ERROR(State::invalid_unicode_hex);
                        if(u >= 0xD800 && u <= 0xDBFF)
                        {
                            unsigned u2 = 0;
                            if(*pCur++ != '\\')
                                STRING_ERROR(State::invalid_unicode_surrogate);
                            if(*pCur++ != 'u');
                            STRING_ERROR(State::invalid_unicode_surrogate);
                            if(!(pCur = parseHex4(pCur, &u2)))
                                STRING_ERROR(State::invalid_unicode_hex);
                            if(u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(State::invalid_unicode_surrogate);
                            u = 0x10000 + (((u - 0xD800) << 10) | (u2 - 0xDC00));
                        }
                        encodeUTF8(pc, u);
                        break;
                    default:
                        pc->top = head;
                        return State::invalid_string_escape;
                }
                break;
            case '\0':
                pc->top = head;
                return State::miss_quotation_mark;
            default:
                if(static_cast<unsigned char>(ch) < 0x20)
                {
                    pc->top = head;
                    return State::invalid_string_escape;
                }
                PUSHC(pc, ch);
        }
    }
}

wt::json::State wt::json::parseString(wt::json::Context *pc, wt::json::Json_value *pv)
{
    State ret;
    char* str;
    size_t len;
    if((ret = parseStringRaw(pc, &str, &len)) == State::ok)
        setString(pv, str, len);
    return ret;
}

wt::json::State  wt::json::parseArray(Context *pc, Json_value *pv)
{
    size_t size = 0;
    State ret;
    ++pc->pJson;
    parseWhiteSpace(pc);
    if(*pc->pJson == ']')
    {
        ++pc->pJson;
        pv->asize = 0;
        pv->pa = nullptr;
        pv->type = Json_t::json_array;
        return State::ok;
    }
    for(;;)
    {
        Json_value tmp;
        initJson(&tmp);
        if((ret = parseValue(pc, &tmp)) != State::ok)
            return ret;
        memcpy(contextPush(pc, sizeof(Json_value)), &tmp, sizeof(Json_value));
        ++size;
        parseWhiteSpace(pc);
        if(*pc->pJson == ',')
        {
            ++pc->pJson;
            parseWhiteSpace(pc);
        }
        else if(*pc->pJson == ']')
        {
            ++pc->pJson;
            pv->asize = size;
            pv->type = Json_t::json_array;
            size *= sizeof(Json_value);
            memcpy(pv->pa = (Json_value*)malloc(size), contextPop(pc, size), size);
            return State::ok;
        }
        else
        {
            ret = State::miss_comma_or_square_bracket;
            break;
        }
    }
    for(int i = 0; i < size; ++i)
        freeJson((Json_value*)contextPop(pc, sizeof(Json_value)));
    pv->type = Json_t::json_null;
    return ret;
}

wt::json::State wt::json::parseObject(Context *pc, Json_value *pv)
{
    size_t size = 0;
    State  ret;
    Json_member temp;
    ++pc->pJson;
    parseWhiteSpace(pc);
    if(*pc->pJson == '}')
    {
        ++pc->pJson;
        pv->osize = 0;
        pv->po = nullptr;
        pv->type = Json_t::json_object;
        return State::ok;
    }
    temp.pk = nullptr;
    for(;;)
    {
        char* str;
        initJson(&temp.val);
        if(*pc->pJson != '"')
        {
            ret = State::miss_key;
            break;
        }
        if((ret = parseStringRaw(pc, &str, &temp.klen)) != State::ok)
            break;
        temp.pk = static_cast<char*>(malloc(temp.klen + 1));
        memcpy(temp.pk, str, temp.klen);
        temp.pk[temp.klen] = '\0';
        parseWhiteSpace(pc);
        if(*pc->pJson != ':')
        {
            ret = State::miss_colon;
            break;
        }
        ++pc->pJson;
        parseWhiteSpace(pc);
        if((ret = parseValue(pc, &temp.val)) != State::ok)
            break;
        memcpy(contextPush(pc, sizeof(Json_member)), &temp, sizeof(Json_member));
        temp.pk = nullptr;
        ++size;
        parseWhiteSpace(pc);
        if(*pc->pJson == ',')
        {
            ++pc->pJson;
            parseWhiteSpace(pc);
        }
        else if(*pc->pJson == '}')
        {
            ++pc->pJson;
            pv->osize = size;
            pv->type = Json_t::json_object;
            size_t cap = size * sizeof(Json_member);
            pv->po = static_cast<Json_member*>(malloc(cap));
            memcpy(pv->po, contextPop(pc, cap), cap);
            return State::ok;
        }
        else
        {
            ret= State::miss_comma_or_curly_bracket;
            break;
        }
    }
    free(temp.pk);
    for(size_t i =0; i < size; ++i)
    {
        Json_member* m = static_cast<Json_member*>(contextPop(pc, sizeof(Json_member)));
        free(m->pk);
        freeJson(&m->val);
    }
    pv->type = Json_t::json_null;
    return ret;
}

wt::json::Json_t wt::json::getType(const wt::json::Json_value &v)
{
    return v.type;
}

void wt::json::setNumber(wt::json::Json_value *pv, double val)
{
    freeJson(pv);
    pv->type = Json_t::json_number;
    pv->number = val;
}

double wt::json::getNumber(const wt::json::Json_value &v)
{
    assert(v.type == Json_t::json_number);
    return v.number;
}

void wt::json::setString(wt::json::Json_value *pv, const char* str, std::size_t len)
{
    assert(pv != nullptr);
    freeJson(pv);
    pv->pstr = static_cast<char*>(std::malloc(len + 1));
    std::memcpy(pv->pstr, str, len);
    pv->pstr[len] = '\0';
    pv->len = len;
    pv->type = Json_t::json_string;
}

void wt::json::freeJson(wt::json::Json_value *pv)
{
    assert(pv != nullptr);
    switch(pv->type)
    {
        case Json_t::json_string:
            free(pv->pstr);
            break;
        case Json_t::json_array:
            for(size_t i = 0; i < pv->asize; ++i)
                freeJson(&pv->pa[i]);
            free(pv->pa);
            break;
        case Json_t::json_object:
            for(size_t i = 0; i < pv->osize; ++i)
            {
                free(pv->po[i].pk);
                freeJson(&pv->po[i].val);
            }
            free(pv->po);
            break;
        default:
            break;
    }
    pv->type = Json_t::json_null;
}

const char* wt::json::getString(const wt::json::Json_value &v)
{
    return v.pstr;
}

std::size_t wt::json::getStringLength(const wt::json::Json_value &v)
{
    return v.len;
}

void* wt::json::contextPush(wt::json::Context *pc, std::size_t size)
{
    assert(size > 0);
    if(pc->top + size >= pc->size)
    {
        if(pc->size == 0)
            pc->size = PARSE_STACK_SIZE;
        while(pc->top + size >= pc->size)
            pc->size += pc->size >> 1;
        pc->pStack = static_cast<char*>(realloc(pc->pStack, pc->size));
    }
    void* ret = pc->pStack + pc->top;
    pc->top += size;
    return ret;
}

void* wt::json::contextPop(wt::json::Context *pc, std::size_t size)
{
    assert(pc->top >= size);
    pc->top -= size;
    return pc->pStack + pc->top;
}

void wt::json::setBool(wt::json::Json_value *pv, bool flag)
{
    freeJson(pv);
    pv->type = flag ? Json_t::json_true : Json_t::json_false;
}

bool wt::json::getBool(const wt::json::Json_value &v)
{
    assert(v.type == Json_t::json_true || v.type == Json_t::json_false);
    return v.type == Json_t::json_true;
}

const char* wt::json::parseHex4(const char *p, unsigned *u)
{
    *u = 0;
    for(int i = 0; i < 4; ++i)
    {
        char ch = *p++;
        *u <<= 4;
        if(ch >= '0' && ch <= '9')
            *u |= ch - '0';
        else if(ch >= 'A' && ch <= 'F')
            *u |= ch - 'A' + 10;
        else if(ch >= 'a' && ch <= 'f')
            *u |= ch - 'a' + 10;
        else
            return nullptr;
    }
    return p;
}

void wt::json::encodeUTF8(wt::json::Context *pc, unsigned u)
{
    assert(u <= 0x10FFFF);
    if(u <= 0x7F)
    {
        PUSHC(pc, u & 0xFF);
    }
    else if(u <= 0x7FF)
    {
        PUSHC(pc, 0xC0 | ((u >> 6) & 0x1F));
        PUSHC(pc, 0x80 | (u & 0x3F));
    }
    else if(u <= 0xFFFF)
    {
        PUSHC(pc, 0xE0 | ((u >> 12) & 0x0F));
        PUSHC(pc, 0x80 | ((u >> 6) & 0x3F));
        PUSHC(pc, 0x80 | (u & 0x3F));
    }
    else
    {
        PUSHC(pc, 0xF0 | ((u >> 18) & 0x07));
        PUSHC(pc, 0x80 | ((u >> 12) & 0x3F));
        PUSHC(pc, 0x80 | ((u >> 6) & 0x3F));
        PUSHC(pc, 0x80 | (u & 0x3F));
    }
}

size_t wt::json::getArraySize(const wt::json::Json_value &pv)
{
    assert(pv.type == Json_t::json_array);
    return pv.asize;
}

wt::json::Json_value* wt::json::getArrayElem(const wt::json::Json_value &pv, size_t index)
{
    assert(pv.type == Json_t::json_array && index < pv.asize);
    return &pv.pa[index];
}

size_t wt::json::getObjectsize(const wt::json::Json_value &pv)
{
    assert(pv.type == Json_t::json_object);
    return pv.osize;
}

const char* wt::json::getObjectKey(const wt::json::Json_value &pv, size_t index)
{
    assert(pv.type == Json_t::json_object && index < pv.osize);
    return pv.po[index].pk;
}

size_t wt::json::getObjectKeyLen(const wt::json::Json_value &pv, size_t index)
{
    assert(pv.type == Json_t::json_object && index < pv.osize);
    return pv.po[index].klen;
}

wt::json::Json_value* wt::json::getObjectValue(const wt::json::Json_value &pv, size_t index)
{
    assert(pv.type == Json_t::json_object && index < pv.osize);
    return &pv.po[index].val;
}
