
#include "src/data_structure/NodeData.cpp"
#include "src/data_structure/NodeData.h"
#include "src/data_structure/NodeManager.cpp"
#include "src/data_structure/NodeManager.h"
#include "src/lib/Hash.cpp"

class MockNodeManager : public node::NodeManager
{
public:
    virtual ~MockNodeManager()
    {
    }
    node::NodeDataArray*
    GetNodeDataArray(uint32_t tid)
    {
        auto tid_iter = nda_map.find(tid);
        if (tid_iter != nda_map.end())
        {
            return tid_iter->second;
        }
        return nullptr;
    }

    void
    CreateNodeDataArray(uint32_t tid)
    {
        node::NodeDataArray* node_data_array = new node::NodeDataArray;
        nda_map.insert(std::make_pair(tid, node_data_array));
    }

    void
    DeleteNodeDataArray(node::NodeDataArray* node_data_array)
    {
        return;
    }

    bool
    CanDeleteNodeDataArray(node::NodeDataArray* node_data_array)
    {
        auto it = nda_map.begin();
        while (it != nda_map.end())
        {
            if (it->second == node_data_array &&
                it->first == 11111) // tid 11111 can not delete
            {
                return false;
            }
            it++;
        }
        return true;
    }
};
