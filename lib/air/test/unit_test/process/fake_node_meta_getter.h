#pragma once

#include "src/lib/Type.h"
#include "src/meta/NodeMeta.h"

#define MAX_NODE 7

class FakeNodeMetaGetter : public meta::NodeMetaGetter
{
public:
    FakeNodeMetaGetter()
    {
    }
    virtual ~FakeNodeMetaGetter()
    {
    }

    air::ProcessorType
    ProcessorType(uint32_t i) const
    {
        switch (i)
        {
            case 0:
                return air::ProcessorType::PERFORMANCE;
            case 1:
            case 2:
            case 3:
                return air::ProcessorType::LATENCY;
            case 4:
                return air::ProcessorType::PROCESSORTYPE_NULL;
            case 5:
            case 6:
                return air::ProcessorType::QUEUE;
            default:
                return air::ProcessorType::PROCESSORTYPE_NULL;
        }
    }

    virtual inline uint32_t
    IndexSize(uint32_t nid) const
    {
        return 32;
    }

    virtual inline uint32_t
    FilterSize(uint32_t nid) const
    {
        return 32;
    }

    virtual inline bool
    Run(uint32_t nid) const
    {
        return true;
    }
};
