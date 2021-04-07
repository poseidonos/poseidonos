
#ifndef AIR_JSONPARSER_H
#define AIR_JSONPARSER_H

#include <iostream>
#include <stdexcept>
#include <string>

#include "src/lib/json/Jsoner.h"

namespace air
{
class JSONparser
{
public:
    static JSONdoc&
    Parse(std::string doc_name, std::string raw_json)
    {
        return GetInstance()._ParseObject(JSONer::JSON(doc_name), doc_name,
            raw_json);
    }
    static JSONparser&
    GetInstance(void)
    {
        static JSONparser instance;
        return instance;
    }

protected:
    void
    _AddJson(JSONdoc& doc, std::string doc_name, JSONtype type,
        std::string key, std::string value)
    {
        switch (type)
        {
            case JSONtype::OBJECT:
            {
                std::string new_key{doc_name + "_" + key};
                auto& obj = JSONer::JSON(new_key);
                _ParseObject(obj, new_key, value);
                doc[key] = {obj};
                break;
            }
            case JSONtype::ARRAY:
            {
                _ParseArray(doc, doc_name, key, value);
                break;
            }
            case JSONtype::STRING:
            {
                doc[key] = {value};
                break;
            }
            case JSONtype::NULLVAL:
            {
                doc[key] = {nullptr};
                break;
            }
            case JSONtype::BOOL:
            {
                if (std::string::npos != value.find("true"))
                {
                    doc[key] = {true};
                }
                else if (std::string::npos != value.find("false"))
                {
                    doc[key] = {false};
                }
                break;
            }
            case JSONtype::DOUBLE:
            {
                doc[key] = {std::stod(value)};
                break;
            }
            case JSONtype::INT64:
            {
                doc[key] = {std::stol(value)};
                break;
            }
            case JSONtype::UINT64:
            {
                doc[key] = {std::stoul(value)};
                break;
            }
            default:
            {
                throw std::logic_error("[error][JSONparser::AddJson] invalid type");
                break;
            }
        }
    }
    void
    _AddJsonArray(JSONdoc& doc, std::string doc_name, std::size_t value_count,
        JSONtype type, std::string key, std::string value)
    {
        switch (type)
        {
            case JSONtype::OBJECT:
            {
                std::string new_key{doc_name + "_" + key + "_" +
                    std::to_string(value_count)};
                auto& obj = JSONer::JSON(new_key);
                doc[key] += {_ParseObject(obj, new_key, value)};
                break;
            }
            case JSONtype::ARRAY:
            {
                _ParseArray(doc[key], doc_name, std::to_string(value_count), value);
                doc[key].SetType(JSONtype::ARRAY);
                break;
            }
            case JSONtype::STRING:
            {
                doc[key] += {value};
                break;
            }
            case JSONtype::NULLVAL:
            {
                doc[key] += {nullptr};
                break;
            }
            case JSONtype::BOOL:
            {
                if (std::string::npos != value.find("true"))
                {
                    doc[key] += {true};
                }
                else if (std::string::npos != value.find("false"))
                {
                    doc[key] += {false};
                }
                break;
            }
            case JSONtype::DOUBLE:
            {
                doc[key] += {std::stod(value)};
                break;
            }
            case JSONtype::INT64:
            {
                doc[key] += {std::stol(value)};
                break;
            }
            case JSONtype::UINT64:
            {
                doc[key] += {std::stoul(value)};
                break;
            }
            default:
            {
                throw std::logic_error("[error][JSONparser::_AddJsonArray] invalid type");
                break;
            }
        }
    }

private:
    JSONparser(void)
    {
    }
    ~JSONparser(void)
    {
    }
    JSONparser(const JSONparser&) = delete;
    JSONparser& operator=(const JSONparser&) = delete;

