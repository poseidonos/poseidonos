
#ifndef AIR_CONFIG_H
#define AIR_CONFIG_H

#include "src/config/ConfigChecker.h"
#include "src/config/ConfigLib.h"
#include "src/config/ConfigString.h"

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
    GetStrArrSize(String cfg_str)
    {
        size_t count{0};
        size_t pos{0};

        pos = cfg_str.Find("\"");
        while (npos != pos)
        {
            pos = cfg_str.Find("\"", pos + 1);
            count++;
        }

        return (count / 2);
    }

    static constexpr String
    GetStrArrFromRawData(String start_del,
        String end_del)
    {
        size_t start_pos = raw_data.Find(start_del) + start_del.Length() + 1;
        size_t end_pos = raw_data.Find(end_del);
        return raw_data.Substr(start_pos, end_pos - start_pos);
    }

private:
    static constexpr String raw_data =
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
    GetIntValueFromEntry(String entry, String key)
    {
        if (key != "AirBuild" && key != "NodeBuild" && key != "NodeRun" &&
            key != "AidSize" && key != "StreamingInterval" &&
            key != "SamplingRatio")
        {
            return -1;
        }

        size_t start_pos = entry.Find(key);

        if (npos == start_pos)
        {
            return -1;
        }

        size_t comma_pos = entry.Find(",", start_pos + 1);
        if (comma_pos > entry.Size())
        {
            comma_pos = entry.Size();
        }
        String key_value = entry.Substr(start_pos, comma_pos - start_pos + 1);
        size_t colon_pos = key_value.Find(":");

        String value =
            key_value.Substr(colon_pos + 1, key_value.Length() - colon_pos);
        value = Strip(value);

        if (key == "AirBuild" || key == "NodeBuild" || key == "NodeRun")
        {
            if (value == "True" || value == "true" || value == "On" ||
                value == "on")
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

    static constexpr String
    GetStrValueFromEntry(String entry, String key)
    {
        if (key != "GroupName" && key != "NodeName" && key != "Type" &&
            key != "NodeType" && key != "Item")
        {
            return "";
        }

        size_t start_pos = entry.Find(key);
        if (npos == start_pos)
        {
            return "";
        }

        size_t comma_pos = entry.Find(",", start_pos + 1);
        if (npos == comma_pos)
        {
            comma_pos = entry.Size();
        }

        size_t colon_pos = entry.Find(":", start_pos + 1);
        String value = entry.Substr(colon_pos + 1, comma_pos - colon_pos);
        value = Strip(value);

        return value;
    }

    static constexpr String
    GetEntryFromStrArr(ConfigType type,
        uint32_t index = 0)
    {
        String cfg_str = str_arr[dtype(type)];
        uint32_t cfg_size = arr_size[dtype(type)];

        if (cfg_size <= index)
        {
            return "";
        }

        size_t skip_count = 2 * index;
        size_t start_pos = cfg_str.Find("\"");
        while (skip_count)
        {
            start_pos = cfg_str.Find("\"", start_pos + 1);
            skip_count--;
        }
        size_t end_pos = cfg_str.Find("\"", start_pos + 1);

        return cfg_str.Substr(start_pos + 1, end_pos - start_pos);
    }

    static constexpr int32_t
    GetIndexFromStrArr(ConfigType type,
        String name = "")
    {
        String cfg_str{""};
        String key_name{""};

        if (ConfigType::DEFAULT == type)
        {
            return 0;
        }
        else if (ConfigType::GROUP == type)
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
        { // WEB
            cfg_str = str_arr[dtype(ConfigType::WEB)];
            key_name = "NodeType";
        }

        size_t start_kv = cfg_str.Find(key_name);
        size_t colon_pos = cfg_str.Find(":");
        size_t separator_pos = cfg_str.Find(",");
        size_t count = 0;
        while (npos != start_kv)
        {
            size_t quote_pos = cfg_str.Find("\"", colon_pos + 1);
            if (npos == separator_pos || quote_pos <= separator_pos)
            {
                separator_pos = quote_pos;
            }

            String key = cfg_str.Substr(start_kv, colon_pos - start_kv + 1);
            key = Strip(key);
            String value = cfg_str.Substr(colon_pos + 1, separator_pos - colon_pos);
            value = Strip(value);

            if (name == value)
            {
                return count;
            }

            start_kv = cfg_str.Find(key_name, start_kv + 1);
            colon_pos = cfg_str.Find(":", start_kv + 1);
            separator_pos = cfg_str.Find(",", start_kv + 1);
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
    _Stoi(String str)
    {
        int32_t length = str.Size();
        int32_t int_value = 0;
        for (int32_t i = 0; i < length; i++)
        {
            int32_t num = str[i] - '0';
            int_value += num * _PowerOfTen(length - i - 1);
        }
        return int_value;
    }

    static constexpr String str_arr[dtype(ConfigType::COUNT)] = {
        ConfigReader::GetStrArrFromRawData("[DEFAULT]", "[/DEFAULT]"),
        ConfigReader::GetStrArrFromRawData("[GROUP]", "[/GROUP]"),
        ConfigReader::GetStrArrFromRawData("[NODE]", "[/NODE]"),
        ConfigReader::GetStrArrFromRawData("[WEB]", "[/WEB]")};

    static constexpr uint32_t arr_size[dtype(ConfigType::COUNT)] = {
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::DEFAULT)]),
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::GROUP)]),
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::NODE)]),
        ConfigReader::GetStrArrSize(str_arr[dtype(ConfigType::WEB)])};
};

} // namespace config

#endif // AIR_CONFIG_H
