
#include "src/meta/NodeMeta.h"

class MockNodeMetaGetter : public meta::NodeMetaGetter
{
public:
    virtual ~MockNodeMetaGetter() {}
    virtual inline bool NodeEnable(uint32_t node_index) const {
        return true;
    }
    virtual inline air::ProcessorType NodeProcessorType(uint32_t node_index) const {

        air::ProcessorType ptype = air::ProcessorType::PROCESSORTYPE_NULL;

        switch (node_index) {
        case 0 :
            ptype = air::ProcessorType::PERFORMANCE;
            break;
        case 1 :
            ptype =  air::ProcessorType::LATENCY;
            break;
        case 2 ... 5 :
            ptype =  air::ProcessorType::QUEUE;
            break;
        case 6 :
            ptype = air::ProcessorType::UTILIZATION;
            break;
        case 7 ... 8 :
            ptype = air::ProcessorType::COUNT;
            break;
        default :
            break;
        }

        return ptype;
    }

    virtual inline int32_t NodeGroupId(uint32_t node_index) const {
        return group_id[node_index];
    }
private:
    int32_t group_id[7] {-1, 0, -1, 1, -1, 0, 1};
};

