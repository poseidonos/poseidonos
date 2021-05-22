
#include "mock_global_meta.h"
#include "mock_node_meta.h"
#include "src/data_structure/NodeManager.cpp"
#include "src/data_structure/NodeManager.h"

class NodeManagerTest : public ::testing::Test
{
public:
    node::NodeManager* node_manager{nullptr};
    MockGlobalMetaGetter* mock_gloal_meta_getter{nullptr};
    MockNodeMetaGetter* mock_node_meta_getter{nullptr};

protected:
    NodeManagerTest()
    {
        mock_gloal_meta_getter = new MockGlobalMetaGetter{};
        mock_node_meta_getter = new MockNodeMetaGetter{};
        node_manager = new node::NodeManager{mock_gloal_meta_getter, mock_node_meta_getter};
        node_manager->Init();
    }
    ~NodeManagerTest() override
    {
        if (nullptr != node_manager)
        {
            delete node_manager;
            node_manager = nullptr;
        }
        if (nullptr != mock_gloal_meta_getter)
        {
            delete mock_gloal_meta_getter;
            mock_gloal_meta_getter = nullptr;
        }
        if (nullptr != mock_node_meta_getter)
        {
            delete mock_node_meta_getter;
            mock_node_meta_getter = nullptr;
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
