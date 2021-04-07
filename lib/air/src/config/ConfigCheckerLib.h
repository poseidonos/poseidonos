
#ifndef AIR_CONFIG_CHECKER_LIB_H
#define AIR_CONFIG_CHECKER_LIB_H

#include <cstdint>

#include "src/config/ConfigLib.h"
#include "src/config/ConfigString.h"

namespace config
{
static constexpr uint32_t NUM_DEFAULT_KEY{6};
static constexpr uint32_t NUM_GROUP_KEY{6};
static constexpr uint32_t NUM_NODE_KEY{8};

static constexpr int32_t
CheckKeyTypo(ConfigType type, const String* key_list, String entry)
{
    size_t cur_pos{entry.Find(":")};
    size_t prev_pos{0};
    bool is_type_violation = false;
    bool has_sampling_ratio = false;

    while (npos != cur_pos)
    {
        String key = entry.Substr(prev_pos, cur_pos - prev_pos + 1);
        key = Strip(key);
        if (key == "SamplingRatio")
        {
            has_sampling_ratio = true;
        }
        if (key == "Type")
        {
            size_t next_comma = entry.Find(",", cur_pos + 1);
            if (npos == next_comma)
            {
                next_comma = entry.Size();
            }
            String value = entry.Substr(cur_pos + 1, next_comma - cur_pos);
            value = Strip(value);
            if (value == "PERFORMANCE" || value == "LATENCY")
            {
                is_type_violation = true;
            }
        }

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
        else
        {
            for (uint32_t i = 0; i < NUM_NODE_KEY; i++)
            {
                if (key == key_list[i])
                {
                    find = true;
                    break;
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

        prev_pos = entry.Find(",", prev_pos + 1);
        prev_pos += 1;
        cur_pos = entry.Find(":", cur_pos + 1);
    }

    return 0;
}

static constexpr int32_t
CheckMandatoryKey(const String* key_list, uint32_t num_mandatory, String entry)
{
    int key_cnt = 0;

    uint32_t num_find = 0;
    for (uint32_t index = 0; index < num_mandatory; index++, key_cnt++)
    {
        size_t cur_pos = entry.Find(":");
        size_t prev_pos{0};
        String mandatory_key = key_list[key_cnt];
        bool find{false};

        while (npos != cur_pos)
        {
            String entry_key = entry.Substr(prev_pos, cur_pos - prev_pos + 1);
            entry_key = Strip(entry_key);

            if (mandatory_key == entry_key)
            {
                find = true;
                num_find += 1;
                break;
            }

            prev_pos = entry.Find(",", prev_pos + 1);
            prev_pos += 1;
            cur_pos = entry.Find(":", cur_pos + 1);
        }

        if (false == find)
        {
            return -3;
        }
    }

    return 0;
}

static constexpr int32_t
CheckKeyDuplication(String entry)
{
    size_t colon_pos{entry.Find(":")};
    size_t comma_pos{0};

    while (npos != colon_pos)
    {
        String target_key = entry.Substr(comma_pos, colon_pos - comma_pos);
        target_key = Strip(target_key);

        size_t comparative_colon{entry.Find(":", colon_pos + 1)};
        size_t comparative_comma{entry.Find(",", comma_pos + 1)};
        comparative_comma += 1;
        while (npos != comparative_colon)
        {
            String comparative_key = entry.Substr(comparative_comma, comparative_colon - comparative_comma);
            comparative_key = Strip(comparative_key);

            if (target_key == comparative_key)
            {
                return -3;
            }

            comparative_comma = entry.Find(",", comparative_comma + 1);
            comparative_comma += 1;
            comparative_colon = entry.Find(":", comparative_colon + 1);
        }

        comma_pos = entry.Find(",", comma_pos + 1);
        comma_pos += 1;
        colon_pos = entry.Find(":", colon_pos + 1);
    }
    return 0;
}

static constexpr int32_t
CheckEntryFormat(String str)
{
    size_t start_quote = str.Find("\"");
    size_t end_quote = str.Find("\"", start_quote + 1);
    size_t comment_pos = str.Find("//");

    if (comment_pos > start_quote && comment_pos < end_quote)
    {
        return -3;
    }

    size_t curr_colon_pos{0};
    size_t curr_comma_pos{0};
    int colon_cnt{0};
    int comma_cnt{0};

    while (curr_colon_pos < str.Size() && curr_comma_pos < str.Size())
    {
        curr_colon_pos = str.Find(":", curr_colon_pos);
        if (npos == curr_colon_pos)
        {
            break;
        }
        colon_cnt++;

        curr_comma_pos = str.Find(",", curr_comma_pos);
        if (npos == curr_comma_pos)
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
    while (prev_comma_pos < str.Size())
    {
        curr_colon_pos = str.Find(":", prev_comma_pos);
        curr_comma_pos = str.Find(",", prev_comma_pos);
        if (npos == curr_comma_pos)
        {
            curr_comma_pos = end_quote;
        }

        String key =
            str.Substr(prev_comma_pos + 1, curr_colon_pos - prev_comma_pos);
        String value =
            str.Substr(curr_colon_pos + 1, curr_comma_pos - curr_colon_pos);
        key = Strip(key);
        value = Strip(value);

        if (0 == key.Size() || 0 == value.Size())
        {
            return -3;
        }

        prev_comma_pos = curr_comma_pos + 1;
    }

    return 0;
}

static constexpr int32_t
CheckArrayFormat(String str)
{
    size_t start_quote{0};
    size_t end_quote{0};
    size_t curr_pos{0};
    size_t start_cnt{0};
    size_t end_cnt{0};

    while (curr_pos < str.Size())
    {
        start_quote = str.Find("\"", curr_pos);
        if (npos == start_quote)
        {
            break;
        }
        start_cnt++;

        end_quote = str.Find("\"", start_quote + 1);
        if (npos == end_quote)
        {
            break;
        }
        end_cnt++;

        if (0 != CheckEntryFormat(str.Substr(start_quote, end_quote - start_quote + 2)))
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
CheckNameDuplication(String key, String str)
{
    size_t pos = str.Find(key);

    while (pos < str.Size())
    {
        size_t colon_pos = str.Find(":", pos + 1);
        size_t comma_pos = str.Find(",", colon_pos + 1);
        size_t quote_pos = str.Find("\"", colon_pos + 1);

        String name{""};
        if (quote_pos < comma_pos)
        {
            name = str.Substr(colon_pos + 1, quote_pos - colon_pos);
        }
        else
        {
            name = str.Substr(colon_pos + 1, comma_pos - colon_pos);
        }
        name = Strip(name);

        size_t next_pos = str.Find(key, pos + 1);
        while (next_pos < str.Size())
        {
            size_t next_colon = str.Find(":", next_pos + 1);
            if (npos == next_colon)
            {
                return -3;
            }
            size_t next_comma = str.Find(",", next_pos + 1);
            if (npos == next_comma)
            {
                next_comma = str.Find("\"", next_pos + 1);
            }

            String next_name = str.Substr(next_colon + 1, next_comma - next_colon);
            next_name = Strip(next_name);

            if (name == next_name)
            {
                return -3;
            }

            next_pos = str.Find(key, next_pos + 1);
        }
        pos = str.Find(key, pos + 1);
    }

    return 0;
}

} // namespace config

#endif // AIR_CONFIG_CHECKER_LIB_H
