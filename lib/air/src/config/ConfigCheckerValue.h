
#ifndef AIR_CONFIG_CHECKER_VALUE_H
#define AIR_CONFIG_CHECKER_VALUE_H

#include <stdexcept>

#include "src/config/ConfigLib.h"
#include "src/config/ConfigParser.h"
#include "src/lib/StringView.h"

namespace config
{
class ConfigCheckerValue
{
public:
    ConfigCheckerValue(void)
    {
    }

    virtual ~ConfigCheckerValue(void)
    {
    }

    static constexpr int32_t
    CheckValueValidity(ParagraphType type, air::string_view sentence)
    {
        size_t colon_pos{sentence.find(":")};
        size_t prev_comma{0};
        size_t post_comma{sentence.find(",")};

        while (colon_pos < sentence.size())
        {
            air::string_view key = sentence.substr(prev_comma, colon_pos - prev_comma);
            key = Strip(key);

            if (post_comma > sentence.size())
            {
                post_comma = sentence.size();
            }

            air::string_view value = sentence.substr(colon_pos + 1, post_comma - colon_pos - 1);
            if (key == "Item")
            {
                std::size_t range_start_pos = sentence.find("(", colon_pos);
                std::size_t range_end_pos = sentence.find(")", colon_pos);
                value = sentence.substr(range_start_pos + 1, range_end_pos - range_start_pos - 1);
            }
            value = Strip(value);

            int32_t result{0};
            if (ParagraphType::DEFAULT == type)
            {
                result = _CheckDefaultValue(key, value);
            }
            else if (ParagraphType::GROUP == type)
            {
                result = _CheckGroupValue(key, value);
            }
            else if (ParagraphType::FILTER == type)
            {
                result = _CheckFilterValue(key, value);
            }
            else if (ParagraphType::NODE == type)
            {
                result = _CheckNodeValue(key, value);
            }
            else
            {
                throw std::invalid_argument("CheckValueValidity got invalid type");
            }

            if (0 > result)
            {
                throw std::logic_error("Value is invalid");
            }

            prev_comma = post_comma + 1;
            colon_pos = sentence.find(":", colon_pos + 1);
            post_comma = sentence.find(",", colon_pos + 1);

            if (post_comma >= sentence.size())
            {
                post_comma = sentence.size();
            }
        }

        return 0;
    }

private:
    static constexpr int32_t
    _CheckNameValue(air::string_view value)
    {
        if (MAX_NAME_LEN < value.size())
        {
            throw std::length_error("Name length limit: 30 characters");
        }
        return 0;
    }

    static constexpr int32_t
    _CheckSameValueInParagraph(air::string_view key, air::string_view value, ParagraphType type)
    {
        const uint32_t paragraph_type_index{dtype(type)};
        bool has_same_value{false};

        for (uint32_t sentence_index = 0; sentence_index < sentences_count[paragraph_type_index]; sentence_index++)
        {
            air::string_view sentence = ConfigParser::GetSentenceFromParagraph(type, sentence_index);
            size_t name_key_pos = sentence.find(key);
            size_t name_colon_pos = sentence.find(":", name_key_pos + 1);
            size_t name_comma_pos = sentence.find(",", name_key_pos + 1);
            size_t name_quote_pos = sentence.find("\"", name_key_pos + 1);
            size_t token_pos = 0;
            if (name_comma_pos < name_quote_pos)
            {
                token_pos = name_comma_pos;
            }
            else
            {
                token_pos = name_quote_pos;
            }
            air::string_view name_value = sentence.substr(name_colon_pos + 1, token_pos - name_colon_pos - 1);
            name_value = Strip(name_value);

            if (value == name_value)
            {
                has_same_value = true;
                break;
            }
        }

        if (false == has_same_value)
        {
            throw std::logic_error("Can not find the value in (Group or Filter)paragraph");
        }

        return 0;
    }

    static constexpr int32_t
    _CheckType(air::string_view value)
    {
        if (value == "PERFORMANCE" || value == "Performance" ||
            value == "LATENCY" || value == "Latency" ||
            value == "QUEUE" || value == "Queue" ||
            value == "UTILIZATION" || value == "Utilization" ||
            value == "COUNT" || value == "Count")
        {
            return 0;
        }
        else
        {
            throw std::logic_error("Invalid type value");
        }
    }

    static constexpr int32_t
    _CheckBoolValue(air::string_view value)
    {
        if (value == "true" || value == "True" || value == "TRUE" || value == "false" || value == "False" || value == "FALSE" ||
            value == "on" || value == "On" || value == "ON" || value == "off" || value == "Off" || value == "OFF")
        {
            return 0;
        }
        else
        {
            throw std::logic_error("Invalid boolean value");
        }
    }

    static constexpr int32_t
    _CheckNumberValue(air::string_view value, uint32_t digit_max)
    {
        if (1 <= value.size() && digit_max >= value.size())
        {
            if ('0' == value[0] || '-' == value[0])
            {
                throw std::range_error("Number value has to be positive");
            }
            for (uint32_t index = 0; index < value.size(); index++)
            {
                if ('0' > value[index] || '9' < value[index])
                {
                    throw std::out_of_range("Only number can be value");
                }
            }
        }
        else
        {
            throw std::overflow_error("Exceed maximum value");
        }

        return 0;
    }

