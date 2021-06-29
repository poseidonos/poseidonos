
#include "src/api/Air.h"
#include "src/data_structure/NodeData.cpp"
#include "src/data_structure/NodeData.h"
#include "src/data_structure/NodeManager.cpp"
#include "src/lib/Hash.cpp"

class FakeNodeManager : public node::NodeManager
{
public:
    FakeNodeManager(meta::GlobalMetaGetter* new_global_meta_getter,
        meta::NodeMetaGetter* new_node_meta_getter)
    : node::NodeManager(new_global_meta_getter, new_node_meta_getter)
    {
    }

    node::NodeDataArray*
    GetNodeDataArray(uint32_t tid) override
    {
        return (node::NodeDataArray*)0x3353;
    }
};
