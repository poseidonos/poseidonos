
#include "src/collection/CollectionManager.h"

int
collection::Subject::Notify(uint32_t index, uint32_t type1, uint32_t type2,
    uint32_t value1, uint32_t value2, int pid,
    int cmd_type, int cmd_order)
{
    if (index < to_dtype(pi::CollectionSubject::COUNT))
    {
        arr_observer[index]->Update(type1, type2, value1, value2, pid, cmd_type,
            cmd_order);
        return 0;
    }
    return -1;
}

collection::CollectionManager::~CollectionManager(void)
{
    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        if (nullptr != collector[i])
        {
            delete collector[i];
        }
    }
}

void
collection::CollectionManager::Init(void)
{
    air_enable = false;
    if (global_meta_getter->Enable())
    {
        air_enable = true;
    }
    max_aid_size = global_meta_getter->AidSize();

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        node_enable[i] = false;
        if (node_meta_getter->NodeEnable(i))
        {
            node_enable[i] = true;
        }

        switch (node_meta_getter->NodeProcessorType(i))
        {
            case (air::ProcessorType::PERFORMANCE):
                collector[i] = new PerformanceCollector{new PerformanceWriter};
                break;
            case (air::ProcessorType::LATENCY):
                collector[i] = new LatencyCollector{new LatencyWriter};
                break;
            case (air::ProcessorType::QUEUE):
                collector[i] = new QueueCollector{new QueueWriter};
                break;
            default:
                // fix me
                collector[i] = nullptr;
                break;
        }
    }
}

void
collection::CollectionManager::HandleMsg(void)
{
    while (!msg.empty())
    {
        lib::MsgEntry entry = msg.front();
        msg.pop();
        int result =
            UpdateCollection(entry.type1, entry.type2, entry.value1, entry.value2);
        int ret_code = 0;
        if (0 > result)
        {
            ret_code = result * -1;
        }
        subject->Notify(to_dtype(pi::CollectionSubject::TO_OUTPUT),
            to_dtype(pi::Type1::COLLECTION_TO_OUTPUT), 0, ret_code, 0,
            entry.pid, entry.cmd_type, entry.cmd_order);
    }
}

int
collection::CollectionManager::UpdateCollection(uint32_t type1,
    uint32_t type2,
    uint32_t value1,
    uint32_t value2)
{
    int result{0}; // SUCCESS

    switch (type2)
    {
        case (to_dtype(pi::Type2::ENABLE_AIR)):
            if (1 == value1)
            {
                air_enable = true;
            }
            else if (0 == value1)
            {
                air_enable = false;
            }
            else
            {
                result = -1;
            }
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE)):
        case (to_dtype(pi::Type2::ENABLE_NODE_ALL)):
        case (to_dtype(pi::Type2::ENABLE_NODE)):
        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP)):
            result = _UpdateEnable(type1, type2, value1, value2);
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE_ALL)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE)):
        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP)):
            result = _UpdateInit(type1, type2, value1, value2);
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE)):
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP)):
            result = _UpdateSamplingRate(type1, type2, value1, value2);
            break;

        default:
            result = -1;
            break;
    }

    return result;
}

int
collection::CollectionManager::_EnableNode(uint32_t node_index,
    uint32_t is_run)
{
    if (to_dtype(pi::OnOff::ON) == is_run)
    {
        node_enable[node_index] = true;
    }
    else if (to_dtype(pi::OnOff::OFF) == is_run)
    {
        node_enable[node_index] = false;
    }
    else
    {
        return -1;
    }

    return 0;
}

int
collection::CollectionManager::_EnableRangeNode(uint32_t start_idx,
    uint32_t end_idx,
    uint32_t is_run)
{
    for (uint32_t i = start_idx; i <= end_idx; i++)
    {
        if (-1 == _EnableNode(i, is_run))
        {
            return -1;
        }
    }

    return 0;
}

int
collection::CollectionManager::_EnableGroupNode(uint32_t gid,
    uint32_t is_run)
{
    for (uint32_t i = 0; i < cfg::GetArrSize(config::ConfigType::NODE); i++)
    {
        if ((uint32_t)node_meta_getter->NodeGroupId(i) == gid)
        {
            if (0 != _EnableNode(i, is_run))
            {
                return -1;
            }
        }
    }

    return 0;
}

