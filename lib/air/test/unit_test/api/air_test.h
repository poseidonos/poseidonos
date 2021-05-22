
#include "fake_collection_manager.h"
#include "fake_instance_manager.h"
#include "fake_node_manager.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/config/ConfigInterface.h"

class MockAIR : public AIR<true, true>
{
public:
    MockAIR()
    : AIR()
    {
    }
    void
    MockInit()
    {
        fake_collection_manager = new FakeCollectionManager{nullptr, nullptr, nullptr, nullptr};
        AIR::collection_manager = fake_collection_manager;

        fake_instance_manager = new FakeInstanceManager{};
        AIR::instance_manager = fake_instance_manager;

        fake_node_manager = new FakeNodeManager{nullptr, nullptr};
        AIR::node_manager = fake_node_manager;
    }
    void
    SetNullNodeManager()
    {
        if (nullptr != fake_node_manager)
        {
            delete fake_node_manager;
        }
        fake_node_manager = nullptr;
        AIR::node_manager = nullptr;
    }
    void
    SetDefaultNodeManager()
    {
        if (nullptr == fake_node_manager)
        {
            fake_node_manager = new FakeNodeManager{nullptr, nullptr};
            AIR::node_manager = fake_node_manager;
        }
    }
    void
    SetNullCollectionManager()
    {
        AIR::collection_manager = nullptr;
    }
    static FakeCollectionManager* fake_collection_manager;
    static FakeInstanceManager* fake_instance_manager;
    static FakeNodeManager* fake_node_manager;
    static thread_local node::NodeDataArray* node_data_array;
};

class TestAPI : public ::testing::Test
{
public:
    MockAIR* mock_air{nullptr};

protected:
    TestAPI()
    {
        mock_air = new MockAIR{};
    }
    ~TestAPI() override
    {
        if (nullptr != mock_air)
        {
            delete mock_air;
            mock_air = nullptr;
        }
    }
    void
    SetUp() override
    {
        mock_air->MockInit();
    }
    void
    TearDown() override
    {
    }
};
