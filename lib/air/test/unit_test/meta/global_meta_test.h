
#include "src/meta/GlobalMeta.h"

class GlobalMetaTest : public ::testing::Test
{
public:
    meta::GlobalMeta* global_meta{nullptr};
    meta::GlobalMetaGetter* global_meta_getter{nullptr};

protected:
    GlobalMetaTest()
    {
        global_meta = new meta::GlobalMeta{};
        global_meta_getter = new meta::GlobalMetaGetter{global_meta};
    }
    ~GlobalMetaTest() override
    {
        if (nullptr != global_meta)
        {
            delete global_meta;
            global_meta = nullptr;
        }
        if (nullptr != global_meta_getter)
        {
            delete global_meta_getter;
            global_meta_getter = nullptr;
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
