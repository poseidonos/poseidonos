
#ifndef AIR_CONFIG_CHECKER_LOGIC_H
#define AIR_CONFIG_CHECKER_LOGIC_H

#include "src/config/ConfigLib.h"
#include "src/config/ConfigString.h"

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
    CheckStreamingInterval(String value)
    {
        if (1 == value.Size())
        {
            if ('0' > value[0] || '9' < value[0])
            {
                return -1;
            }
        }
        else if (2 == value.Size())
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
    CheckSamplingRatio(String value)
    {
        if (1 <= value.Size() && 5 >= value.Size())
        {
            if ('0' == value[0] || '-' == value[0])
            {
                return -3;
            }
            for (uint32_t index = 1; index < value.Size(); index++)
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
    CheckName(String value)
    {
        if (MAX_NAME_LEN < value.Size())
        {
            return -3;
        }
        return 0;
    }

    static constexpr int32_t
    CheckType(String value)
    {
        if (value != "PERFORMANCE" && value != "LATENCY" && value != "QUEUE")
        {
            return -3;
        }
        return 0;
    }

    static constexpr int32_t
    CheckBoolValue(String value)
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
    CheckAidSize(String value)
    {
        if (10 < value.Size())
        {
            return -3;
        }
        else if (10 == value.Size())
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
            for (uint32_t index = 0; index < value.Size(); index++)
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
    CheckDefaultValue(String key, String value)
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
    CheckGroupNodeValue(String key, String value)
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
