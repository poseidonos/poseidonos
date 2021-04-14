
#ifndef AIR_CONFIG_INTERFACE_H
#define AIR_CONFIG_INTERFACE_H

#include <string_view>

#include "src/config/ConfigLib.h"
#include "src/config/ConfigParser.h"

namespace config
{
class ConfigInterface
{
public:
    ConfigInterface(void)
    {
    }
    virtual ~ConfigInterface(void)
    {
    }

    static constexpr int32_t
    CheckConfigRule(void)
    {
        return Config::CheckRule();
    }

    static constexpr int32_t
    GetIndex(ConfigType type, std::string_view name = "")
    {
        return config.GetIndexFromStrArr(type, name);
    }

    static constexpr int32_t
    GetIntValue(ConfigType type, std::string_view key, int index_c)
    {
        int32_t index = index_c;
        std::string_view entry = config.GetEntryFromStrArr(type, index);
        int32_t ret = config.GetIntValueFromEntry(entry, key);
        if (0 > ret &&
            (key == "NodeBuild" || key == "NodeRun" || key == "SamplingRatio"))
        {
            if (ConfigType::NODE == type)
            {
                std::string_view group_name = config.GetStrValueFromEntry(entry, "GroupName");
                index = config.GetIndexFromStrArr(ConfigType::GROUP, group_name);
                entry = config.GetEntryFromStrArr(ConfigType::GROUP, index);
                ret = config.GetIntValueFromEntry(entry, key);
                if (ret < 0)
                {
                    entry = config.GetEntryFromStrArr(ConfigType::DEFAULT, 0);
                    ret = config.GetIntValueFromEntry(entry, key);
                }
            }
            else
            {
                entry = config.GetEntryFromStrArr(ConfigType::DEFAULT, 0);
                ret = config.GetIntValueFromEntry(entry, key);
            }
        }
        return ret;
    }

    static constexpr int32_t
    GetIntValue(ConfigType type, std::string_view key, std::string_view name = "")
    {
        int32_t index = config.GetIndexFromStrArr(type, name);
        return GetIntValue(type, key, index);
    }

    static constexpr std::string_view
    GetStrValue(ConfigType type, std::string_view key, std::string_view name = "")
    {
        int32_t index = config.GetIndexFromStrArr(type, name);
        std::string_view entry = config.GetEntryFromStrArr(type, index);
        return config.GetStrValueFromEntry(entry, key);
    }

    static constexpr std::string_view
    GetName(ConfigType type, uint32_t entry_index)
    {
        std::string_view key{""};
        if (ConfigType::GROUP == type)
        {
            key = "GroupName";
        }
        else if (ConfigType::NODE == type)
        {
            key = "NodeName";
        }
        else
        {
            return "";
        }

        std::string_view entry = config.GetEntryFromStrArr(type, entry_index);
        return config.GetStrValueFromEntry(entry, key);
    }

    static constexpr uint32_t
    GetArrSize(ConfigType type)
    {
        return config.GetArrSize(type);
    }

private:
    static Config config;
};

} // namespace config

typedef config::ConfigInterface cfg;

#endif // AIR_CONFIG_INTERFACE_H
