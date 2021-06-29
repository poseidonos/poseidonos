#include "src/collection/CollectionManager.h"

#include <cassert>

#include "src/collection/writer/CountWriter.h"
#include "src/collection/writer/LatencyWriter.h"
#include "src/collection/writer/PerformanceWriter.h"
#include "src/collection/writer/QueueWriter.h"
#include "src/collection/writer/UtilizationWriter.h"

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
            collector[i] = nullptr;
        }
    }
}

void
collection::CollectionManager::Init(void)
{
    if (global_meta_getter->AirPlay())
    {
        air_run = true;
    }
    else
    {
        air_run = false;
    }

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        if (node_meta_getter->Run(i))
        {
            node_run[i] = true;
        }
        else
        {
            node_run[i] = false;
        }

        switch (node_meta_getter->ProcessorType(i))
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
            case (air::ProcessorType::UTILIZATION):
                collector[i] = new UtilizationCollector{new UtilizationWriter};
                break;
            case (air::ProcessorType::COUNT):
                collector[i] = new CountCollector{new CountWriter};
                break;
            default:
                assert(0);
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
                air_run = true;
            }
            else if (0 == value1)
            {
                air_run = false;
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
        node_run[node_index] = true;
    }
    else if (to_dtype(pi::OnOff::OFF) == is_run)
    {
        node_run[node_index] = false;
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
    for (uint32_t i = 0; i < cfg::GetSentenceCount(config::ParagraphType::NODE); i++)
    {
        if ((uint32_t)node_meta_getter->GroupId(i) == gid)
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
collection::CollectionManager::_InitNode(uint32_t nid)
{
    uint32_t index_size = node_meta_getter->IndexSize(nid);
    uint32_t filter_size = node_meta_getter->FilterSize(nid);

    if (nullptr == collector[nid])
    {
        return;
    }

    if (air::ProcessorType::LATENCY ==
        node_meta_getter->ProcessorType(nid))
    {
        for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
        {
            for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
            {
                collector[nid]->InformInit(node_manager->GetAccLatData(nid, hash_index, filter_index));
            }
        }
    }
    else
    {
        for (auto iter = node_manager->nda_map.begin(); iter != node_manager->nda_map.end(); ++iter)
        {
            node::NodeDataArray* arr = iter->second;
            node::NodeData* node_data = arr->node[nid];
            if (nullptr != node_data)
            {
                for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
                {
                    for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                    {
                        collector[nid]->InformInit(node_data->GetAccData(hash_index, filter_index));
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
            for (uint32_t i = 0; i < cfg::GetSentenceCount(config::ParagraphType::NODE); i++)
            {
                if ((uint32_t)node_meta_getter->GroupId(i) == value1)
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
            for (uint32_t i = 0; i < cfg::GetSentenceCount(config::ParagraphType::NODE); i++)
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
