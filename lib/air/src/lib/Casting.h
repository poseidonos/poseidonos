
#ifndef AIR_CASTING_H
#define AIR_CASTING_H

#include <type_traits>

template<typename E>
constexpr auto
to_dtype(E e) noexcept // to primitive data types
{
    return static_cast<std::underlying_type_t<E>>(e);
}

#endif // AIR_CASTING_H
