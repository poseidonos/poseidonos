
#include "src/meta/NodeMeta.h"

class NodeMetaTest : public ::testing::Test
{
public:
    meta::NodeMeta* node_meta{nullptr};
    meta::NodeMetaGetter* node_meta_getter{nullptr};

protected:
    NodeMetaTest()
    {
        node_meta = new meta::NodeMeta{};
        node_meta_getter = new meta::NodeMetaGetter{node_meta};
    }
    ~NodeMetaTest() override
    {
        if (nullptr != node_meta)
        {
            delete node_meta;
            node_meta = nullptr;
        }
        if (nullptr != node_meta_getter)
        {
            delete node_meta_getter;
            node_meta_getter = nullptr;
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
