
#include "src/meta/NodeMeta.h"

class MockNodeMetaGetter : public meta::NodeMetaGetter
{
public:
    virtual ~MockNodeMetaGetter()
    {
    }

    virtual air::ProcessorType
    ProcessorType(uint32_t i) const
    {
        switch (i)
        {
            case 0:
                return air::ProcessorType::PERFORMANCE;
            case 1:
                return air::ProcessorType::LATENCY;
            case 2:
                return air::ProcessorType::QUEUE;
            default:
                return air::ProcessorType::PROCESSORTYPE_NULL;
        }
    }

    virtual inline uint32_t
    IndexSize(uint32_t nid) const
    {
        return 10;
    }

    virtual inline uint32_t
    FilterSize(uint32_t nid) const
    {
        return 10;
    }

private:
};
