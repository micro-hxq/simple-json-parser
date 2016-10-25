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
    ::free(c.pStack);
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

wt::json::State wt::json::parseString(wt::json::Context *pc, wt::json::Json_value *pv)
{
    std::size_t head = pc->top, len;
    const char* pCur = pc->pJson;
    assert(*pCur == '\"');
    ++pCur;
    for(;;)
    {
        char ch = *pCur++;
        switch(ch)
        {
            case '\"':
                len = pc->top - head;
                setString(pv, static_cast<const char*>(contextPop(pc, len)), len);
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
    if(pv->type == Json_t::json_string)
        std::free(pv->pstr);
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
        pc->pStack = static_cast<char*>(::realloc(pc->pStack, pc->size));
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
