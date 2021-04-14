#ifndef FAKE_NODE_MANAGER_H
#define FAKE_NODE_MANAGER_H

#include <map>

#include "src/profile_data/node/NodeManager.h"
#include "src/profile_data/node/NodeManager.cpp"

class FakeNodeManager : public node::NodeManager
{
public:
    node::NodeManager* node_manager {nullptr};
    FakeGlobalMetaGetter* fake_global_meta_getter {nullptr};
    FakeNodeMetaGetter* fake_node_meta_getter {nullptr};

    void ThreadMapLock() {
        return;
    }
    void ThreadMapUnlock() {
        return;
    }

    lib::AccLatencySeqData* GetAccLatSeqData(uint32_t nid, uint32_t aid, uint32_t sid) {
        return &(acc_data[aid].seq_data[sid]);
    }

    lib::AccLatencyData* GetAccLatData(uint32_t nid, uint32_t aid) {
        return &(acc_data[aid]);
    }

    FakeNodeManager(FakeGlobalMetaGetter* new_global_meta_getter,
            FakeNodeMetaGetter* new_node_meta_getter) {
        
        fake_global_meta_getter = new_global_meta_getter;
        fake_node_meta_getter = new_node_meta_getter;
        
        node_manager = new node::NodeManager(fake_global_meta_getter, fake_node_meta_getter);

        
        node::ThreadArray* thread_array = new node::ThreadArray;
        
        thread_array->node[0] = new node::Thread(air::ProcessorType::PERFORMANCE, 32);
        thread_array->node[1] = new node::Thread(air::ProcessorType::LATENCY, 32);
        thread_array->node[2] = new node::Thread(air::ProcessorType::LATENCY, 32);
        thread_array->node[3] = new node::Thread(air::ProcessorType::LATENCY, 32);
        thread_array->node[4] = new node::Thread(air::ProcessorType::LATENCY, 32);
        thread_array->node[5] = new node::Thread(air::ProcessorType::QUEUE, 32);
        thread_array->node[6] = new node::Thread(air::ProcessorType::QUEUE, 32);

        thread_array->node[0]->SetIsLogging(true);
        thread_array->node[1]->SetIsLogging(true);
        thread_array->node[2]->SetIsLogging(true);
        thread_array->node[3]->SetIsLogging(true);
        thread_array->node[4]->SetIsLogging(true);
        thread_array->node[5]->SetIsLogging(true);
        thread_array->node[6]->SetIsLogging(true);

        thread_map.insert(std::make_pair(123, *thread_array));
        
        num_aid = 32;
    }
    ~FakeNodeManager() {
        thread_map.clear();
        if (nullptr != node_manager)
            delete node_manager;
    }

    int GetNumAid() {
        return num_aid;
    }

private:
    int num_aid;
    lib::AccLatencyData acc_data[32];
};

#endif // #define FAKE_NODE_MANAGER_H
