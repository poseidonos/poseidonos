
#ifndef AIR_CONFIG_LIB_H
#define AIR_CONFIG_LIB_H

#include <cstdint>
#include <type_traits>

#include "src/config/ConfigString.h"

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
    WEB,

    COUNT,
    CONFIGTYPE_NULL
};

static constexpr String
Strip(String str)
{
    if (0 != str.Size())
    {
        std::size_t tail = str.Size() - 1;

        if (' ' == str[0] || '\t' == str[0] || '\n' == str[0])
        {
            return Strip(str.Substr(1, str.Size()));
        }
        else if (' ' == str[tail] || '\t' == str[tail] || '\n' == str[tail])
        {
            return Strip(str.Substr(0, str.Size()));
        }
    }

    return str;
}

} // namespace config

#endif // AIR_CONFIG_LIB_H