int
collection::CollectionManager::_UpdateEnable(uint32_t type1, uint32_t type2,
    uint32_t value1,
    uint32_t value2)
{
    // value1: enable/disable, value2: node info(node id, range, group id)
    int result{0};
    uint32_t upper_bit = (value2 >> 16) & 0x0000FFFF;
    uint32_t lower_bit = value2 & 0x0000FFFF;

    switch (type2)
    {
        case (to_dtype(pi::Type2::ENABLE_NODE)):
            result = _EnableNode(value2, value1);
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP)):
            result = _EnableGroupNode(value2, value1);
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE)):
            result = _EnableRangeNode(upper_bit, lower_bit, value1);
            break;

        case (to_dtype(pi::Type2::ENABLE_NODE_ALL)):
            result = _EnableRangeNode(0, MAX_NID_SIZE - 1, value1);
            break;
    }

    return result;
}

void
collection::CollectionManager::_InitNode(uint32_t node_index)
{
    if (air::ProcessorType::LATENCY ==
        node_meta_getter->NodeProcessorType(node_index))
    {
        uint32_t max_aid_size = global_meta_getter->AidSize();
        for (uint32_t aid = 0; aid < max_aid_size; aid++)
        {
            collector[node_index]->InformInit(
                node_manager->GetAccLatData(node_index, aid));
        }
    }
    else
    {
        for (auto j = node_manager->thread_map.begin(); j != node_manager->thread_map.end(); ++j)
        {
            node::ThreadArray* arr = &(j->second);
            node::Thread* thread = arr->node[node_index];
            if (nullptr != thread)
            {
                for (uint32_t k = 0; k < max_aid_size; k++)
                {
                    if (nullptr != collector[node_index])
                    {
                        collector[node_index]->InformInit(thread->GetAccData(k));
                    }
                }
            }
        }
    }
}

int
collection::CollectionManager::_UpdateInit(uint32_t type1, uint32_t type2,
    uint32_t value1,
    uint32_t value2)
{
    // value1: node info(node id, range, group id)
    uint32_t upper_bit = (value1 >> 16) & 0x0000FFFF;
    uint32_t lower_bit = value1 & 0x0000FFFF;
    switch (type2)
    {
        case (to_dtype(pi::Type2::INITIALIZE_NODE)):
            _InitNode(value1);
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE)):
            for (uint32_t i = upper_bit; i <= lower_bit; i++)
            {
                _InitNode(i);
            }
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP)):
            for (uint32_t i = 0; i < cfg::GetArrSize(config::ConfigType::NODE); i++)
            {
                if ((uint32_t)node_meta_getter->NodeGroupId(i) == value1)
                {
                    _InitNode(i);
                }
            }
            break;

        case (to_dtype(pi::Type2::INITIALIZE_NODE_ALL)):
            for (uint32_t i = 0; i <= MAX_NID_SIZE - 1; i++)
            {
                _InitNode(i);
            }
            break;
    }

    return 0;
}

int
collection::CollectionManager::_UpdateNodeSamplingRate(uint32_t node_index,
    uint32_t new_ratio)
{
    if (collector[node_index] != nullptr)
    {
        return collector[node_index]->SetSamplingRate(new_ratio);
    }
    return 0;
}

int
collection::CollectionManager::_UpdateSamplingRate(uint32_t type1,
    uint32_t type2,
    uint32_t value1,
    uint32_t value2)
{
    // value1: rate, value2: node info(node id, range, group id)
    uint32_t lower_bit = value2 & 0x0000FFFF;
    uint32_t upper_bit = (value2 >> 16) & 0x0000FFFF;
    int result{0};

    switch (type2)
    {
        case (to_dtype(pi::Type2::SET_SAMPLING_RATE)):
            result = _UpdateNodeSamplingRate(value2, value1);
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE)):
            for (uint32_t i = upper_bit; i <= lower_bit; i++)
            {
                result = _UpdateNodeSamplingRate(i, value1);
                if (result != 0)
                {
                    break;
                }
            }
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP)):
            for (uint32_t i = 0; i < cfg::GetArrSize(config::ConfigType::NODE); i++)
            {
                result = _UpdateNodeSamplingRate(i, value1);
                if (result != 0)
                {
                    break;
                }
            }
            break;

        case (to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL)):
            for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
            {
                result = _UpdateNodeSamplingRate(i, value1);
                if (result != 0)
                {
                    break;
                }
            }
            break;
    }

    return result;
}
