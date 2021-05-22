
#ifndef AIR_CONFIG_LIB_H
#define AIR_CONFIG_LIB_H

#include <cstdint>
#include <type_traits>

#include "src/lib/StringView.h"

template<typename E>
constexpr auto
dtype(E e) noexcept // to primitive data types
{
    return static_cast<std::underlying_type_t<E>>(e);
}

namespace config
{
static constexpr uint32_t NUM_DEFAULT_KEY{6};
static constexpr uint32_t NUM_GROUP_KEY{5};
static constexpr uint32_t NUM_FILTER_KEY{2};
static constexpr uint32_t NUM_NODE_KEY{8};

enum class ParagraphType : uint32_t
{
    DEFAULT = 0,
    GROUP,
    FILTER,
    NODE,

    COUNT,
    CONFIGTYPE_NULL
};

static constexpr bool
HasSpace(air::string_view value)
{
    size_t pos{0};

    if (0 == value.size())
    {
        return false;
    }

    while (pos < value.size())
    {
        if (' ' == value[pos] || '\t' == value[pos] || '\n' == value[pos] ||
            '\v' == value[pos] || '\f' == value[pos] || '\r' == value[pos])
        {
            return true;
        }
        pos++;
    }

    return false;
}

static constexpr bool
IsSpace(char c)
{
    if (' ' == c || '\t' == c || '\n' == c || '\v' == c || '\f' == c || '\r' == c)
    {
        return true;
    }

    return false;
}

static constexpr bool
IsUInt(air::string_view value)
{
    size_t pos{0};

    if (0 == value.size())
    {
        return false;
    }

    while (pos < value.size())
    {
        if ('0' > value[pos] || '9' < value[pos])
        {
            return false;
        }
        pos++;
    }
    return true;
}

static constexpr air::string_view
Strip(air::string_view str)
{
    if (0 != str.size())
    {
        std::size_t tail = str.size() - 1;

        if (IsSpace(str[0]))
        {
            return Strip(str.substr(1, str.size() - 1));
        }
        else if (IsSpace(str[tail]))
        {
            return Strip(str.substr(0, str.size() - 1));
        }
    }
    return str;
}

static constexpr uint32_t
Stoi(air::string_view str)
{
    uint32_t length = str.size();
    uint32_t int_value = 0;
    for (uint32_t idx_length = 0; idx_length < length; idx_length++)
    {
        uint32_t num = str[idx_length] - '0';
        uint32_t pow = length - idx_length - 1;
        uint32_t pow_of_ten = 1;
        if (0 < pow)
        {
            for (uint32_t idx_pow = 0; idx_pow < pow; idx_pow++)
            {
                pow_of_ten *= 10;
            }
        }
        int_value += num * pow_of_ten;
    }
    return int_value;
}

} // namespace config

#endif // AIR_CONFIG_LIB_H
