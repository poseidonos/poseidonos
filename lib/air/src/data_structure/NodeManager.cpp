
#include "src/data_structure/NodeManager.h"

node::NodeDataArray*
node::NodeManager::GetNodeDataArray(uint32_t tid)
{
    auto iter = nda_map.find(tid);
    if (iter != nda_map.end())
    {
        return iter->second;
    }

    return nullptr;
}

void
node::NodeManager::SetNodeDataArrayName(uint32_t tid)
{
    auto iter = nda_map.find(tid);
    if (iter != nda_map.end())
    {
        node::NodeDataArray* node_data_array = iter->second;
        char str[NAMELEN];
        int rc = pthread_getname_np(pthread_self(), str, NAMELEN);
        if (0 == rc)
        {
            node_data_array->tname.clear();
            node_data_array->tname = str;
        }
    }
}

void
node::NodeManager::CreateNodeDataArray(uint32_t tid)
{
    NodeDataArray* node_data_array = GetNodeDataArray(tid);
    if (nullptr != node_data_array)
    {
        return;
    }

    NodeDataArray* new_node_data_array = new NodeDataArray;

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        air::ProcessorType type = node_meta_getter->ProcessorType(i);
        switch (type)
        {
            case (air::ProcessorType::PERFORMANCE):
            case (air::ProcessorType::LATENCY):
            case (air::ProcessorType::QUEUE):
            case (air::ProcessorType::UTILIZATION):
            case (air::ProcessorType::COUNT):
                new_node_data_array->node[i] =
                    new NodeData(type, node_meta_getter->IndexSize(i), node_meta_getter->FilterSize(i));
                break;
            default:
                new_node_data_array->node[i] = nullptr;
                break;
        }
    }
    nda_map.insert(std::make_pair(tid, new_node_data_array));
}

void
node::NodeManager::DeleteNodeDataArray(NodeDataArray* node_data_array)
{
    if (nullptr == node_data_array)
    {
        return;
    }

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        if (nullptr != node_data_array->node[i])
        {
            delete node_data_array->node[i];
            node_data_array->node[i] = nullptr;
        }
    }

    if (nullptr != node_data_array)
    {
        delete node_data_array;
        node_data_array = nullptr;
    }
}

bool
node::NodeManager::CanDeleteNodeDataArray(NodeDataArray* node_data_array)
{
    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        node::NodeData* node_data{nullptr};
        lib::Data* air_data{nullptr};
        lib::Data* user_data{nullptr};

        node_data = node_data_array->node[i];
        if (nullptr != node_data)
        {
            const uint32_t index_size = node_meta_getter->IndexSize(i);
            const uint32_t filter_size = node_meta_getter->FilterSize(i);
            for (uint32_t hash_idx = 0; hash_idx < index_size; hash_idx++)
            {
                for (uint32_t filter_idx = 0; filter_idx < filter_size; filter_idx++)
                {
                    if (air::ProcessorType::LATENCY ==
                        node_meta_getter->ProcessorType(i))
                    {
                        user_data = node_data->GetUserDataByHashIndex(hash_idx, filter_idx);
                        if (1 == user_data->access)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        air_data = node_data->GetAirData(hash_idx, filter_idx);
                        user_data = node_data->GetUserDataByHashIndex(hash_idx, filter_idx);
                        if ((1 == air_data->access) || (1 == user_data->access))
                        {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void
node::NodeManager::Init(void)
{
    acc_lat_data = new lib::AccLatencyData**[MAX_NID_SIZE];
    for (uint32_t nid = 0; nid < MAX_NID_SIZE; nid++)
    {
        if (air::ProcessorType::LATENCY == node_meta_getter->ProcessorType(nid))
        {
            const uint32_t index_size = node_meta_getter->IndexSize(nid);
            const uint32_t filter_size = node_meta_getter->FilterSize(nid);
            acc_lat_data[nid] = new lib::AccLatencyData*[index_size];

            for (uint32_t hash_index = 0; hash_index < index_size; hash_index++)
            {
                acc_lat_data[nid][hash_index] = new lib::AccLatencyData[filter_size];
            }
        }
    }
}
