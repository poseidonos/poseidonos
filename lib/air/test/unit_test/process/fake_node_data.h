
#include "src/data_structure/NodeData.cpp"
#include "src/data_structure/NodeData.h"
#include "src/lib/Hash.cpp"

class FakeNodeData : public node::NodeData
{
public:
    FakeNodeData(air::ProcessorType type, uint32_t index_size, uint32_t filter_size)
    : NodeData(type, index_size, filter_size)
    {
    }
};
