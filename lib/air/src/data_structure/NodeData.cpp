
#include "src/data_structure/NodeData.h"

#include <iostream>

node::NodeData::NodeData(air::ProcessorType ptype, uint32_t index_size, uint32_t filter_size)
: ptype(ptype),
  index_size(index_size),
  filter_size(filter_size)
{
    user_data = new lib::Data**[index_size];
    air_data = new lib::Data**[index_size];
    acc_data = new lib::AccData**[index_size];

    for (uint32_t index = 0; index < index_size; index++)
    {
        if (air::ProcessorType::LATENCY == ptype)
        {
            user_data[index] = new lib::Data*[filter_size];
        }
        else
        {
            user_data[index] = new lib::Data*[filter_size];
            air_data[index] = new lib::Data*[filter_size];
            acc_data[index] = new lib::AccData*[filter_size];
        }
    }

    switch (ptype)
    {
        case (air::ProcessorType::LATENCY):
        {
            for (uint32_t index = 0; index < index_size; index++)
            {
                for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                {
                    user_data[index][filter_index] = new lib::LatencyData;
                }
            }
            break;
        }
        case (air::ProcessorType::PERFORMANCE):
        {
            for (uint32_t index = 0; index < index_size; index++)
            {
                for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                {
                    user_data[index][filter_index] = new lib::PerformanceData;
                    air_data[index][filter_index] = new lib::PerformanceData;
                    acc_data[index][filter_index] = new lib::AccPerformanceData;
                }
            }
            break;
        }
        case (air::ProcessorType::QUEUE):
        {
            for (uint32_t index = 0; index < index_size; index++)
            {
                for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                {
                    user_data[index][filter_index] = new lib::QueueData;
                    air_data[index][filter_index] = new lib::QueueData;
                    acc_data[index][filter_index] = new lib::AccQueueData;
                }
            }
            break;
        }
        case (air::ProcessorType::UTILIZATION):
        {
            for (uint32_t index = 0; index < index_size; index++)
            {
                for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                {
                    user_data[index][filter_index] = new lib::UtilizationData;
                    air_data[index][filter_index] = new lib::UtilizationData;
                    acc_data[index][filter_index] = new lib::AccUtilizationData;
                }
            }
            break;
        }
        case (air::ProcessorType::COUNT):
        {
            for (uint32_t index = 0; index < index_size; index++)
            {
                for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
                {
                    user_data[index][filter_index] = new lib::CountData;
                    air_data[index][filter_index] = new lib::CountData;
                    acc_data[index][filter_index] = new lib::AccCountData;
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }

    hash_map = new air::HashMap<uint64_t>{index_size};
}

node::NodeData::~NodeData(void)
{
    for (uint32_t index = 0; index < index_size; index++)
    {
        for (uint32_t filter_index = 0; filter_index < filter_size; filter_index++)
        {
            delete user_data[index][filter_index];
            if (air::ProcessorType::LATENCY != ptype)
            {
                delete air_data[index][filter_index];
                delete acc_data[index][filter_index];
            }
        }
    }

    for (uint32_t index = 0; index < index_size; index++)
    {
        delete[] user_data[index];
        if (air::ProcessorType::LATENCY != ptype)
        {
            delete[] air_data[index];
            delete[] acc_data[index];
        }
    }

    delete[] user_data;
    delete[] air_data;
    delete[] acc_data;

    if (nullptr != hash_map)
    {
        delete hash_map;
        hash_map = nullptr;
    }
}
