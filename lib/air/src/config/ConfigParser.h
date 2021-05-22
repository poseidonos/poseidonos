
#ifndef AIR_CONFIG_PARSER_H
#define AIR_CONFIG_PARSER_H

#include <iostream>
#include <string>

#include "src/config/ConfigLib.h"
#include "src/config/ConfigReader.h"
#include "src/lib/StringView.h"

namespace config
{
class ConfigParser
{
public:
    ConfigParser(void)
    {
    }
    virtual ~ConfigParser(void)
    {
    }

    static constexpr int32_t
    GetItemSizeFromFilterSentence(air::string_view sentence)
    {
        size_t value_start_pos = sentence.find("(");
        size_t value_end_pos = sentence.find(")");
        air::string_view value = sentence.substr(value_start_pos + 1, value_end_pos - value_start_pos - 1);

        size_t range_pos = value.find("...");
        size_t comma_pos = value.find(",");

        if (air::string_view::npos != range_pos)
        {
            air::string_view start_item = value.substr(0, range_pos);
            air::string_view end_item = value.substr(range_pos + 3, value.size() - range_pos - 3);
            start_item = Strip(start_item);
            end_item = Strip(end_item);
            size_t start_token_pos = start_item.find("_");
            size_t end_token_pos = end_item.find("_");
            air::string_view start_suffix = start_item.substr(start_token_pos + 1, start_item.size() - start_token_pos);
            air::string_view end_suffix = end_item.substr(end_token_pos + 1, end_item.size() - end_token_pos);
            uint32_t start_num = Stoi(start_suffix);
            uint32_t end_num = Stoi(end_suffix);

            if (end_num < start_num)
            {
                return -1;
            }

            return end_num - start_num + 1;
        }
        else if (air::string_view::npos != comma_pos)
        {
            int32_t count{0};
            size_t c_index{0};

            while (c_index < value.size())
            {
                if (',' == value[c_index])
                {
                    count++;
                }
                c_index++;
            }

            return count + 1;
        }
        else
        {
            return 1;
        }

        return -1;
    }

