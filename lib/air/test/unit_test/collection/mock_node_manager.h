
#include "src/lib/Hash.cpp"
#include "src/profile_data/node/NodeManager.cpp"
#include "src/profile_data/node/NodeManager.h"
#include "src/profile_data/node/NodeThread.cpp"

class MockNodeManager : public node::NodeManager
{
public:
    virtual ~MockNodeManager()
    {
        thread_map.clear();
    }
    node::ThreadArray*
    GetThread(uint32_t tid)
    {
        std::map<uint32_t, node::ThreadArray>::iterator tid_iter;

        tid_iter = thread_map.find(tid);
        if (tid_iter != thread_map.end())
        {
            return &(tid_iter->second);
        }

        return nullptr;
    }

    lib::AccLatencyData*
    GetAccLatData(uint32_t nid, uint32_t aid)
    {
        return &(mock_acc_lat_data[aid]);
    }

    int
    CreateThread(uint32_t tid)
    {
        node::ThreadArray* thread = GetThread(tid);
        if (nullptr != thread)
        {
            return 0; // already create
        }

        node::ThreadArray thread_array;
        thread_array.node[0] = new node::Thread(air::ProcessorType::PERFORMANCE, 3);
        thread_array.node[1] = new node::Thread(air::ProcessorType::LATENCY, 3);
        thread_array.node[2] = new node::Thread(air::ProcessorType::QUEUE, 3);
        thread_array.node[3] = nullptr;
        thread_array.node[4] = nullptr;
        thread_array.node[5] = new node::Thread(air::ProcessorType::QUEUE, 3);

        thread_map.insert(std::make_pair(tid, thread_array));

        return 1;
    }

private:
    lib::AccLatencyData mock_acc_lat_data[32];
};
