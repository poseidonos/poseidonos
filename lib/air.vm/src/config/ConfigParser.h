
#ifndef AIR_CONFIG_H
#define AIR_CONFIG_H

#include <string_view>

#include "src/config/ConfigChecker.h"
#include "src/config/ConfigLib.h"

#define AIR_CFG_FILE <AIR_CFG>

namespace config
{
struct ConfigReader
{
    ConfigReader(void)
    {
    }
    ~ConfigReader(void)
    {
    }
    static constexpr uint32_t
    GetStrArrSize(std::string_view cfg_str)
    {
        size_t count{0};
        size_t pos{0};

        pos = cfg_str.find("\"");
        while (std::string_view::npos != pos)
        {
            pos = cfg_str.find("\"", pos + 1);
            count++;
        }

        return (count / 2);
    }

    static constexpr std::string_view
    GetStrArrFromRawData(std::string_view start_del, std::string_view end_del)
    {
        size_t start_pos = raw_data.find(start_del) + start_del.size() + 1;
        size_t end_pos = raw_data.find(end_del);
        return raw_data.substr(start_pos, end_pos - start_pos);
    }

private:
    static constexpr std::string_view raw_data =
#include AIR_CFG_FILE
        ;
};

class Config
{
public:
    Config(void)
    {
    }
    virtual ~Config(void)
    {
    }