    static constexpr int32_t
    GetIntValueFromSentence(air::string_view sentence, air::string_view key, air::string_view index = "")
    {
        size_t start_pos = sentence.find(key);

        if (air::string_view::npos == start_pos)
        {
            return -1;
        }

        if (key == "Item")
        {
            return _GetItemIndexValueFromFilterSentence(sentence, index);
        }

        size_t comma_pos = sentence.find(",", start_pos + 1);
        if (comma_pos > sentence.size())
        {
            comma_pos = sentence.size();
        }
        air::string_view key_value = sentence.substr(start_pos, comma_pos - start_pos);
        size_t colon_pos = key_value.find(":");

        air::string_view value = key_value.substr(colon_pos + 1, key_value.size() - colon_pos);
        value = Strip(value);

        if (key == "AirBuild" || key == "NodeBuild" || key == "NodeRun" || key == "Build" || key == "Run")
        {
            if (value == "TRUE" || value == "True" || value == "true" || value == "ON" || value == "On" || value == "on")
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

        return Stoi(value);
    }

    static std::string
    GetItemStrFromFilterSentence(air::string_view sentence, uint32_t index)
    {
        size_t value_start_pos = sentence.find("(");
        size_t value_end_pos = sentence.find(")");
        air::string_view value = sentence.substr(value_start_pos + 1, value_end_pos - value_start_pos - 1);

        size_t comma_pos = value.find(",");
        size_t range_pos = value.find("...");

        std::string result{""};

        if (air::string_view::npos != range_pos)
        {
            air::string_view start_item = value.substr(0, range_pos);
            start_item = Strip(start_item);
            size_t start_token_pos = start_item.find("_");
            air::string_view start_prefix = start_item.substr(0, start_token_pos);
            air::string_view start_suffix = start_item.substr(start_token_pos + 1, start_item.size() - start_token_pos);
            uint32_t start_num = Stoi(start_suffix);
            air::string_view end_item = value.substr(range_pos + 3, value.size() - range_pos - 3);
            end_item = Strip(end_item);
            size_t end_token_pos = end_item.find("_");
            air::string_view end_suffix = end_item.substr(end_token_pos + 1, end_item.size() - end_token_pos);
            uint32_t end_num = Stoi(end_suffix);

            if (end_num < start_num + index)
            {
                return "";
            }

            result.assign(start_prefix.data(), start_prefix.size());
            result += "_";
            result += std::to_string(start_num + index);
        }
        else if (air::string_view::npos != comma_pos)
        {
            uint32_t count = 0;
            size_t prev_comma = 0;
            size_t curr_comma = comma_pos;

            while (prev_comma < value.size())
            {
                air::string_view range_value = value.substr(prev_comma, curr_comma - prev_comma);
                range_value = Strip(range_value);

                if (index == count)
                {
                    result.assign(range_value.data(), range_value.size());
                    break;
                }
                count++;

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
            air::string_view strip_value = Strip(value);
            result.assign(strip_value.data(), strip_value.size());
        }

        return result;
    }

    static constexpr air::string_view
    GetStrValueFromSentence(air::string_view sentence, air::string_view key)
    {
        if (key != "Group" && key != "Node" && key != "Filter" && key != "Type")
        {
            return "";
        }

        size_t start_pos = sentence.find(key);
        if (air::string_view::npos == start_pos)
        {
            return "";
        }

        size_t comma_pos = sentence.find(",", start_pos + 1);
        if (air::string_view::npos == comma_pos)
        {
            comma_pos = sentence.size();
        }

        size_t colon_pos = sentence.find(":", start_pos + 1);
        air::string_view value = sentence.substr(colon_pos + 1, comma_pos - colon_pos - 1);
        value = Strip(value);

        return value;
    }

    static constexpr air::string_view
    GetSentenceFromParagraph(ParagraphType type, uint32_t index = 0)
    {
        air::string_view paragraph = paragraphs[dtype(type)];
        uint32_t sentence_size = sentences_count[dtype(type)];

        if (sentence_size <= index)
        {
            return "";
        }

        size_t skip_count = 2 * index;
        size_t start_pos = paragraph.find("\"");
        while (skip_count)
        {
            start_pos = paragraph.find("\"", start_pos + 1);
            skip_count--;
        }
        size_t end_pos = paragraph.find("\"", start_pos + 1);

        return paragraph.substr(start_pos + 1, end_pos - start_pos - 1);
    }

    static constexpr int32_t
    GetIndexFromParagraph(ParagraphType type, air::string_view name = "")
    {
        air::string_view paragraph{""};
        air::string_view key_name{""};

        if (ParagraphType::GROUP == type)
        {
            key_name = "Group";
        }
        else if (ParagraphType::FILTER == type)
        {
            key_name = "Filter";
        }
        else if (ParagraphType::NODE == type)
        {
            key_name = "Node";
        }
        else
        {
            return 0;
        }

        paragraph = paragraphs[dtype(type)];

        size_t start_kv = paragraph.find(key_name);
        size_t colon_pos = paragraph.find(":");
        size_t separator_pos = paragraph.find(",");
        size_t count = 0;
        while (air::string_view::npos != start_kv)
        {
            size_t quote_pos = paragraph.find("\"", colon_pos + 1);
            if (air::string_view::npos == separator_pos || quote_pos <= separator_pos)
            {
                separator_pos = quote_pos;
            }

            air::string_view key = paragraph.substr(start_kv, colon_pos - start_kv + 1);
            key = Strip(key);
            air::string_view value = paragraph.substr(colon_pos + 1, separator_pos - colon_pos - 1);
            value = Strip(value);

            if (0 == name.compare(value))
            {
                return count;
            }

            start_kv = paragraph.find(key_name, start_kv + 1);
            colon_pos = paragraph.find(":", start_kv + 1);
            separator_pos = paragraph.find(",", start_kv + 1);
            count += 1;
        }

        return -1;
    }

    static constexpr uint32_t
    GetSentenceCount(ParagraphType type)
    {
        return sentences_count[dtype(type)];
    }

private:
    static constexpr int32_t
    _GetItemIndexValueFromFilterSentence(air::string_view sentence, air::string_view index)
    {
        size_t value_end_pos = sentence.find(")");
        size_t value_start_pos = sentence.find("(");
        air::string_view value = sentence.substr(value_start_pos + 1, value_end_pos - value_start_pos - 1);

        size_t comma_pos = value.find(",");
        size_t range_pos = value.find("...");

        if (air::string_view::npos != range_pos)
        {
            air::string_view end_item = value.substr(range_pos + 3, value.size() - range_pos - 3);
            air::string_view start_item = value.substr(0, range_pos);
            end_item = Strip(end_item);
            start_item = Strip(start_item);
            size_t end_token_pos = end_item.find("_");
            size_t start_token_pos = start_item.find("_");
            air::string_view end_suffix = end_item.substr(end_token_pos + 1, end_item.size() - end_token_pos);
            air::string_view start_suffix = start_item.substr(start_token_pos + 1, start_item.size() - start_token_pos);
            int32_t end_num = static_cast<int32_t>(Stoi(end_suffix));
            int32_t start_num = static_cast<int32_t>(Stoi(start_suffix));

            size_t index_token_pos = index.find("_");
            air::string_view index_suffix = index.substr(index_token_pos + 1, index.size() - index_token_pos);
            int32_t index_num = static_cast<int32_t>(Stoi(index_suffix));

            if (end_num < index_num)
            {
                return -1;
            }
            
            return index_num - start_num;
        }
        else if (air::string_view::npos != comma_pos)
        {
            size_t prev_comma = 0;
            size_t curr_comma = comma_pos;
            uint32_t count = 0;

            while (prev_comma < value.size())
            {
                air::string_view range_value = value.substr(prev_comma, curr_comma - prev_comma);
                range_value = Strip(range_value);

                if (index == range_value)
                {
                    return count;
                }
                count++;

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
            return 0;
        }

        return -1;
    }
};

} // namespace config

#endif // AIR_CONFIG_PARSER_H
