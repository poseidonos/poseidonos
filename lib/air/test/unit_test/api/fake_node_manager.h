
#include "src/api/AirTemplate.h"
#include "src/profile_data/node/NodeManager.cpp"
#include "src/profile_data/node/NodeThread.h"
#include "src/profile_data/node/NodeThread.cpp"
#include "src/lib/Hash.cpp"

class FakeNodeManager : public node::NodeManager
{
public:
    FakeNodeManager(meta::GlobalMetaGetter* new_global_meta_getter, 
        meta::NodeMetaGetter* new_node_meta_getter):

        node::NodeManager(new_global_meta_getter,
            new_node_meta_getter) {}

    node::ThreadArray* GetThread(uint32_t tid) override {
        return (node::ThreadArray*)0x3353;
    }
};