    static constexpr int32_t
    GetIntValueFromEntry(std::string_view entry, std::string_view key)
    {
        if (key != "AirBuild" && key != "NodeBuild" && key != "NodeRun" &&
            key != "AidSize" && key != "StreamingInterval" &&
            key != "SamplingRatio")
        {
            return -1;
        }

        size_t start_pos = entry.find(key);

        if (std::string_view::npos == start_pos)
        {
            return -1;
        }

        size_t comma_pos = entry.find(",", start_pos + 1);
        if (comma_pos > entry.size())
        {
            comma_pos = entry.size();
        }
        std::string_view key_value = entry.substr(start_pos, comma_pos - start_pos);
        size_t colon_pos = key_value.find(":");

        std::string_view value =
            key_value.substr(colon_pos + 1, key_value.size() - colon_pos);
        value = Strip(value);

        if (key == "AirBuild" || key == "NodeBuild" || key == "NodeRun")
        {
            if (value == "True" || value == "true" || value == "On" || value == "on")
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

        return _Stoi(value);
    }

    static constexpr std::string_view
    GetStrValueFromEntry(std::string_view entry, std::string_view key)
    {
        if (key != "GroupName" && key != "NodeName" && key != "Type" &&
            key != "NodeType" && key != "Item")
        {
            return "";
        }

        size_t start_pos = entry.find(key);
        if (std::string_view::npos == start_pos)
        {
            return "";
        }

        size_t comma_pos = entry.find(",", start_pos + 1);
        if (std::string_view::npos == comma_pos)
        {
            comma_pos = entry.size();
        }

        size_t colon_pos = entry.find(":", start_pos + 1);
        std::string_view value = entry.substr(colon_pos + 1, comma_pos - colon_pos - 1);
        value = Strip(value);

        return value;
    }

    static constexpr std::string_view
    GetEntryFromStrArr(ConfigType type, uint32_t index = 0)
    {
        std::string_view cfg_str = str_arr[dtype(type)];
        uint32_t cfg_size = arr_size[dtype(type)];

        if (cfg_size <= index)
        {
            return "";
        }

        size_t skip_count = 2 * index;
        size_t start_pos = cfg_str.find("\"");
        while (skip_count)
        {
            start_pos = cfg_str.find("\"", start_pos + 1);
            skip_count--;
        }
        size_t end_pos = cfg_str.find("\"", start_pos + 1);

        return cfg_str.substr(start_pos + 1, end_pos - start_pos - 1);
    }

    static constexpr int32_t
    GetIndexFromStrArr(ConfigType type, std::string_view name = "")
    {
        std::string_view cfg_str{""};
        std::string_view key_name{""};

        if (ConfigType::GROUP == type)
        {
            cfg_str = str_arr[dtype(ConfigType::GROUP)];
            key_name = "GroupName";
        }
        else if (ConfigType::NODE == type)
        {
            cfg_str = str_arr[dtype(ConfigType::NODE)];
            key_name = "NodeName";
        }
        else
        {
            return 0;
        }

        size_t start_kv = cfg_str.find(key_name);
        size_t colon_pos = cfg_str.find(":");
        size_t separator_pos = cfg_str.find(",");
        size_t count = 0;
        while (std::string_view::npos != start_kv)
        {
            size_t quote_pos = cfg_str.find("\"", colon_pos + 1);
            if (std::string_view::npos == separator_pos || quote_pos <= separator_pos)
            {
                separator_pos = quote_pos;
            }

            std::string_view key = cfg_str.substr(start_kv, colon_pos - start_kv + 1);
            key = Strip(key);
            std::string_view value = cfg_str.substr(colon_pos + 1, separator_pos - colon_pos - 1);
            value = Strip(value);

            if (0 == name.compare(value))
            {
                return count;
            }

            start_kv = cfg_str.find(key_name, start_kv + 1);
            colon_pos = cfg_str.find(":", start_kv + 1);
            separator_pos = cfg_str.find(",", start_kv + 1);
            count += 1;
        }

        return -1;
    }

    static constexpr uint32_t
    GetArrSize(ConfigType type)
    {
        return arr_size[dtype(type)];
    }

    static constexpr int32_t
    CheckRule(void)
    {
        int32_t result{0};

        result = _CheckDefaultRule();
        if (0 != result)
        {
            return result;
        }

        result = _CheckGroupRule();
        if (0 != result)
        {
            return result;
        }

        result = _CheckNodeRule();
        if (0 != result)
        {
            return result;
        }

        return 0;
    }

private:
    static constexpr int32_t
    _CheckDefaultRule(void)
    {
        return _CheckRule(dtype(ConfigType::DEFAULT));
    }

    static constexpr int32_t
    _CheckGroupRule(void)
    {
        if (0 == arr_size[dtype(ConfigType::GROUP)])
        {
            return 0;
        }
        else
        {
            return _CheckRule(dtype(ConfigType::GROUP));
        }
    }

    static constexpr int32_t
    _CheckNodeRule(void)
    {
        return _CheckRule(dtype(ConfigType::NODE));
    }

    static constexpr int32_t
    _CheckRule(uint32_t cfg_index)
    {
        int32_t result = ConfigChecker::CheckArrayRule(
            static_cast<ConfigType>(cfg_index), str_arr[cfg_index]);
        if (result < 0)
        {
            return result;
        }

        for (uint32_t entry_index = 0; entry_index < arr_size[cfg_index]; entry_index++)
        {
            result = ConfigChecker::CheckKeyRule(
                static_cast<ConfigType>(cfg_index),
                GetEntryFromStrArr(static_cast<ConfigType>(cfg_index), entry_index));
            if (result < 0)
            {
                return result;
            }

            result = ConfigChecker::CheckValueRule(
                static_cast<ConfigType>(cfg_index),
                GetEntryFromStrArr(static_cast<ConfigType>(cfg_index), entry_index));
            if (result < 0)
            {
                return result;
            }

            if (dtype(ConfigType::NODE) == cfg_index)
            {
                result = ConfigChecker::CheckGroupNameInNode(
                    GetEntryFromStrArr(ConfigType::NODE, entry_index),
                    str_arr[dtype(ConfigType::GROUP)]);
                if (result < 0)
                {
                    return result;
                }
            }
        }

        return 0;
    }

    static constexpr int32_t
    _PowerOfTen(int32_t pow)
    {
        int32_t result = 1;
        if (0 < pow)
        {
            for (int32_t i = 0; i < pow; i++)
            {
                result *= 10;
            }
        }
        return result;
    }

    static constexpr int32_t
    _Stoi(std::string_view str)
    {
        int32_t length = str.size();
        int32_t int_value = 0;
        for (int32_t i = 0; i < length; i++)
        {
            int32_t num = str[i] - '0';
            int_value += num * _PowerOfTen(length - i - 1);
        }
        return int_value;
    }

    static constexpr std::string_view str_arr[dtype(ConfigType::COUNT)] = {
        ConfigReader::GetStrArrFromRawData("[DEFAULT]", "[/DEFAULT]"),
        ConfigReader::GetStrArrFromRawData("[GROUP]", "[/GROUP]"),
        ConfigReader::GetStrArrFromRawData("[NODE]", "[/NODE]")};

    static constexpr uint32_t arr_size[dtype(ConfigType::COUNT)] = {
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::DEFAULT)]),
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::GROUP)]),
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::NODE)])};
};

} // namespace config

#endif // AIR_CONFIG_H
