
#include "src/meta/NodeMeta.h"

class MockNodeMetaGetter : public meta::NodeMetaGetter
{
public:
    virtual ~MockNodeMetaGetter() {}

    air::ProcessorType NodeProcessorType(uint32_t i) const {
        switch (i) {
        case 0 :
            return air::ProcessorType::PERFORMANCE;
        case 1 :
            return air::ProcessorType::LATENCY;
        case 2 :
            return air::ProcessorType::QUEUE;
        default :
            return air::ProcessorType::PROCESSORTYPE_NULL;
        }
    }

private:
};
