
#ifndef AIR_CONFIG_CHECKER_LIB_H
#define AIR_CONFIG_CHECKER_LIB_H

#include <cstdint>
#include <string_view>

#include "src/config/ConfigLib.h"

namespace config
{
static constexpr uint32_t NUM_DEFAULT_KEY{6};
static constexpr uint32_t NUM_GROUP_KEY{6};
static constexpr uint32_t NUM_NODE_KEY{8};

static constexpr int32_t
CheckKeyTypo(ConfigType type, const std::string_view* key_list, std::string_view entry)
{
    size_t cur_pos{entry.find(":")};
    size_t prev_pos{0};
    bool is_type_violation = false;
    bool has_sampling_ratio = false;

    while (std::string_view::npos != cur_pos)
    {
        std::string_view key = entry.substr(prev_pos, cur_pos - prev_pos);
        key = Strip(key);

        bool find = false;

        if (ConfigType::DEFAULT == type)
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
        else if (ConfigType::GROUP == type)
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
        else // NODE
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
                size_t next_comma = entry.find(",", cur_pos + 1);
                if (std::string_view::npos == next_comma)
                {
                    next_comma = entry.size();
                }
                std::string_view value = entry.substr(cur_pos + 1, next_comma - cur_pos - 1);
                value = Strip(value);
                if (value == "PERFORMANCE" || value == "LATENCY" || value == "UTILIZATION" || value == "COUNT")
                {
                    is_type_violation = true;
                }
            }
        }

        if (false == find)
        {
            return -3;
        }

        if (is_type_violation && has_sampling_ratio)
        {
            return -3;
        }

        prev_pos = entry.find(",", prev_pos + 1);
        prev_pos += 1;
        cur_pos = entry.find(":", cur_pos + 1);
    }

    return 0;
}

static constexpr int32_t
CheckMandatoryKey(const std::string_view* key_list, uint32_t num_mandatory, std::string_view entry)
{
    int key_cnt = 0;

    uint32_t num_find = 0;
    for (uint32_t index = 0; index < num_mandatory; index++, key_cnt++)
    {
        size_t cur_pos = entry.find(":");
        size_t prev_pos{0};
        std::string_view mandatory_key = key_list[key_cnt];
        bool find{false};

        while (std::string_view::npos != cur_pos)
        {
            std::string_view entry_key = entry.substr(prev_pos, cur_pos - prev_pos);
            entry_key = Strip(entry_key);

            if (mandatory_key == entry_key)
            {
                find = true;
                num_find += 1;
                break;
            }

            prev_pos = entry.find(",", prev_pos + 1);
            prev_pos += 1;
            cur_pos = entry.find(":", cur_pos + 1);
        }

        if (false == find)
        {
            return -3;
        }
    }

    return 0;
}

static constexpr int32_t
CheckKeyDuplication(std::string_view entry)
{
    size_t colon_pos{entry.find(":")};
    size_t comma_pos{0};

    while (std::string_view::npos != colon_pos)
    {
        std::string_view target_key = entry.substr(comma_pos, colon_pos - comma_pos);
        target_key = Strip(target_key);

        size_t comparative_colon{entry.find(":", colon_pos + 1)};
        size_t comparative_comma{entry.find(",", comma_pos + 1)};
        comparative_comma += 1;
        while (std::string_view::npos != comparative_colon)
        {
            std::string_view comparative_key = entry.substr(comparative_comma, comparative_colon - comparative_comma);
            comparative_key = Strip(comparative_key);

            if (target_key == comparative_key)
            {
                return -3;
            }

            comparative_comma = entry.find(",", comparative_comma + 1);
            comparative_comma += 1;
            comparative_colon = entry.find(":", comparative_colon + 1);
        }

        comma_pos = entry.find(",", comma_pos + 1);
        comma_pos += 1;
        colon_pos = entry.find(":", colon_pos + 1);
    }
    return 0;
}

