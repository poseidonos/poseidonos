
#include "src/data_structure/NodeData.cpp"
#include "src/data_structure/NodeData.h"
#include "src/lib/Hash.cpp"

class NodeDataTest : public ::testing::Test
{
public:
    node::NodeData* node_data{nullptr};

protected:
    NodeDataTest()
    {
    }
    ~NodeDataTest() override
    {
        if (nullptr != node_data)
        {
            delete node_data;
            node_data = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
