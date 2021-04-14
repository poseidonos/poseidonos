
#ifndef AIR_CONFIG_CHECKER_LOGIC_H
#define AIR_CONFIG_CHECKER_LOGIC_H

#include <string_view>

#include "src/config/ConfigLib.h"

namespace config
{
class ConfigCheckerLogic
{
public:
    ConfigCheckerLogic(void)
    {
    }
    virtual ~ConfigCheckerLogic(void)
    {
    }

    static constexpr int32_t
    CheckStreamingInterval(std::string_view value)
    {
        if (1 == value.size())
        {
            if ('0' > value[0] || '9' < value[0])
            {
                return -1;
            }
        }
        else if (2 == value.size())
        {
            if ('1' > value[0] || '3' < value[0])
            {
                return -1;
            }
            if ('0' > value[1] || '9' < value[1])
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }

        return 0;
    }

    static constexpr int32_t
    CheckSamplingRatio(std::string_view value)
    {
        if (1 <= value.size() && 5 >= value.size())
        {
            if ('0' == value[0] || '-' == value[0])
            {
                return -3;
            }
            for (uint32_t index = 1; index < value.size(); index++)
            {
                if ('0' > value[index] || '9' < value[index])
                {
                    return -3;
                }
            }
        }
        else
        {
            return -3;
        }

        return 0;
    }

    static constexpr int32_t
    CheckName(std::string_view value)
    {
        if (MAX_NAME_LEN < value.size())
        {
            return -3;
        }
        return 0;
    }

    static constexpr int32_t
    CheckType(std::string_view value)
    {
        if (value != "PERFORMANCE" && value != "LATENCY" && value != "QUEUE" && value != "UTILIZATION" && value != "COUNT")
        {
            return -3;
        }
        return 0;
    }

    static constexpr int32_t
    CheckBoolValue(std::string_view value)
    {
        if (value != "true" && value != "True" && value != "false" &&
            value != "False" && value != "On" && value != "on" && value != "Off" &&
            value != "off" && value != "TRUE" && value != "FALSE")
        {
            return -3;
        }
        return 0;
    }

    static constexpr int32_t
    CheckAidSize(std::string_view value)
    {
        if (10 < value.size())
        {
            return -3;
        }
        else if (10 == value.size())
        {
            if ('4' < value[0] || '2' < value[1] || '9' < value[2] ||
                '4' < value[3] || '9' < value[4] || '6' < value[5] ||
                '7' < value[6] || '2' < value[7] || '9' < value[8] ||
                '5' < value[9])
            {
                return -3;
            }
        }
        else
        {
            for (uint32_t index = 0; index < value.size(); index++)
            {
                if ('0' > value[index] || '9' < value[index])
                {
                    return -3;
                }
            }
        }

        return 0;
    }

    static constexpr int32_t
    CheckDefaultValue(std::string_view key, std::string_view value)
    {
        if (key == "StreamingInterval")
        {
            return CheckStreamingInterval(value);
        }
        else if (key == "SamplingRatio")
        {
            return CheckSamplingRatio(value);
        }
        else if (key == "AidSize")
        {
            return CheckAidSize(value);
        }
        else
        { // NodeBuild, AirBuild, NodeRun
            return CheckBoolValue(value);
        }
    }

    static constexpr int32_t
    CheckGroupNodeValue(std::string_view key, std::string_view value)
    {
        if (key == "GroupName" || key == "NodeName")
        {
            return CheckName(value);
        }
        else if (key == "Type")
        {
            return CheckType(value);
        }
        else if (key == "SamplingRatio")
        {
            return CheckSamplingRatio(value);
        }
        else if (key == "Condition")
        {
            return 0;
        }
        else if (key == "Delegation")
        {
            return 0;
        }
        else
        { // NodeBuild, NodeRun
            return CheckBoolValue(value);
        }
    }

private:
    static constexpr uint32_t MAX_NAME_LEN{18};
};

} // namespace config

#endif // AIR_CONFIG_CHECKER_LOGIC_H