static constexpr int32_t
CheckEntryFormat(std::string_view str)
{
    size_t start_quote = str.find("\"");
    size_t end_quote = str.find("\"", start_quote + 1);
    size_t comment_pos = str.find("//");

    if (comment_pos > start_quote && comment_pos < end_quote)
    {
        return -3;
    }

    size_t curr_colon_pos{0};
    size_t curr_comma_pos{0};
    int colon_cnt{0};
    int comma_cnt{0};

    while (curr_colon_pos < str.size() && curr_comma_pos < str.size())
    {
        curr_colon_pos = str.find(":", curr_colon_pos);
        if (std::string_view::npos == curr_colon_pos)
        {
            break;
        }
        colon_cnt++;

        curr_comma_pos = str.find(",", curr_comma_pos);
        if (std::string_view::npos == curr_comma_pos)
        {
            break;
        }
        comma_cnt++;

        if (curr_colon_pos >= curr_comma_pos)
        {
            return -3;
        }
        curr_colon_pos++;
        curr_comma_pos++;
    }

    if (colon_cnt < 1)
    {
        return -3;
    }

    if (colon_cnt != (comma_cnt + 1))
    {
        return -3;
    }

    size_t prev_comma_pos{start_quote};
    while (prev_comma_pos < str.size())
    {
        curr_colon_pos = str.find(":", prev_comma_pos);
        curr_comma_pos = str.find(",", prev_comma_pos);
        if (std::string_view::npos == curr_comma_pos)
        {
            curr_comma_pos = end_quote;
        }

        std::string_view key =
            str.substr(prev_comma_pos + 1, curr_colon_pos - prev_comma_pos - 1);
        std::string_view value =
            str.substr(curr_colon_pos + 1, curr_comma_pos - curr_colon_pos - 1);
        key = Strip(key);
        value = Strip(value);

        if (0 == key.size() || 0 == value.size())
        {
            return -3;
        }

        prev_comma_pos = curr_comma_pos + 1;
    }

    return 0;
}

static constexpr int32_t
CheckArrayFormat(std::string_view str)
{
    size_t start_quote{0};
    size_t end_quote{0};
    size_t curr_pos{0};
    size_t start_cnt{0};
    size_t end_cnt{0};

    while (curr_pos < str.size())
    {
        start_quote = str.find("\"", curr_pos);
        if (std::string_view::npos == start_quote)
        {
            break;
        }
        start_cnt++;

        end_quote = str.find("\"", start_quote + 1);
        if (std::string_view::npos == end_quote)
        {
            break;
        }
        end_cnt++;

        if (0 != CheckEntryFormat(str.substr(start_quote, end_quote - start_quote + 1)))
        {
            return -1;
        }

        curr_pos = end_quote + 1;
    }

    if (0 == start_cnt)
    {
        return -1;
    }

    if (start_cnt != end_cnt)
    {
        return -1;
    }

    return 0;
}

static constexpr int32_t
CheckNameDuplication(std::string_view key, std::string_view str)
{
    size_t pos = str.find(key);

    while (pos < str.size())
    {
        size_t colon_pos = str.find(":", pos + 1);
        size_t comma_pos = str.find(",", colon_pos + 1);
        size_t quote_pos = str.find("\"", colon_pos + 1);

        std::string_view name{""};
        if (quote_pos < comma_pos)
        {
            name = str.substr(colon_pos + 1, quote_pos - colon_pos - 1);
        }
        else
        {
            name = str.substr(colon_pos + 1, comma_pos - colon_pos - 1);
        }
        name = Strip(name);

        size_t next_pos = str.find(key, pos + 1);
        while (next_pos < str.size())
        {
            size_t next_colon = str.find(":", next_pos + 1);
            if (std::string_view::npos == next_colon)
            {
                return -3;
            }
            size_t next_comma = str.find(",", next_pos + 1);
            if (std::string_view::npos == next_comma)
            {
                next_comma = str.find("\"", next_pos + 1);
            }

            std::string_view next_name = str.substr(next_colon + 1, next_comma - next_colon - 1);
            next_name = Strip(next_name);

            if (name == next_name)
            {
                return -3;
            }

            next_pos = str.find(key, next_pos + 1);
        }
        pos = str.find(key, pos + 1);
    }

    return 0;
}

} // namespace config

#endif // AIR_CONFIG_CHECKER_LIB_H
