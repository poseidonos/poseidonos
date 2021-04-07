
#ifndef AIR_CONFIG_STRING_H
#define AIR_CONFIG_STRING_H

#include <iostream>
#include <stdexcept>
#include <string>

namespace config
{
const std::size_t npos{0xFFFFFFFF};

class String
{
public:
    template<std::size_t N>
    constexpr String(const char (&a)[N])
    : str(a),
      len(N - 1)
    {
    }
    constexpr String(const char* a, std::size_t N)
    : str(a),
      len(N - 1)
    {
    }

    constexpr char operator[](std::size_t n) const
    {
        return n < len ? str[n] : '\0';
    }
    constexpr bool
    operator==(const String& a)
    {
        if (0 == Compare(a))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    constexpr bool
    operator!=(const String& a)
    {
        if (0 == Compare(a))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    friend std::ostream&
    operator<<(std::ostream& os, const String& a)
    {
        for (std::size_t i = 0; i < a.Length(); i++)
        {
            os << a[i];
        }
        return os;
    }
    constexpr const char*
    Data(void) const noexcept
    {
        return str;
    }
    constexpr std::size_t
    Length(void) const
    {
        return len;
    }
    constexpr std::size_t
    Size(void) const
    {
        return len;
    }

    constexpr std::size_t
    Find(String a, std::size_t pos = 0) const noexcept
    {
        std::size_t result{npos};
        if (pos < len)
        {
            std::size_t cnt{0};
            std::size_t curr_pos{pos};
            while (curr_pos < len)
            {
                if (a[cnt] == str[curr_pos])
                {
                    cnt++;
                }
                else
                {
                    cnt = 0;
                }
                if (a.Length() == cnt)
                {
                    return (curr_pos - cnt + 1);
                }
                curr_pos++;
            }
        }
        return result;
    }

    constexpr std::size_t
    FindFirstNotOf(char c, std::size_t pos = 0) const noexcept
    {
        std::size_t result{npos};
        if (pos < len)
        {
            std::size_t curr_pos{pos};
            while (curr_pos < len)
            {
                if (c != str[curr_pos])
                {
                    return curr_pos;
                }
                curr_pos++;
            }
        }
        return result;
    }

    constexpr String
    Substr(std::size_t start, std::size_t length) const noexcept
    {
        return String{str + start, length};
    }

    constexpr void
    RemovePrefix(std::size_t n) noexcept
    {
        this->str += n;
        this->len -= n;
    }

    constexpr void
    RemoveSuffix(std::size_t n) noexcept
    {
        this->len -= n;
    }

    constexpr int
    Compare(String a) const noexcept
    {
        if (a.Length() != len)
        {
            return -1;
        }
        for (std::size_t i = 0; i < len; i++)
        {
            if (a[i] != str[i])
            {
                return -1;
            }
        }
        return 0;
    }

private:
    const char* str;
    std::size_t len;
};

} // namespace config

#endif // AIR_CONFIG_STRING_H
