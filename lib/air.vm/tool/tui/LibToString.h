#ifndef AIR_TUI_LIB_TO_STRING_H
#define AIR_TUI_LIB_TO_STRING_H

#include <iomanip>
#include <sstream>
#include <string>

namespace air
{
std::string
Double2StringWithBWFormat(double value)
{
    std::stringstream stream;

    stream << std::fixed << std::setprecision(1);

    if (value < 1000ULL)
    {
        stream << value;
    }
    else if (value < 1000000ULL)
    {
        stream << (value / 1000ULL) << "KB";
    }
    else if (value < 1000000000ULL)
    {
        stream << (value / 1000000ULL) << "MB";
    }
    else
    {
        stream << (value / 1000000000ULL) << "GB";
    }

    size_t padding_count = 7 - stream.str().size();
    while (0 < padding_count)
    {
        stream << " ";
        padding_count--;
    }

    return stream.str();
}

std::string
Double2String(double value)
{
    std::stringstream stream;

    stream << std::fixed << std::setprecision(1);

    if (value < 1000ULL)
    {
        stream << value;
    }
    else if (value < 1000000ULL)
    {
        stream << (value / 1000ULL) << "k";
    }
    else if (value < 1000000000ULL)
    {
        stream << (value / 1000000ULL) << "m";
    }
    else
    {
        stream << (value / 1000000000ULL) << "g";
    }

    size_t padding_count = 6 - stream.str().size();
    while (0 < padding_count)
    {
        stream << " ";
        padding_count--;
    }

    return stream.str();
}

std::string
ULL2StringWithLatencyFormat(uint64_t value)
{
    std::stringstream stream;

    if (value < 1000ULL)
    {
        stream << value << "ns";
    }
    else if (value < 1000000ULL)
    {
        stream << (value / 1000ULL) << "us";
    }
    else if (value < 1000000000ULL)
    {
        stream << (value / 1000000ULL) << "ms";
    }
    else
    {
        stream << (value / 1000000000ULL) << "s";
    }

    size_t padding_count = 5 - stream.str().size();
    while (0 < padding_count)
    {
        stream << " ";
        padding_count--;
    }

    return stream.str();
}

std::string
LL2String(int64_t value)
{
    std::stringstream stream;

    stream << std::fixed << std::setprecision(1);

    if (value < 1000LL && value > -1000LL)
    {
        stream << value;
    }
    else if (value < 1000000LL && value > -1000000LL)
    {
        stream << ((double)value / 1000LL) << "k";
    }
    else if (value < 1000000000LL && value > -1000000000LL)
    {
        stream << ((double)value / 1000000LL) << "m";
    }
    else
    {
        stream << ((double)value / 1000000000LL) << "g";
    }

    size_t padding_count = 7 - stream.str().size();
    while (0 < padding_count)
    {
        stream << " ";
        padding_count--;
    }

    return stream.str();
}

} // namespace air

#endif // AIR_TUI_LIB_TO_STRING_H
