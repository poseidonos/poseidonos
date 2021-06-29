
#include "src/meta/GroupMeta.h"

class GroupMetaTest : public ::testing::Test
{
public:
    meta::GroupMeta* group_meta{nullptr};
    meta::GroupMetaGetter* group_meta_getter{nullptr};

protected:
    GroupMetaTest()
    {
        group_meta = new meta::GroupMeta{};
        group_meta_getter = new meta::GroupMetaGetter{group_meta};
    }
    ~GroupMetaTest()
    {
        if (nullptr != group_meta)
        {
            delete group_meta;
            group_meta = nullptr;
        }
        if (nullptr != group_meta_getter)
        {
            delete group_meta_getter;
            group_meta_getter = nullptr;
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