    static constexpr int32_t
    _CheckRangeValue(air::string_view value)
    {
        size_t range_pos = value.find("...");
        size_t comma_pos = value.find(",");

        if (air::string_view::npos != range_pos && air::string_view::npos != comma_pos)
        {
            throw std::logic_error("Range expression cannot have both , and ... token");
        }
        else if (air::string_view::npos != range_pos)
        {
            air::string_view start_item = value.substr(0, range_pos);
            start_item = Strip(start_item);
            air::string_view end_item = value.substr(range_pos + 3, value.size() - range_pos - 3);
            end_item = Strip(end_item);

            if (0 == start_item.size() || 0 == end_item.size())
            {
                throw std::logic_error("Range expression missed start or end value");
            }

            size_t start_token_pos = start_item.find("_");
            size_t end_token_pos = end_item.find("_");
            if (air::string_view::npos == start_token_pos || air::string_view::npos == end_token_pos)
            {
                throw std::logic_error("Range value must have underscope(_)");
            }

            air::string_view start_prefix = start_item.substr(0, start_token_pos);
            air::string_view end_prefix = end_item.substr(0, end_token_pos);
            if (start_prefix != end_prefix)
            {
                throw std::logic_error("Range value prefix must be same");
            }

            air::string_view start_suffix = start_item.substr(start_token_pos + 1, start_item.size() - start_token_pos);
            air::string_view end_suffix = end_item.substr(end_token_pos + 1, end_item.size() - end_token_pos);

            if (!IsUInt(start_suffix) || !IsUInt(end_suffix))
            {
                throw std::logic_error("Range value suffix must be an unsigned integer");
            }

            uint32_t start_num = Stoi(start_suffix);
            uint32_t end_num = Stoi(end_suffix);

            if (start_num >= end_num)
            {
                throw std::logic_error("Range value suffix underflow");
            }

            if (100 < end_num - start_num + 1)
            {
                throw std::logic_error("Range value suffix overflow");
            }
        }
        else if (air::string_view::npos != comma_pos)
        {
            size_t prev_comma = 0;
            size_t curr_comma = comma_pos;

            while (prev_comma < value.size())
            {
                air::string_view range_value = value.substr(prev_comma, curr_comma - prev_comma);
                range_value = Strip(range_value);

                if (0 == range_value.size())
                {
                    throw std::logic_error("Range value is empty");
                }
                else if (HasSpace(range_value))
                {
                    throw std::logic_error("Range value cannot have space");
                }

                prev_comma = curr_comma + 1;
                curr_comma = value.find(",", curr_comma + 1);

                if (air::string_view::npos == curr_comma)
                {
                    curr_comma = value.size();
                }
            }
        }
        else
        {
            if (HasSpace(value))
            {
                throw std::logic_error("Range value cannot have space");
            }
            else if (air::string_view::npos != value.find("."))
            {
                throw std::logic_error("Range expression is invalid");
            }
            else if (0 == value.size())
            {
                throw std::logic_error("Range expression is empty");
            }
        }

        return 0;
    }

    static constexpr int32_t
    _CheckDefaultValue(air::string_view key, air::string_view value)
    {
        if (key == "NodeSamplingRatio")
        {
            return _CheckNumberValue(value, 5);
        }
        else if (key == "NodeIndexSize")
        {
            return _CheckNumberValue(value, 3);
        }
        else if (key == "StreamingInterval")
        {
            return _CheckNumberValue(value, 2);
        }
        else if (key == "AirBuild" || key == "NodeBuild" || key == "NodeRun")
        {
            return _CheckBoolValue(value);
        }
        else
        {
            throw std::invalid_argument("_CheckDefaultValue got invalid key");
        }
    }

    static constexpr int32_t
    _CheckGroupValue(air::string_view key, air::string_view value)
    {
        if (key == "Group")
        {
            return _CheckNameValue(value);
        }
        else if (key == "NodeSamplingRatio")
        {
            return _CheckNumberValue(value, 5);
        }
        else if (key == "NodeIndexSize")
        {
            return _CheckNumberValue(value, 3);
        }
        else if (key == "NodeBuild" || key == "NodeRun")
        {
            return _CheckBoolValue(value);
        }
        else
        {
            throw std::invalid_argument("_CheckGroupValue got invalid key");
        }
    }

    static constexpr int32_t
    _CheckFilterValue(air::string_view key, air::string_view value)
    {
        if (key == "Filter")
        {
            return _CheckNameValue(value);
        }
        else if (key == "Item")
        {
            return _CheckRangeValue(value);
        }
        else
        {
            throw std::invalid_argument("_CheckFilterValue got invalid key");
        }

        return 0;
    }

    static constexpr int32_t
    _CheckNodeValue(air::string_view key, air::string_view value)
    {
        if (key == "Node")
        {
            return _CheckNameValue(value);
        }
        else if (key == "Group")
        {
            return _CheckSameValueInParagraph("Group", value, ParagraphType::GROUP);
        }
        else if (key == "Filter")
        {
            return _CheckSameValueInParagraph("Filter", value, ParagraphType::FILTER);
        }
        else if (key == "Type")
        {
            return _CheckType(value);
        }
        else if (key == "SamplingRatio")
        {
            return _CheckNumberValue(value, 5);
        }
        else if (key == "IndexSize")
        {
            return _CheckNumberValue(value, 3);
        }
        else if (key == "Build" || key == "Run")
        {
            return _CheckBoolValue(value);
        }
        else
        {
            throw std::invalid_argument("_CheckNodeValue got invalid key");
        }
    }

    static constexpr uint32_t MAX_NAME_LEN{30};
};

} // namespace config

#endif // AIR_CONFIG_CHECKER_VALUE_H
