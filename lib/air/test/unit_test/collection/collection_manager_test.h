
#include "collector_test.h"
#include "mock_global_meta.h"
#include "mock_node_manager.h"
#include "mock_node_meta.h"
#include "mock_output_observer.h"
#include "src/collection/CollectionManager.cpp"
#include "src/collection/CollectionManager.h"
#include "src/lib/Design.cpp"
#include "src/lib/Design.h"
#include "src/lib/Msg.h"

class CollectionManagerTest : public ::testing::Test
{
public:
    MockGlobalMetaGetter* mock_global_meta_getter{nullptr};
    MockNodeMetaGetter* mock_node_meta_getter{nullptr};
    MockNodeManager* mock_node_manager = {nullptr};

    collection::Subject* collection_subject{nullptr};
    collection::CollectionManager* collection_manager{nullptr};

protected:
    CollectionManagerTest()
    {
        mock_global_meta_getter = new MockGlobalMetaGetter{};
        mock_node_meta_getter = new MockNodeMetaGetter{};
        mock_node_manager = new MockNodeManager{mock_global_meta_getter, mock_node_meta_getter};
        collection_subject = new collection::Subject{};
        collection_manager = new collection::CollectionManager{
            mock_global_meta_getter, mock_node_meta_getter,
            mock_node_manager, collection_subject};
    }
    ~CollectionManagerTest() override
    {
        if (nullptr != mock_global_meta_getter)
        {
            delete mock_global_meta_getter;
            mock_global_meta_getter = nullptr;
        }
        if (nullptr != mock_node_meta_getter)
        {
            delete mock_node_meta_getter;
            mock_node_meta_getter = nullptr;
        }
        if (nullptr != collection_subject)
        {
            delete collection_subject;
            collection_subject = nullptr;
        }
        if (nullptr != collection_manager)
        {
            delete collection_manager;
            collection_manager = nullptr;
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
