
#include "src/policy/Ruler.h"

#include "src/config/ConfigInterface.h"

int
policy::Ruler::_CheckEnableNodeRule(uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2)
{
    /*  type2: command, value1: bool, value2: node_id range  */
    int result{-1};
    uint32_t upper_bit, lower_bit;

    switch (type2)
    {
        case (to_dtype(pi::Type2::ENABLE_NODE)):
            result = 0;
            if (value2 >= MAX_NID_SIZE)
            {
                result = -11;
            }
            break;
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP)):
            result = 0;
            if (value2 >= cfg::GetSentenceCount(config::ParagraphType::GROUP))
            {
                result = -11;
            }
            break;
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE)):
            result = 0;
            lower_bit = value2 & 0x0000FFFF;
            upper_bit = (value2 >> 16) & 0x0000FFFF;
            if (lower_bit >= MAX_NID_SIZE)
            {
                result = -11;
            }
            if (upper_bit >= MAX_NID_SIZE)
            {
                result = -11;
            }
            if (lower_bit < upper_bit)
            {
                result = -11;
            }
            break;
        case (to_dtype(pi::Type2::ENABLE_NODE_ALL)):
            result = 0;
            break;
    }

    return result;
}

int
policy::Ruler::_CheckInitNodeRule(uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2)
{
    /*  type2: command, value1: node_id range group_id  */
    int result{-1};
    uint32_t upper_bit, lower_bit;

    switch (type2)
    {
        case (to_dtype(pi::Type2::INITIALIZE_NODE)):
            result = 0;
            if (value1 >= MAX_NID_SIZE)
            {
                result = -11;
            }
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP)):
            result = 0;
            if (value1 >= cfg::GetSentenceCount(config::ParagraphType::GROUP))
            {
                result = -11;
            }
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE)):
            result = 0;
            lower_bit = value1 & 0x0000FFFF;
            upper_bit = (value1 >> 16) & 0x0000FFFF;
            if (lower_bit >= MAX_NID_SIZE)
            {
                result = -11;
            }
            if (lower_bit < upper_bit)
            {
                result = -11;
            }
            if (upper_bit >= MAX_NID_SIZE)
            {
                result = -11;
            }
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_ALL)):
            result = 0;
            break;
    }

    return result;
}

int
policy::Ruler::_CheckSetSamplingRatioRule(uint32_t type1, uint32_t type2,
    uint32_t value1,
    uint32_t value2)
{
    /*  type2: command, value1: ratio, value2: node_id range  */
    int result{-1};
    uint32_t upper_bit, lower_bit;

    if (1 > value1 || 10000 < value1)
    {
        result = -13;
        return result;
    }

    switch (type2)
    {
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE)):
            result = 0;
            if (value2 >= MAX_NID_SIZE)
            {
                result = -11;
            }
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP)):
            result = 0;
            if (value2 >= cfg::GetSentenceCount(config::ParagraphType::GROUP))
            {
                result = -11;
            }
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE)):
            result = 0;
            upper_bit = (value2 >> 16) & 0x0000FFFF;
            lower_bit = value2 & 0x0000FFFF;
            if (upper_bit >= MAX_NID_SIZE)
            {
                result = -11;
            }
            if (lower_bit >= MAX_NID_SIZE)
            {
                result = -11;
            }
            if (lower_bit < upper_bit)
            {
                result = -11;
            }
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL)):
            result = 0;
            break;
    }

    return result;
}

int
policy::Ruler::_CheckStreamInterval(uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2)
{
    /*  value1: interval  */
    int result{0};
    if (0 >= value1 || 30 < value1)
    {
        result = -12;
    }

    return result;
}

int
policy::Ruler::CheckRule(uint32_t type1, uint32_t type2, uint32_t value1,
    uint32_t value2)
{
    int result{0};

    switch (type2)
    {
        case (to_dtype(pi::Type2::ENABLE_AIR)):
            break;

        case (to_dtype(pi::Type2::SET_STREAMING_INTERVAL)):
            result = _CheckStreamInterval(type1, type2, value1, value2);
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE)):
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE)):
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP)):
        case (to_dtype(pi::Type2::ENABLE_NODE_ALL)):
            result = _CheckEnableNodeRule(type1, type2, value1, value2);
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE_ALL)):
            result = _CheckInitNodeRule(type1, type2, value1, value2);
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL)):
            result = _CheckSetSamplingRatioRule(type1, type2, value1, value2);
            break;
        default:
            result = -1; // command not found
            break;
    }

    return result;
}

