#ifndef FAKE_NODE_MANAGER_H
#define FAKE_NODE_MANAGER_H

#include <map>

#include "src/data_structure/NodeManager.cpp"
#include "src/data_structure/NodeManager.h"
#include "test/unit_test/process/fake_global_meta_getter.h"
#include "test/unit_test/process/fake_node_meta_getter.h"

class FakeNodeManager : public node::NodeManager
{
public:
    node::NodeManager* node_manager{nullptr};
    FakeGlobalMetaGetter* fake_global_meta_getter{nullptr};
    FakeNodeMetaGetter* fake_node_meta_getter{nullptr};

    void
    ThreadMapLock()
    {
        return;
    }
    void
    ThreadMapUnlock()
    {
        return;
    }

    lib::AccLatencyData*
    GetAccLatData(uint32_t nid, uint32_t hash_index, uint32_t filter_index)
    {
        return &(acc_data[filter_index]);
    }

    FakeNodeManager(FakeGlobalMetaGetter* new_global_meta_getter,
        FakeNodeMetaGetter* new_node_meta_getter)
    {
        fake_global_meta_getter = new_global_meta_getter;
        fake_node_meta_getter = new_node_meta_getter;

        node_manager = new node::NodeManager(fake_global_meta_getter, fake_node_meta_getter);

        node::NodeDataArray* node_data_array = new node::NodeDataArray;

        node_data_array->node[0] = new node::NodeData(air::ProcessorType::PERFORMANCE, 32, 32);
        node_data_array->node[1] = new node::NodeData(air::ProcessorType::LATENCY, 32, 32);
        node_data_array->node[2] = new node::NodeData(air::ProcessorType::LATENCY, 32, 32);
        node_data_array->node[3] = new node::NodeData(air::ProcessorType::LATENCY, 32, 32);
        node_data_array->node[4] = new node::NodeData(air::ProcessorType::LATENCY, 32, 32);
        node_data_array->node[5] = new node::NodeData(air::ProcessorType::QUEUE, 32, 32);
        node_data_array->node[6] = new node::NodeData(air::ProcessorType::QUEUE, 32, 32);

        nda_map.insert(std::make_pair(123, node_data_array));
    }
    ~FakeNodeManager()
    {
        if (nullptr != node_manager)
            delete node_manager;
    }

private:
    lib::AccLatencyData acc_data[32];
};

#endif // #define FAKE_NODE_MANAGER_H