    JSONtype
    _GetType(std::string value)
    {
        JSONtype type{JSONtype::UNDEFINED};

        if (std::string::npos != value.find("null"))
        {
            type = JSONtype::NULLVAL;
        }
        else if (std::string::npos != value.find("true"))
        {
            type = JSONtype::BOOL;
        }
        else if (std::string::npos != value.find("false"))
        {
            type = JSONtype::BOOL;
        }
        else if (std::string::npos != value.find('.'))
        {
            type = JSONtype::DOUBLE;
        }
        else if (std::string::npos != value.find('-'))
        {
            type = JSONtype::INT64;
        }
        else
        {
            type = JSONtype::UINT64;
        }

        return type;
    }

    void
    _CheckStringOrOthers(std::string& value, JSONtype& type)
    {
        if (std::string::npos != value.find('"'))
        {
            type = JSONtype::STRING;
            std::size_t start_quote, end_quote;
            start_quote = value.find('"') + 1;
            end_quote = value.find('"', start_quote);
            value = value.substr(start_quote, end_quote - start_quote);
        }
        else
        {
            type = _GetType(value);
        }
    }

    JSONdoc&
    _ParseObject(JSONdoc& doc, std::string doc_name, std::string raw_json)
    {
        std::string key;
        std::string value;

        std::size_t json_length{raw_json.length()};
        std::size_t idx{0};

        bool has_key{false};
        bool has_key_start_idx{false};
        std::size_t key_start_idx{0};

        bool has_value{false};
        bool has_value_start_idx{false};
        std::size_t value_start_idx{0};
        std::size_t obj_bracket_count{0};
        std::size_t arr_bracket_count{0};
        JSONtype type{JSONtype::UNDEFINED};

        while (idx < json_length)
        {
            // step 1. key finding
            if ('"' == raw_json.at(idx))
            {
                if (false == has_key)
                {
                    if (false == has_key_start_idx)
                    {
                        key_start_idx = idx + 1;
                        has_key_start_idx = true;
                    }
                    else
                    {
                        key = raw_json.substr(key_start_idx, idx - key_start_idx);
                        has_key = true;
                    }
                }
            }

            // step 2. value finding
            // step 2.1. detect object
            // step 2.2. detect array
            // step 2.3. detect rest of all(number, null, bool, string)
            if (true == has_key && false == has_value_start_idx &&
                ':' == raw_json.at(idx))
            {
                value_start_idx = idx + 1;
                has_value_start_idx = true;
            }

            if (true == has_key && true == has_value_start_idx)
            {
                if (JSONtype::UNDEFINED == type)
                {
                    if ('{' == raw_json.at(idx))
                    {
                        type = JSONtype::OBJECT;
                        obj_bracket_count++;
                    }
                    else if ('[' == raw_json.at(idx))
                    {
                        type = JSONtype::ARRAY;
                        arr_bracket_count++;
                    }
                    else
                    {
                        // rest of all (null, number, bool, string)
                        if (',' == raw_json.at(idx) || '}' == raw_json.at(idx))
                        {
                            value = raw_json.substr(value_start_idx, idx - value_start_idx);
                            has_value = true;

                            _CheckStringOrOthers(value, type);
                        }
                    }
                }
                else if (JSONtype::OBJECT == type)
                {
                    if ('{' == raw_json.at(idx))
                    {
                        obj_bracket_count++;
                    }
                    else if ('}' == raw_json.at(idx))
                    {
                        obj_bracket_count--;
                        if (0 == obj_bracket_count)
                        {
                            value = raw_json.substr(value_start_idx, idx - value_start_idx + 1);
                            has_value = true;
                        }
                    }
                }
                else if (JSONtype::ARRAY == type)
                {
                    if ('[' == raw_json.at(idx))
                    {
                        arr_bracket_count++;
                    }
                    else if (']' == raw_json.at(idx))
                    {
                        arr_bracket_count--;
                        if (0 == arr_bracket_count)
                        {
                            value = raw_json.substr(value_start_idx, idx - value_start_idx + 1);
                            has_value = true;
                        }
                    }
                }
            }

            // step 3. [key:value] set
            // step 2.1. if object, recursive call
            // step 2.2. if array, _ParseArray call
            // step 2.3. set value with proper type casting(string to ...)
            if (has_key && has_value)
            {
                // std::cout << key << " | " << value << std::endl;
                _AddJson(doc, doc_name, type, key, value);
                // clear
                has_key = false;
                has_key_start_idx = false;
                key_start_idx = 0;
                has_value = false;
                has_value_start_idx = false;
                value_start_idx = 0;
                obj_bracket_count = 0;
                arr_bracket_count = 0;
                type = JSONtype::UNDEFINED;
            }
            idx++;
        }
        return doc;
    }

