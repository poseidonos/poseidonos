
#include "src/data_structure/NodeData.cpp"
#include "src/data_structure/NodeManager.cpp"
#include "src/data_structure/NodeManager.h"
#include "src/lib/Hash.cpp"

class MockNodeManager : public node::NodeManager
{
public:
    virtual ~MockNodeManager()
    {
        nda_map.clear();
    }
    MockNodeManager(meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter)
    : node::NodeManager(new_global_meta_getter, new_node_meta_getter)
    {
    }
    node::NodeDataArray*
    GetNodeDataArray(uint32_t tid)
    {
        auto nda_iter = nda_map.find(tid);
        if (nda_iter != nda_map.end())
        {
            return nda_iter->second;
        }

        return nullptr;
    }

    lib::AccLatencyData*
    GetAccLatData(uint32_t nid, uint32_t aid)
    {
        return &(mock_acc_lat_data[aid]);
    }

    void
    CreateNodeDataArray(uint32_t tid)
    {
        node::NodeDataArray* node_data_array = GetNodeDataArray(tid);
        if (nullptr != node_data_array)
        {
            return; // already create
        }

        node::NodeDataArray* nda = new node::NodeDataArray;
        nda->node[0] = new node::NodeData(air::ProcessorType::PERFORMANCE, 3, 3);
        nda->node[1] = new node::NodeData(air::ProcessorType::LATENCY, 3, 3);
        nda->node[2] = new node::NodeData(air::ProcessorType::QUEUE, 3, 3);
        nda->node[3] = nullptr;
        nda->node[4] = nullptr;
        nda->node[5] = new node::NodeData(air::ProcessorType::QUEUE, 3, 3);

        nda_map.insert(std::make_pair(tid, nda));
    }

private:
    lib::AccLatencyData mock_acc_lat_data[32];
};
