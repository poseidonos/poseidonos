
#ifndef AIR_JSONDOC_H
#define AIR_JSONDOC_H

#include <initializer_list>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <typeinfo>

#include "src/lib/json/Jsonprint.h"
#include "src/lib/json/Jsontype.h"

namespace air
{
class JSONdoc
{
public:
    JSONdoc(JSONtype new_type = JSONtype::WHITESPACE, bool new_ownership = false,
        bool new_key = false)
    : type(new_type),
      ownership(new_ownership),
      key(new_key)
    {
    }

    template<typename T>
    void
    operator=(std::initializer_list<T> t)
    {
        if (false == key)
        {
            throw std::logic_error(
                "[error][JSONdoc::operator=<T>] invalid syntax: no key");
        }
        Clear();
        is_cleared = true;
        operator+=(t);
    }

    void
    operator=(void* empty)
    {
        Clear();
    }

    template<typename T>
    void
    operator+=(std::initializer_list<T> t)
    {
        if (false == key)
        {
            throw std::logic_error(
                "[error][JSONdoc::operator+=<T>] invalid syntax: no key");
        }
        auto it = t.begin();
        size_t map_key = map.size();
        while (it != t.end())
        {
            std::string value_type(typeid(*it).name());

            T* dt = new T{*it};

            type = cpp_type(std::type_index(typeid(*dt)));

            if (JSONtype::NULLVAL == type)
            {
                map.insert({std::to_string(map_key), {nullptr, type}});
                delete dt;
            }
            else if (JSONtype::UNDEFINED == type)
            {
                throw std::logic_error(
                    "[error][JSONdoc::operator+=<T>] deduction failed");
            }
            else
            {
                map.insert({std::to_string(map_key), {dt, type}});
            }

            ++it;
            map_key++;
        }
        if (2 <= map.size() || (false == is_cleared))
        {
            type = JSONtype::ARRAY;
        }
        is_cleared = false;
    }

    void
    operator+=(std::initializer_list<const char*> t)
    {
        if (false == key)
        {
            throw std::logic_error(
                "[error][JSONdoc::operator+=<const char*>] invalid syntax: no key");
        }
        auto it = t.begin();
        size_t map_key = map.size();
        while (it != t.end())
        {
            std::string value_type(typeid(*it).name());

            if (0 == value_type.compare("PKc"))
            {
                type = JSONtype::CHARP;
                std::string* str_value = new std::string{*it};
                map.insert({std::to_string(map_key), {str_value, JSONtype::CHARP}});
            }
            ++it;
            map_key++;
        }
        if (2 <= map.size() || (false == is_cleared))
        {
            type = JSONtype::ARRAY;
        }
        is_cleared = false;
    }

    void
    operator+=(std::initializer_list<JSONdoc> t)
    {
        if (false == key)
        {
            throw std::logic_error(
                "[error][JSONdoc::operator+=<JSONdoc>] invalid syntax: no key");
        }
        if (1 >= map.size() + t.size() && is_cleared)
        {
            type = (*(t.begin())).type;
            map = (*(t.begin())).map;
            ownership = false;
        }
        else
        {
            type = JSONtype::ARRAY;
            auto it = t.begin();
            size_t map_key = map.size();
            while (it != t.end())
            {
                JSONdoc* doc = new JSONdoc{};
                doc->type = (*it).type; // JSONtype::OBJECT;
                doc->map = (*it).map;
                doc->ownership = false;
                map.insert({std::to_string(map_key), {doc, JSONtype::OBJECT}});
                ++it;
                map_key++;
            }
        }
        is_cleared = false;
    }

    JSONdoc& operator[](std::string key)
    {
        auto it = map.find(key);
        if (it != map.end())
        { // found
            void* v = (it->second).data;
            JSONdoc* doc = reinterpret_cast<JSONdoc*>(v);
            return *doc;
        }
        else
        { // not found
            JSONdoc* doc = new JSONdoc{JSONtype::OBJECT, true, true};
            map.insert({key, {doc, JSONtype::OBJECT}});
            return *doc;
        }
    }

