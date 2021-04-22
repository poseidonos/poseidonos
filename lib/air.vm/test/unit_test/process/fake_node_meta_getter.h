#pragma once

#include "src/lib/Type.h"
#include "src/meta/NodeMeta.h"

#define MAX_NODE 7

class FakeNodeMetaGetter : public meta::NodeMetaGetter
{
public:
    FakeNodeMetaGetter()
    {
        for (int i = 0; i < MAX_NODE; i++)
            node_status[i] = true;
    }
    virtual ~FakeNodeMetaGetter()
    {
    }

    air::ProcessorType
    NodeProcessorType(uint32_t i) const
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
    bool
    NodeEnable(uint32_t i) const
    {
        if (0 <= i && MAX_NODE > i)
            return node_status[i];
        else
            return false;
    }

    void
    SetNodeDisable(uint32_t i)
    {
        node_status[i] = false;
    }

private:
    bool node_status[MAX_NODE];
};
