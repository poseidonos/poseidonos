
#ifndef AIR_JSONPRINT_H
#define AIR_JSONPRINT_H

#include <iostream>
#include <string>

#include "src/lib/json/Jsontype.h"

namespace air
{
static std::ostream&
PrintValue(std::ostream& os, JSONtype type, void* value)
{
    switch (type)
    {
        case JSONtype::LONGLONG:
        {
            os << *((long long*)value);
            break;
        }
        case JSONtype::ULONGLONG:
        {
            os << *((unsigned long long*)value);
            break;
        }
        case JSONtype::INT16:
        {
            os << *((int16_t*)value);
            break;
        }
        case JSONtype::INT32:
        {
            os << *((int32_t*)value);
            break;
        }
        case JSONtype::INT64:
        {
            os << *((int64_t*)value);
            break;
        }
        case JSONtype::UINT16:
        {
            os << *((uint16_t*)value);
            break;
        }
        case JSONtype::UINT32:
        {
            os << *((uint32_t*)value);
            break;
        }
        case JSONtype::UINT64:
        {
            os << *((uint64_t*)value);
            break;
        }
        case JSONtype::FLOAT:
        {
            os << *((float*)value);
            break;
        }
        case JSONtype::DOUBLE:
        {
            os << *((double*)value);
            break;
        }
        case JSONtype::STRING:
        case JSONtype::CHARP:
        {
            os << "\"" << *((std::string*)value) << "\"";
            break;
        }
        case JSONtype::BOOL:
        {
            if (*((bool*)value))
            {
                os << "true";
            }
            else
            {
                os << "false";
            }
            break;
        }
        case JSONtype::NULLVAL:
        {
            os << "null";
            break;
        }
        default:
        {
            throw std::logic_error("[error][JSONdoc::Print] invalid type");
            break;
        }
    }
    return os;
}

} // namespace air

#endif // AIR_JSONPRINT_H
