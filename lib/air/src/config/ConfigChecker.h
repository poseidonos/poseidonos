
#ifndef AIR_CONFIG_CHECKER_H
#define AIR_CONFIG_CHECKER_H

#include <array>
#include <string_view>

#include "src/config/ConfigCheckerLib.h"
#include "src/config/ConfigCheckerLogic.h"
#include "src/config/ConfigLib.h"

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
    CheckArrayRule(ConfigType type, std::string_view str)
    {
        if (0 > CheckArrayFormat(str))
        {
            return -1;
        }
        if (ConfigType::GROUP == type)
        {
            std::string_view key = "GroupName";
            if (0 > CheckNameDuplication(key, str))
            {
                return -2;
            }
        }
        else if (ConfigType::NODE == type)
        {
            std::string_view key = "NodeName";
            if (0 > CheckNameDuplication(key, str))
            {
                return -2;
            }
        }

        return 0;
    }

    static constexpr int32_t
    CheckKeyRule(ConfigType type, std::string_view str_entry)
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
    CheckValueRule(ConfigType type, std::string_view str_entry)
    {
        if (0 > _CheckValueValidity(type, str_entry))
        {
            return -6;
        }

        return 0;
    }

    static constexpr int32_t
    CheckGroupNameInNode(std::string_view node_entry, std::string_view group_str_arr)
    {
        size_t node_group_name_pos = node_entry.find("GroupName");
        if (std::string_view::npos == node_group_name_pos)
        {
            return 0;
        }
        size_t node_colon_pos = node_entry.find(":", node_group_name_pos + 1);
        size_t node_comma_pos = node_entry.find(",", node_group_name_pos + 1);
        if (std::string_view::npos == node_comma_pos)
        {
            node_comma_pos = node_entry.size();
        }
        std::string_view node_group_name =
            node_entry.substr(node_colon_pos + 1, node_comma_pos - node_colon_pos);
        node_group_name = Strip(node_group_name);

        size_t group_name_pos = group_str_arr.find("GroupName");
        bool is_group_name = false;
        while (std::string_view::npos != group_name_pos)
        {
            size_t group_colon_pos = group_str_arr.find(":", group_name_pos + 1);
            size_t group_comma_pos = group_str_arr.find(",", group_name_pos + 1);
            size_t group_quote_pos = group_str_arr.find("\"", group_name_pos + 1);
            size_t token_pos = 0;
            if (group_comma_pos < group_quote_pos)
            {
                token_pos = group_comma_pos;
            }
            else
            {
                token_pos = group_quote_pos;
            }
            std::string_view group_name = group_str_arr.substr(group_colon_pos + 1,
                token_pos - group_colon_pos - 1);
            group_name = Strip(group_name);

            if (group_name == node_group_name)
            {
                is_group_name = true;
                break;
            }

            group_name_pos = group_str_arr.find("GroupName", group_name_pos + 1);
        }

        if (false == is_group_name)
        {
            return -3;
        }

        return 0;
    }

private:
    static constexpr bool
    _CheckNoSpace(std::string_view str)
    {
        size_t pos{0};
        while (str.size() > pos)
        {
            if (' ' == str[pos] || '\t' == str[pos] || '\n' == str[pos] || '\v' == str[pos] || '\f' == str[pos] || '\r' == str[pos])
            {
                return false;
            }
            pos += 1;
        }
        return true;
    }

    static constexpr int32_t
    _CheckValueValidity(ConfigType type, std::string_view entry)
    {
        size_t colon_pos{entry.find(":")};
        size_t prev_comma{0};
        size_t post_comma{entry.find(",")};

        while (colon_pos < entry.size())
        {
            std::string_view key = entry.substr(prev_comma, colon_pos - prev_comma);
            key = Strip(key);

            if (post_comma > entry.size())
            {
                post_comma = entry.size();
            }
            std::string_view value = entry.substr(colon_pos + 1, post_comma - colon_pos - 1);
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
            colon_pos = entry.find(":", colon_pos + 1);
            post_comma = entry.find(",", colon_pos + 1);

            if (post_comma >= entry.size())
            {
                post_comma = entry.size();
            }
        }

        return 0;
    }

    static constexpr std::string_view default_keys[NUM_DEFAULT_KEY]{
        "StreamingInterval", "AirBuild", "NodeBuild",
        "NodeRun", "SamplingRatio", "AidSize"};
    static constexpr std::string_view group_keys[NUM_GROUP_KEY]{
        "GroupName", "NodeBuild", "NodeRun",
        "SamplingRatio", "Condition", "Delegation"};
    static constexpr std::string_view node_keys[NUM_NODE_KEY]{
        "NodeName", "Type", "GroupName", "NodeBuild",
        "NodeRun", "SamplingRatio", "Condition", "Delegation"};

    static constexpr uint32_t num_mandatory_list[3]{6, 1, 3};
};

} // namespace config

#endif // AIR_CONFIG_CHECKER_H
