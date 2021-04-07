
#ifndef AIR_CONFIG_CHECKER_H
#define AIR_CONFIG_CHECKER_H

#include <array>

#include "src/config/ConfigCheckerLib.h"
#include "src/config/ConfigCheckerLogic.h"
#include "src/config/ConfigLib.h"
#include "src/config/ConfigString.h"

namespace config
{
class ConfigChecker
{
public:
    ConfigChecker(void)
    {
    }
    virtual ~ConfigChecker(void)
    {
    }

    static constexpr int32_t
    CheckArrayRule(ConfigType type, String str)
    {
        if (0 > CheckArrayFormat(str))
        {
            return -1;
        }
        if (ConfigType::GROUP == type)
        {
            String key = "GroupName";
            if (0 > CheckNameDuplication(key, str))
            {
                return -2;
            }
        }
        else if (ConfigType::NODE == type)
        {
            String key = "NodeName";
            if (0 > CheckNameDuplication(key, str))
            {
                return -2;
            }
        }

        return 0;
    }

    static constexpr int32_t
    CheckKeyRule(ConfigType type, String str_entry)
    {
        uint32_t num_mandatory{num_mandatory_list[dtype(type)]};
        if (ConfigType::DEFAULT == type)
        {
            if (0 > CheckKeyTypo(type, default_keys, str_entry))
            {
                return -3;
            }
            if (0 > CheckMandatoryKey(default_keys, num_mandatory, str_entry))
            {
                return -4;
            }
        }
        else if (ConfigType::GROUP == type)
        {
            if (0 > CheckKeyTypo(type, group_keys, str_entry))
            {
                return -3;
            }
            if (0 > CheckMandatoryKey(group_keys, num_mandatory, str_entry))
            {
                return -4;
            }
        }
        else
        { // ConfigType::NODE
            if (0 > CheckKeyTypo(type, node_keys, str_entry))
            {
                return -3;
            }
            if (0 > CheckMandatoryKey(node_keys, num_mandatory, str_entry))
            {
                return -4;
            }
        }

        if (0 > CheckKeyDuplication(str_entry))
        {
            return -5;
        }

        return 0;
    }

    static constexpr int32_t
    CheckValueRule(ConfigType type, String str_entry)
    {
        if (0 > _CheckValueValidity(type, str_entry))
        {
            return -6;
        }

        return 0;
    }

    static constexpr int32_t
    CheckGroupNameInNode(String node_entry, String group_str_arr)
    {
        size_t node_group_name_pos = node_entry.Find("GroupName");
        if (npos == node_group_name_pos)
        {
            return 0;
        }
        size_t node_colon_pos = node_entry.Find(":", node_group_name_pos + 1);
        size_t node_comma_pos = node_entry.Find(",", node_group_name_pos + 1);
        if (npos == node_comma_pos)
        {
            node_comma_pos = node_entry.Size();
        }
        String node_group_name =
            node_entry.Substr(node_colon_pos + 1, node_comma_pos - node_colon_pos);
        node_group_name = Strip(node_group_name);

        size_t group_name_pos = group_str_arr.Find("GroupName");
        bool is_group_name = false;
        while (npos != group_name_pos)
        {
            size_t group_colon_pos = group_str_arr.Find(":", group_name_pos + 1);
            size_t group_comma_pos = group_str_arr.Find(",", group_name_pos + 1);
            size_t group_quote_pos = group_str_arr.Find("\"", group_name_pos + 1);
            size_t token_pos = 0;
            if (group_comma_pos < group_quote_pos)
            {
                token_pos = group_comma_pos;
            }
            else
            {
                token_pos = group_quote_pos;
            }
            String group_name = group_str_arr.Substr(group_colon_pos + 1,
                token_pos - group_colon_pos);
            group_name = Strip(group_name);

            if (group_name == node_group_name)
            {
                is_group_name = true;
                break;
            }

            group_name_pos = group_str_arr.Find("GroupName", group_name_pos + 1);
        }

        if (false == is_group_name)
        {
            return -3;
        }

        return 0;
    }

private:
    static constexpr bool
    _CheckNoSpace(String str)
    {
        size_t pos{0};
        while (str.Size() > pos)
        {
            if (' ' == str[pos])
            {
                return false;
            }
            pos += 1;
        }
        return true;
    }

    static constexpr int32_t
    _CheckValueValidity(ConfigType type, String entry)
    {
        size_t colon_pos{entry.Find(":")};
        size_t prev_comma{0};
        size_t post_comma{entry.Find(",")};

        while (colon_pos < entry.Size())
        {
            String key = entry.Substr(prev_comma, colon_pos - prev_comma + 1);
            key = Strip(key);

            if (post_comma > entry.Size())
            {
                post_comma = entry.Size();
            }
            String value = entry.Substr(colon_pos + 1, post_comma - colon_pos);
            value = Strip(value);

            int32_t result{0};
            if (ConfigType::DEFAULT == type)
            {
                result = ConfigCheckerLogic::CheckDefaultValue(key, value);
            }
            else if (ConfigType::GROUP == type)
            {
                result = ConfigCheckerLogic::CheckGroupNodeValue(key, value);
            }
            else
            { // ConfigType::NODE
                result = ConfigCheckerLogic::CheckGroupNodeValue(key, value);
            }

            if (0 > result)
            {
                return -3;
            }

            prev_comma = post_comma + 1;
            colon_pos = entry.Find(":", colon_pos + 1);
            post_comma = entry.Find(",", colon_pos + 1);

            if (post_comma >= entry.Size())
            {
                post_comma = entry.Size();
            }
        }

        return 0;
    }

    static constexpr String default_keys[NUM_DEFAULT_KEY]{
        "StreamingInterval", "AirBuild", "NodeBuild",
        "NodeRun", "SamplingRatio", "AidSize"};
    static constexpr String group_keys[NUM_GROUP_KEY]{
        "GroupName", "NodeBuild", "NodeRun",
        "SamplingRatio", "Condition", "Delegation"};
    static constexpr String node_keys[NUM_NODE_KEY]{
        "NodeName", "Type", "GroupName", "NodeBuild",
        "NodeRun", "SamplingRatio", "Condition", "Delegation"};

    static constexpr uint32_t num_mandatory_list[3]{6, 1, 3};
};

} // namespace config

#endif // AIR_CONFIG_CHECKER_H