bool
policy::Ruler::_SetEnableNodeRule(uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2)
{
    bool result{false};
    bool data;
    uint32_t i, upper_bit, lower_bit;

    switch (type2)
    {
        case (to_dtype(pi::Type2::ENABLE_NODE)):
            data = (bool)value1;
            if (node_meta->Run(value2) != data)
            {
                node_meta->SetRun(value2, data);
            }
            result = true;
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE)):
            data = (bool)value1;
            upper_bit = (value2 >> 16) & 0x0000FFFF;
            lower_bit = value2 & 0x0000FFFF;
            for (i = upper_bit; i <= lower_bit; i++)
            {
                if (node_meta->Run(i) != data)
                {
                    node_meta->SetRun(i, data);
                }
            }
            result = true;
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP)):
            data = (bool)value1;
            for (i = 0; i < cfg::GetSentenceCount(config::ParagraphType::NODE); i++)
            {
                if (node_meta->GroupId(i) == value2)
                {
                    if (node_meta->Run(i) != data)
                    {
                        node_meta->SetRun(i, data);
                    }
                }
            }
            result = true;
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_ALL)):
            data = (bool)value1;
            for (i = 0; i < MAX_NID_SIZE; i++)
            {
                if (node_meta->Run(i) != (bool)value1)
                {
                    node_meta->SetRun(i, (bool)value1);
                }
            }
            result = true;
            break;
    }

    return result;
}

bool
policy::Ruler::_SetSamplingRatioRule(uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2)
{
    /*  type2: command, value1: ratio, value2: node_id range*/
    bool result{false};
    uint32_t i, upper_bit, lower_bit;

    switch (type2)
    {
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE)):
            if (node_meta->SampleRatio(value2) != value1)
            {
                node_meta->SetSampleRatio(value2, value1);
            }
            result = true;
            break;
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE)):
            upper_bit = (value2 >> 16) & 0x0000FFFF;
            lower_bit = value2 & 0x0000FFFF;
            for (i = upper_bit; i <= lower_bit; i++)
            {
                if (node_meta->SampleRatio(i) != value1)
                {
                    node_meta->SetSampleRatio(i, value1);
                }
            }
            result = true;
            break;
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP)):
            for (i = 0; i < cfg::GetSentenceCount(config::ParagraphType::NODE); i++)
            {
                if ((uint32_t)node_meta->GroupId(i) == value2)
                {
                    if (node_meta->SampleRatio(i) != value1)
                    {
                        node_meta->SetSampleRatio(i, value1);
                    }
                }
            }
            result = true;
            break;
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL)):
            for (i = 0; i < MAX_NID_SIZE; i++)
            {
                if (node_meta->SampleRatio(i) != value1)
                {
                    node_meta->SetSampleRatio(i, value1);
                }
            }
            result = true;
            break;
    }

    return result;
}

bool
policy::Ruler::SetRule(uint32_t type1, uint32_t type2, uint32_t value1,
    uint32_t value2)
{
    bool result{false};
    bool data;

    switch (type2)
    {
        case (to_dtype(pi::Type2::ENABLE_AIR)):
            data = (bool)value1;
            if (global_meta->AirPlay() != data)
            {
                global_meta->SetAirPlay(data);
            }
            result = true;

            break;

        case (to_dtype(pi::Type2::SET_STREAMING_INTERVAL)):
            if (global_meta->StreamingInterval() != value1)
            {
                global_meta->SetStreamingInterval(value1);
            }
            result = true;
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE)):
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE)):
        case (to_dtype(pi::Type2::ENABLE_NODE_ALL)):
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP)):
            result = _SetEnableNodeRule(type1, type2, value1, value2);
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_ALL)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP)):
            result = true;
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP)):
            result = _SetSamplingRatioRule(type1, type2, value1, value2);
            break;
    }

    return result;
}
