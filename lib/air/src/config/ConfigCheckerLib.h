
#ifndef AIR_CONFIG_CHECKER_LIB_H
#define AIR_CONFIG_CHECKER_LIB_H

#include <cstdint>
#include <stdexcept>

#include "src/config/ConfigLib.h"
#include "src/lib/StringView.h"

namespace config
{
static constexpr int32_t
CheckCharacterFormat(air::string_view sentence)
{
    size_t start_quote = sentence.find("\"");
    if (air::string_view::npos == start_quote)
    {
        throw std::logic_error("Sentence has to start with quotes");
    }

    size_t end_quote = sentence.find("\"", start_quote + 1);
    if (air::string_view::npos == end_quote)
    {
        throw std::logic_error("Sentence has to end with quotes");
    }

    size_t pos{start_quote};
    while (pos < end_quote)
    {
        pos++;

        if (IsSpace(sentence[pos]))
        {
            continue;
        }
        else if ('0' <= sentence[pos] && '9' >= sentence[pos])
        {
            continue;
        }
        else if ('a' <= sentence[pos] && 'z' >= sentence[pos])
        {
            continue;
        }
        else if ('A' <= sentence[pos] && 'Z' >= sentence[pos])
        {
            continue;
        }
        else if ('(' == sentence[pos] || ')' == sentence[pos] || '.' == sentence[pos])
        {
            continue;
        }
        else if (':' == sentence[pos] || ',' == sentence[pos])
        {
            continue;
        }
        else if ('_' == sentence[pos] || '"' == sentence[pos])
        {
            continue;
        }
        else
        {
            throw std::logic_error("Sentence has invalid character");
        }
    }

    return 0;
}

static constexpr int32_t
CheckSentenceFormat(air::string_view sentence)
{
    if (0 > CheckCharacterFormat(sentence))
    {
        throw std::logic_error("Invalid character format");
    }
    size_t start_quote = sentence.find("\"");
    size_t end_quote = sentence.find("\"", start_quote + 1);

    size_t curr_colon_pos{0};
    size_t curr_comma_pos{0};
    int colon_cnt{0};
    int comma_cnt{0};

    while (curr_colon_pos < sentence.size() && curr_comma_pos < sentence.size())
    {
        curr_colon_pos = sentence.find(":", curr_colon_pos);
        if (air::string_view::npos == curr_colon_pos)
        {
            break;
        }
        colon_cnt++;

        curr_comma_pos = sentence.find(",", curr_comma_pos);
        if (air::string_view::npos == curr_comma_pos)
        {
            break;
        }
        comma_cnt++;

        if (curr_colon_pos >= curr_comma_pos)
        {
            throw std::logic_error("Sentence has invalid comma or colon");
        }
        curr_colon_pos++;
        curr_comma_pos++;
    }

    if (colon_cnt < 1)
    {
        throw std::logic_error("Too few key:value pair");
    }

    if (colon_cnt != (comma_cnt + 1))
    {
        throw std::logic_error("Wrong key:value pair");
    }

    size_t prev_comma_pos{start_quote};
    while (prev_comma_pos < sentence.size())
    {
        curr_colon_pos = sentence.find(":", prev_comma_pos);
        curr_comma_pos = sentence.find(",", prev_comma_pos);
        if (air::string_view::npos == curr_comma_pos)
        {
            curr_comma_pos = end_quote;
        }

        air::string_view key =
            sentence.substr(prev_comma_pos + 1, curr_colon_pos - prev_comma_pos - 1);
        air::string_view value =
            sentence.substr(curr_colon_pos + 1, curr_comma_pos - curr_colon_pos - 1);
        key = Strip(key);
        value = Strip(value);

        if (0 == key.size() || 0 == value.size())
        {
            throw std::logic_error("Key or value is empty");
        }

        prev_comma_pos = curr_comma_pos + 1;
    }

    return 0;
}

static constexpr int32_t
CheckSentenceFilterFormat(air::string_view sentence)
{
    if (0 > CheckCharacterFormat(sentence))
    {
        throw std::logic_error("Invalid character format in filter sentence");
    }

    size_t pos{0};
    const size_t range_start_pos = sentence.find("(");
    if (air::string_view::npos == range_start_pos)
    {
        throw std::logic_error("Range expression() is missing in filter sentence");
    }
    if (air::string_view::npos != sentence.find("(", range_start_pos + 1))
    {
        throw std::logic_error("Invalid range expression() in filter sentence");
    }
    const size_t range_end_pos = sentence.find(")");
    if (air::string_view::npos == range_end_pos)
    {
        throw std::logic_error("Range expression() is missing in filter sentence");
    }
    if (air::string_view::npos != sentence.find(")", range_end_pos + 1))
    {
        throw std::logic_error("Invalid range expression() in filter sentence");
    }
    size_t comma_cnt{0};
    size_t colon_cnt{0};

    while (pos < sentence.size())
    {
        if (',' == sentence[pos])
        {
            if (range_start_pos > pos || range_end_pos < pos)
            {
                comma_cnt++;
            }
        }
        else if (':' == sentence[pos])
        {
            if (range_start_pos < pos && range_end_pos > pos)
            {
                throw std::logic_error("Colon cannot exist in range expression() in filter sentence");
            }
            colon_cnt++;
        }

        pos++;
    }

    if (colon_cnt < 1)
    {
        throw std::logic_error("Too few key:value pair in filter sentence");
    }

    if (colon_cnt != (comma_cnt + 1))
    {
        throw std::logic_error("Wrong key:value pair in filter sentence");
    }

    size_t filter_key_pos = sentence.find("Filter");
    if (air::string_view::npos == filter_key_pos)
    {
        throw std::logic_error("Filter key is missing in filter sentence");
    }
    size_t filter_value_start_pos = sentence.find(":", filter_key_pos) + 1;
    size_t filter_value_end_pos = sentence.find(",", filter_value_start_pos);
    air::string_view filter_value = sentence.substr(filter_value_start_pos,
        filter_value_end_pos - filter_value_start_pos);

    filter_value = Strip(filter_value);
    if (0 == filter_value.size())
    {
        throw std::logic_error("Filter value is empty in filter sentence");
    }

    air::string_view item_value = sentence.substr(range_start_pos + 1,
        range_end_pos - range_start_pos - 1);
    if (0 == item_value.size())
    {
        throw std::logic_error("Item value is empty in filter sentence");
    }

    return 0;
}

static constexpr int32_t
CheckParagraphFormat(ParagraphType type, air::string_view paragraph)
{
    size_t start_quote{0};
    size_t end_quote{0};
    size_t curr_pos{0};
    size_t start_cnt{0};
    size_t end_cnt{0};

    while (curr_pos < paragraph.size())
    {
        start_quote = paragraph.find("\"", curr_pos);
        if (air::string_view::npos == start_quote)
        {
            break;
        }
        start_cnt++;

        end_quote = paragraph.find("\"", start_quote + 1);
        if (air::string_view::npos == end_quote)
        {
            break;
        }
        end_cnt++;

        if (ParagraphType::FILTER == type)
        {
            if (0 != CheckSentenceFilterFormat(paragraph.substr(start_quote, end_quote - start_quote + 1)))
            {
                throw std::logic_error("SentenceFilterFormat violation");
            }
        }
        else
        {
            if (0 != CheckSentenceFormat(paragraph.substr(start_quote, end_quote - start_quote + 1)))
            {
                throw std::logic_error("SentenceFormat violation");
            }
        }

        curr_pos = end_quote + 1;
    }

    if (0 == start_cnt)
    {
        throw std::logic_error("No sentence in paragraph");
    }

    if (start_cnt != end_cnt)
    {
        throw std::logic_error("Quotes are missing");
    }

    return 0;
}

static constexpr int32_t
CheckValueDuplication(air::string_view key, air::string_view paragraph)
{
    size_t pos = paragraph.find(key);

    while (pos < paragraph.size())
    {
        size_t colon_pos = paragraph.find(":", pos + 1);
        size_t comma_pos = paragraph.find(",", colon_pos + 1);
        size_t quote_pos = paragraph.find("\"", colon_pos + 1);

        air::string_view value{""};
        if (quote_pos < comma_pos)
        {
            value = paragraph.substr(colon_pos + 1, quote_pos - colon_pos - 1);
        }
        else
        {
            value = paragraph.substr(colon_pos + 1, comma_pos - colon_pos - 1);
        }
        value = Strip(value);

        size_t next_pos = paragraph.find(key, pos + 1);
        while (next_pos < paragraph.size())
        {
            size_t next_colon = paragraph.find(":", next_pos + 1);
            if (air::string_view::npos == next_colon)
            {
                throw std::logic_error("Sentence syntax is invalid");
            }
            size_t next_comma = paragraph.find(",", next_pos + 1);
            if (air::string_view::npos == next_comma)
            {
                next_comma = paragraph.find("\"", next_pos + 1);
            }

            air::string_view next_value = paragraph.substr(next_colon + 1, next_comma - next_colon - 1);
            next_value = Strip(next_value);

            if (value == next_value)
            {
                throw std::logic_error("Value duplicated");
            }

            next_pos = paragraph.find(key, next_pos + 1);
        }
        pos = paragraph.find(key, pos + 1);
    }

    return 0;
}

} // namespace config

#endif // AIR_CONFIG_CHECKER_LIB_H
