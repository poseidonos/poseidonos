
#ifndef AIR_CONFIG_LIB_H
#define AIR_CONFIG_LIB_H

#include <cstdint>
#include <string_view>
#include <type_traits>

template<typename E>
constexpr auto
dtype(E e) noexcept // to primitive data types
{
    return static_cast<std::underlying_type_t<E>>(e);
}

namespace config
{
enum class ConfigType : uint32_t
{
    DEFAULT = 0,
    GROUP,
    NODE,

    COUNT,
    CONFIGTYPE_NULL
};

static constexpr std::string_view
Strip(std::string_view str)
{
    if (0 != str.size())
    {
        std::size_t tail = str.size() - 1;

        if (' ' == str[0] || '\t' == str[0] || '\n' == str[0] || '\v' == str[0] || '\f' == str[0] || '\r' == str[0])
        {
            return Strip(str.substr(1, str.size() - 1));
        }
        else if (' ' == str[tail] || '\t' == str[tail] || '\n' == str[tail] || '\v' == str[tail] || '\f' == str[tail] || '\r' == str[tail])
        {
            return Strip(str.substr(0, str.size() - 1));
        }
    }
    return str;
}

} // namespace config

#endif // AIR_CONFIG_LIB_H
