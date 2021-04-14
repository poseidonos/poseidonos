
#ifndef AIR_JSONTYPE_H
#define AIR_JSONTYPE_H

#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace air
{
enum class JSONtype : uint32_t
{
    OBJECT, // 0
    ARRAY,
    SHORT,
    INT,
    LONG,
    LONGLONG, // 5
    USHORT,
    UINT,
    ULONG,
    ULONGLONG,
    INT16, // 10
    INT32,
    INT64,
    UINT16,
    UINT32,
    UINT64, // 15
    FLOAT,
    DOUBLE,
    STRING,
    CHARP,
    BOOL, // 20
    NULLVAL,
    WHITESPACE,
    UNDEFINED,
};

struct JSONvalue
{
    void* data{nullptr};
    JSONtype type{JSONtype::WHITESPACE};
};

class CPPtype
{
public:
    static JSONtype
    Type(std::type_index index)
    {
        return _GetInstance()._GetType(index); // types[index];
    }

private:
    CPPtype(void)
    {
        types.clear();
        types[std::type_index(typeid(short))] = JSONtype::SHORT;
        types[std::type_index(typeid(int))] = JSONtype::INT;
        types[std::type_index(typeid(long))] = JSONtype::LONG;
        types[std::type_index(typeid(long long))] = JSONtype::LONGLONG;
        types[std::type_index(typeid(unsigned short))] = JSONtype::USHORT;
        types[std::type_index(typeid(unsigned int))] = JSONtype::UINT;
        types[std::type_index(typeid(unsigned long))] = JSONtype::ULONG;
        types[std::type_index(typeid(unsigned long long))] = JSONtype::ULONGLONG;
        types[std::type_index(typeid(int16_t))] = JSONtype::INT16;
        types[std::type_index(typeid(int32_t))] = JSONtype::INT32;
        types[std::type_index(typeid(int64_t))] = JSONtype::INT64;
        types[std::type_index(typeid(uint16_t))] = JSONtype::UINT16;
        types[std::type_index(typeid(uint32_t))] = JSONtype::UINT32;
        types[std::type_index(typeid(uint64_t))] = JSONtype::UINT64;
        types[std::type_index(typeid(float))] = JSONtype::FLOAT;
        types[std::type_index(typeid(double))] = JSONtype::DOUBLE;
        types[std::type_index(typeid(std::string))] = JSONtype::STRING;
        types[std::type_index(typeid(const char*))] = JSONtype::CHARP;
        types[std::type_index(typeid(bool))] = JSONtype::BOOL;
        types[std::type_index(typeid(nullptr))] = JSONtype::NULLVAL;
    }
    ~CPPtype(void)
    {
        types.clear();
    }
    CPPtype(const CPPtype&) = delete;
    CPPtype& operator=(const CPPtype&) = delete;

    static CPPtype&
    _GetInstance(void)
    {
        static CPPtype instance;
        return instance;
    }

    JSONtype
    _GetType(std::type_index index)
    {
        auto search = types.find(index);
        if (types.end() != search)
        {
            return search->second;
        }
        else
        {
            return JSONtype::UNDEFINED;
        }
    }

    std::unordered_map<std::type_index, JSONtype> types;
};

constexpr auto cpp_type = CPPtype::Type;

} // namespace air

#endif // AIR_JSONTYPE_H
