
#ifndef AIR_CONFIG_CHECKER_KEY_H
#define AIR_CONFIG_CHECKER_KEY_H

#include <cstdint>
#include <stdexcept>

#include "src/config/ConfigLib.h"
#include "src/lib/StringView.h"

namespace config
{
static constexpr int32_t
CheckKeyTypo(ParagraphType type, const air::string_view* key_list, air::string_view sentence)
{
    size_t cur_pos{sentence.find(":")};
    size_t prev_pos{0};
    bool is_type_violation = false;
    bool has_sampling_ratio = false;

    while (air::string_view::npos != cur_pos)
    {
        air::string_view key = sentence.substr(prev_pos, cur_pos - prev_pos);
        key = Strip(key);

        bool find = false;

        if (ParagraphType::DEFAULT == type)
        {
            for (uint32_t i = 0; i < NUM_DEFAULT_KEY; i++)
            {
                if (key == key_list[i])
                {
                    find = true;
                    break;
                }
            }
        }
        else if (ParagraphType::GROUP == type)
        {
            for (uint32_t i = 0; i < NUM_GROUP_KEY; i++)
            {
                if (key == key_list[i])
                {
                    find = true;
                    break;
                }
            }
        }
        else if (ParagraphType::FILTER == type)
        {
            for (uint32_t i = 0; i < NUM_FILTER_KEY; i++)
            {
                if (key == key_list[i])
                {
                    find = true;
                    break;
                }
            }
        }
        else if (ParagraphType::NODE == type)
        {
            for (uint32_t i = 0; i < NUM_NODE_KEY; i++)
            {
                if (key == key_list[i])
                {
                    find = true;
                    break;
                }
            }
            if (key == "SamplingRatio")
            {
                has_sampling_ratio = true;
            }
            if (key == "Type")
            {
                size_t next_comma = sentence.find(",", cur_pos + 1);
                if (air::string_view::npos == next_comma)
                {
                    next_comma = sentence.size();
                }
                air::string_view value = sentence.substr(cur_pos + 1, next_comma - cur_pos - 1);
                value = Strip(value);
                if (value == "PERFORMANCE" || value == "LATENCY" || value == "UTILIZATION" || value == "COUNT" ||
                    value == "Performance" || value == "Latency" || value == "Utilization" || value == "Count")
                {
                    is_type_violation = true;
                }
            }
        }
        else
        {
            throw std::invalid_argument("CheckKeyTypo got invalid type");
        }

        if (false == find)
        {
            throw std::logic_error("Invalid key was used");
        }

        if (is_type_violation && has_sampling_ratio)
        {
            throw std::logic_error("Only queue type can have SamplingRatio");
        }

        prev_pos = sentence.find(",", prev_pos + 1);
        prev_pos += 1;
        cur_pos = sentence.find(":", cur_pos + 1);
    }

    return 0;
}

static constexpr int32_t
CheckMandatoryKey(const air::string_view* key_list, uint32_t num_mandatory, air::string_view sentence)
{
    int key_cnt = 0;

    uint32_t num_find = 0;
    for (uint32_t index = 0; index < num_mandatory; index++, key_cnt++)
    {
        size_t cur_pos = sentence.find(":");
        size_t prev_pos{0};
        air::string_view mandatory_key = key_list[key_cnt];
        bool find{false};

        while (air::string_view::npos != cur_pos)
        {
            air::string_view sentence_key = sentence.substr(prev_pos, cur_pos - prev_pos);
            sentence_key = Strip(sentence_key);

            if (mandatory_key == sentence_key)
            {
                find = true;
                num_find += 1;
                break;
            }

            prev_pos = sentence.find(",", prev_pos + 1);
            prev_pos += 1;
            cur_pos = sentence.find(":", cur_pos + 1);
        }

        if (false == find)
        {
            throw std::logic_error("Mandatory key is missing");
        }
    }

    return 0;
}

static constexpr int32_t
CheckKeyDuplication(air::string_view sentence)
{
    size_t colon_pos{sentence.find(":")};
    size_t comma_pos{0};

    while (air::string_view::npos != colon_pos)
    {
        air::string_view target_key = sentence.substr(comma_pos, colon_pos - comma_pos);
        target_key = Strip(target_key);

        size_t comparative_colon{sentence.find(":", colon_pos + 1)};
        size_t comparative_comma{sentence.find(",", comma_pos + 1)};
        comparative_comma += 1;
        while (air::string_view::npos != comparative_colon)
        {
            air::string_view comparative_key = sentence.substr(comparative_comma, comparative_colon - comparative_comma);
            comparative_key = Strip(comparative_key);

            if (target_key == comparative_key)
            {
                throw std::logic_error("Key duplicated");
            }

            comparative_comma = sentence.find(",", comparative_comma + 1);
            comparative_comma += 1;
            comparative_colon = sentence.find(":", comparative_colon + 1);
        }

        comma_pos = sentence.find(",", comma_pos + 1);
        comma_pos += 1;
        colon_pos = sentence.find(":", colon_pos + 1);
    }
    return 0;
}

} // namespace config

#endif // AIR_CONFIG_CHECKER_KEY_H
