
#include "src/profile_data/node/NodeManager.h"

node::ThreadArray*
node::NodeManager::GetThread(uint32_t tid)
{
    std::map<uint32_t, ThreadArray>::iterator tid_iter;

    tid_iter = thread_map.find(tid);
    if (tid_iter != thread_map.end())
    {
        return &(tid_iter->second);
    }

    return nullptr;
}

void
node::NodeManager::SetThreadName(uint32_t tid)
{
    std::map<uint32_t, ThreadArray>::iterator tid_iter;

    tid_iter = thread_map.find(tid);
    if (tid_iter != thread_map.end())
    {
        node::ThreadArray* thread_array = &(tid_iter->second);
        char str[NAMELEN];
        int rc = pthread_getname_np(pthread_self(), str, NAMELEN);
        if (0 == rc)
        {
            thread_array->tname.clear();
            thread_array->tname = str;
        }
    }
}

int
node::NodeManager::CreateThread(uint32_t tid)
{
    ThreadArray* thread = GetThread(tid);
    if (nullptr != thread)
    {
        return 0; // already create
    }

    ThreadArray thread_array;
    uint32_t max_aid_size = global_meta_getter->AidSize();

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        air::ProcessorType type = node_meta_getter->NodeProcessorType(i);
        switch (type)
        {
            case (air::ProcessorType::PERFORMANCE):
            case (air::ProcessorType::LATENCY):
            case (air::ProcessorType::QUEUE):
                thread_array.node[i] = new Thread(type, max_aid_size);
                break;
            default:
                thread_array.node[i] = nullptr;
                break;
        }
    }
    thread_map.insert(std::make_pair(tid, thread_array));

    return 1; // create complete
}

void
node::NodeManager::DeleteThread(ThreadArray* thread_array)
{
    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        if (nullptr != thread_array->node[i])
        {
            delete thread_array->node[i];
            thread_array->node[i] = nullptr;
        }
    }
}

bool
node::NodeManager::CanDelete(ThreadArray* thread_array)
{
    uint32_t max_aid_size = global_meta_getter->AidSize();

    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        node::Thread* thread{nullptr};
        lib::Data* air_data{nullptr};
        lib::Data* user_data{nullptr};

        thread = thread_array->node[i];
        if (nullptr != thread)
        {
            for (uint32_t aid_idx = 0; aid_idx < max_aid_size; aid_idx++)
            {
                if (air::ProcessorType::LATENCY ==
                    node_meta_getter->NodeProcessorType(i))
                {
                    user_data = thread->GetUserDataByAidIndex(aid_idx);
                    if (1 == user_data->access)
                    {
                        return false;
                    }
                }
                else
                {
                    air_data = thread->GetAirData(aid_idx);
                    user_data = thread->GetUserDataByAidIndex(aid_idx);
                    if ((1 == air_data->access) || (1 == user_data->access))
                    {
                        return false;
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
    uint32_t max_aid_size = global_meta_getter->AidSize();

    acc_lat_data = new lib::AccLatencyData*[MAX_NID_SIZE];
    for (uint32_t i = 0; i < MAX_NID_SIZE; i++)
    {
        acc_lat_data[i] = new lib::AccLatencyData[max_aid_size];
    }
}
