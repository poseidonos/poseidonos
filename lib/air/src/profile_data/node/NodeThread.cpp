
#include "src/profile_data/node/NodeThread.h"

node::Thread::Thread(air::ProcessorType new_ptype, uint32_t new_aid_size)
{
    ptype = new_ptype;
    max_aid_size = new_aid_size;
    user_data = new lib::Data*[max_aid_size];

    switch (ptype)
    {
        case (air::ProcessorType::LATENCY):
            for (uint32_t i = 0; i < max_aid_size; i++)
            {
                user_data[i] = new lib::LatencyData;
            }
            break;
        case (air::ProcessorType::PERFORMANCE):
            air_data = new lib::Data*[max_aid_size];
            acc_data = new lib::AccData*[max_aid_size];
            for (uint32_t i = 0; i < max_aid_size; i++)
            {
                user_data[i] = new lib::PerformanceData;
                air_data[i] = new lib::PerformanceData;
                acc_data[i] = new lib::AccPerformanceData;
            }
            break;
        case (air::ProcessorType::QUEUE):
            air_data = new lib::Data*[max_aid_size];
            acc_data = new lib::AccData*[max_aid_size];
            for (uint32_t i = 0; i < max_aid_size; i++)
            {
                user_data[i] = new lib::QueueData;
                air_data[i] = new lib::QueueData;
                acc_data[i] = new lib::AccQueueData;
            }
            break;
        default:
            break;
    }

    hash_map = new hash::HashMap<uint64_t>{max_aid_size};
}

node::Thread::~Thread(void)
{
    if (air::ProcessorType::PROCESSORTYPE_NULL == ptype)
    {
        delete[] user_data;
        if (nullptr != hash_map)
        {
            delete hash_map;
            hash_map = nullptr;
        }
        return;
    }

    for (uint32_t i = 0; i < max_aid_size; i++)
    {
        if (nullptr != user_data[i])
        {
            delete user_data[i];
            user_data[i] = nullptr;
        }
        if (air::ProcessorType::LATENCY != ptype)
        {
            if (nullptr != air_data[i])
            {
                delete air_data[i];
                air_data[i] = nullptr;
            }
            if (nullptr != acc_data[i])
            {
                delete acc_data[i];
                acc_data[i] = nullptr;
            }
        }
    }

    delete[] user_data;
    if (air::ProcessorType::LATENCY != ptype)
    {
        delete[] air_data;
        delete[] acc_data;
    }
    if (nullptr != hash_map)
    {
        delete hash_map;
        hash_map = nullptr;
    }
}
