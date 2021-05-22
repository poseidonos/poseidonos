
#ifndef AIR_STRING_VIEW_H
#define AIR_STRING_VIEW_H

#include <iostream>
#include <stdexcept>
#include <string>

namespace air
{
class string_view
{
public:
    template<std::size_t N>
    constexpr string_view(const char (&a)[N])
    : str(a),
      len(N - 1)
    {
    }
    constexpr string_view(const char* a, std::size_t N)
    : str(a),
      len(N)
    {
    }
    constexpr std::size_t
    size() const
    {
        return len;
    }
    constexpr const char*
    data() const noexcept
    {
        return str;
    }
    constexpr char operator[](std::size_t n) const
    {
        return n < len ? str[n] : throw std::out_of_range("air::string_view operator[] invalid");
    }
    constexpr bool
    operator==(const air::string_view& arg) const
    {
        if (0 == compare(arg))
        {
            return true;
        }
        return false;
    }
    constexpr bool
    operator!=(const air::string_view& arg) const
    {
        if (0 == compare(arg))
        {
            return false;
        }
        return true;
    }
    constexpr int
    compare(air::string_view arg) const
    {
        if (arg.size() != len)
        {
            return -1;
        }
        for (std::size_t i = 0; i < len; i++)
        {
            if (arg[i] != str[i])
            {
                return -1;
            }
        }
        return 0;
    }
    constexpr std::size_t
    find(air::string_view arg, std::size_t pos = 0) const
    {
        std::size_t result{npos};
        if (len > pos)
        {
            std::size_t cnt{0};
            std::size_t curr_pos{pos};
            while (len > curr_pos)
            {
                if (arg[cnt] == str[curr_pos])
                {
                    cnt++;
                }
                else
                {
                    cnt = 0;
                }
                if (arg.size() == cnt)
                {
                    return curr_pos - cnt + 1;
                }

                curr_pos++;
            }
        }
        return result;
    }
    constexpr air::string_view
    substr(std::size_t start, std::size_t length) const noexcept
    {
        std::size_t left_size = length;
        if (len < start + length)
        {
            left_size = len - start;
        }
        return air::string_view{str + start, left_size};
    }
    friend std::ostream&
    operator<<(std::ostream& os, const air::string_view& arg)
    {
        for (std::size_t i = 0; i < arg.size(); i++)
        {
            os << arg[i];
        }
        return os;
    }
    const static std::size_t npos{0xFFFFFFFF};

private:
    const char* str;
    std::size_t len;
};

} // namespace air

#endif // AIR_STRING_VIEW_H