    friend std::ostream&
    operator<<(std::ostream& os, const JSONdoc& doc)
    {
        switch (doc.type)
        {
            case JSONtype::OBJECT:
            {
                os << "{";
                size_t obj_count = 0;
                for (auto const& obj : doc.map)
                {
                    void* v = obj.second.data;
                    JSONdoc* child_doc = reinterpret_cast<JSONdoc*>(v);
                    os << "\"" << obj.first << "\": " << *child_doc;
                    obj_count++;
                    if (obj_count == doc.map.size())
                    {
                        break;
                    }
                    os << ", ";
                }
                os << "}";
                break;
            }
            case JSONtype::ARRAY:
            {
                os << "[";
                size_t value_count = 0;
                for (auto const& obj : doc.map)
                {
                    if (JSONtype::OBJECT == obj.second.type)
                    {
                        void* v = obj.second.data;
                        JSONdoc* child_doc = reinterpret_cast<JSONdoc*>(v);
                        os << *child_doc;
                    }
                    else
                    {
                        PrintValue(os, obj.second.type, obj.second.data);
                    }
                    value_count++;
                    if (doc.map.size() == value_count)
                    {
                        break;
                    }
                    os << ", ";
                }
                os << "]";
                break;
            }
            case JSONtype::WHITESPACE:
            {
                os << " ";
                break;
            }
            case JSONtype::UNDEFINED:
            {
                throw std::logic_error("[error][JSONdoc::operator<<] Invalid JSONtype");
                break;
            }
            default:
            {
                auto value = doc.map.begin();
                PrintValue(os, (*value).second.type, (*value).second.data);
                break;
            }
        }
        return os;
    }

    bool
    HasKey(std::string key)
    {
        auto it = map.find(key);
        if (it != map.end())
        {
            return true;
        }
        return false;
    }

    void
    Clear(void)
    {
        for (const auto& value : map)
        {
            DeleteValue(value.second.type, value.second.data);
        }
        map.clear();
    }

    void
    DeleteValue(JSONtype type, void* v)
    {
        switch (type)
        {
            case JSONtype::OBJECT:
            {
                JSONdoc* doc = reinterpret_cast<JSONdoc*>(v);
                if (doc->ownership)
                {
                    doc->Clear();
                }
                delete doc;
                break;
            }
            case JSONtype::LONGLONG:
            {
                long long* value = reinterpret_cast<long long*>(v);
                delete value;
                break;
            }
            case JSONtype::ULONGLONG:
            {
                unsigned long long* value = reinterpret_cast<unsigned long long*>(v);
                delete value;
                break;
            }
            case JSONtype::INT16:
            {
                int16_t* value = reinterpret_cast<int16_t*>(v);
                delete value;
                break;
            }
            case JSONtype::INT32:
            {
                int32_t* value = reinterpret_cast<int32_t*>(v);
                delete value;
                break;
            }
            case JSONtype::INT64:
            {
                int64_t* value = reinterpret_cast<int64_t*>(v);
                delete value;
                break;
            }
            case JSONtype::UINT16:
            {
                uint16_t* value = reinterpret_cast<uint16_t*>(v);
                delete value;
                break;
            }
            case JSONtype::UINT32:
            {
                uint32_t* value = reinterpret_cast<uint32_t*>(v);
                delete value;
                break;
            }
            case JSONtype::UINT64:
            {
                uint64_t* value = reinterpret_cast<uint64_t*>(v);
                delete value;
                break;
            }
            case JSONtype::FLOAT:
            {
                float* value = reinterpret_cast<float*>(v);
                delete value;
                break;
            }
            case JSONtype::DOUBLE:
            {
                double* value = reinterpret_cast<double*>(v);
                delete value;
                break;
            }
            case JSONtype::STRING:
            {
                std::string* value = reinterpret_cast<std::string*>(v);
                delete value;
                break;
            }
            case JSONtype::CHARP:
            {
                std::string* value = reinterpret_cast<std::string*>(v);
                delete value;
                break;
            }
            case JSONtype::BOOL:
            {
                bool* value = reinterpret_cast<bool*>(v);
                delete value;
                break;
            }
            case JSONtype::NULLVAL:
            {
                break;
            }
            default:
            {
                throw std::logic_error("[error][JSONdoc::clear] Invalid JSONtype");
                break;
            }
        }
    }

    bool
    Ownership(void)
    {
        return ownership;
    }
    void
    SetType(JSONtype new_type)
    {
        type = new_type;
    }

    friend class RangeImpl;
    friend class RangeIterator;

private:
    JSONdoc& operator=(const JSONdoc&) = delete;

    JSONtype type{JSONtype::WHITESPACE};
    std::map<std::string, JSONvalue> map;
    bool ownership{false};
    bool key{false};
    bool is_cleared{false};
};

} // namespace air

#endif // AIR_JSONDOC_H