    void
    _ParseArray(JSONdoc& doc, std::string doc_name, std::string key, std::string arr)
    {
        // std::cout << "arr +key+: " << key
        //<< ", +value+: " << arr << std::endl;
        std::size_t arr_length{arr.length()};
        std::size_t idx{0};
        std::size_t value_count{0};

        std::size_t value_start_idx{0};
        std::size_t obj_bracket_count{0};
        std::size_t arr_bracket_count{0};
        JSONtype type{JSONtype::UNDEFINED};
        bool has_value{false};
        std::string value;

        bool first_bracket_skipped{false};

        while (idx < arr_length)
        {
            // step 1. value seperation & detect type
            if ('[' == arr.at(idx))
            { // at the beginning
                if (false == first_bracket_skipped)
                {
                    idx++;
                    value_start_idx = idx;
                    first_bracket_skipped = true;
                    continue;
                }
                if (JSONtype::UNDEFINED == type || JSONtype::ARRAY == type)
                {
                    type = JSONtype::ARRAY;
                    if (0 == arr_bracket_count)
                    {
                        value_start_idx = idx;
                    }
                    idx++;
                    arr_bracket_count++;
                    continue;
                }
            }

            if (']' == arr.at(idx) && JSONtype::ARRAY == type)
            {
                arr_bracket_count--;
                if (0 == arr_bracket_count)
                {
                    value = arr.substr(value_start_idx, idx - value_start_idx + 1);
                    value_start_idx = idx + 1;
                    has_value = true;
                }
            }

            if ('{' == arr.at(idx))
            {
                if (JSONtype::UNDEFINED == type || JSONtype::OBJECT == type)
                {
                    type = JSONtype::OBJECT;
                    if (0 == obj_bracket_count)
                    {
                        value_start_idx = idx;
                    }
                    idx++;
                    obj_bracket_count++;
                    continue;
                }
            }

            if ('}' == arr.at(idx) && JSONtype::OBJECT == type)
            {
                obj_bracket_count--;
                if (0 == obj_bracket_count)
                {
                    value = arr.substr(value_start_idx, idx - value_start_idx + 1);
                    value_start_idx = idx + 1;
                    has_value = true;
                }
            }

            if ((',' == arr.at(idx) || ']' == arr.at(idx)) &&
                JSONtype::UNDEFINED == type)
            {
                value = arr.substr(value_start_idx, idx - value_start_idx);
                // std::cout << "size:" << value.size() << std::endl;
                if (0 == value.size())
                {
                    idx++;
                    value_start_idx = idx + 1;
                    continue;
                }

                value_start_idx = idx + 1;
                has_value = true;

                _CheckStringOrOthers(value, type);
            }

            // step 2. append value via type
            if (has_value)
            {
                // std::cout << key << (uint32_t)type << "parsing val: " << value <<
                // std::endl;
                _AddJsonArray(doc, doc_name, value_count, type, key, value);

                value_count++;

                obj_bracket_count = 0;
                arr_bracket_count = 0;
                type = JSONtype::UNDEFINED;
                has_value = false;
            }

            idx++;
        }
    }
};

constexpr auto parse = JSONparser::Parse;

} // namespace air

#endif // AIR_JSONPARSER_H
