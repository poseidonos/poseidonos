#include "src/metafs/util/metafs_mem_lib.h"

#include <gtest/gtest.h>

namespace pos
{
class MetaFsMemLibTester
{
public:
    MetaFsMemLibTester(void)
    {
    }

    static void CallbackTest(void* obj)
    {
    }
};

TEST(MetaFsMemLib, CheckAvailable)
{
    EXPECT_FALSE(MetaFsMemLib::IsResourceAvailable());
}

TEST(MetaFsMemLib, SetResourceAvailable)
{
    MetaFsMemLib::EnableResourceUse();
    EXPECT_TRUE(MetaFsMemLib::IsResourceAvailable());
}

TEST(MetaFsMemLib, MemoryCopyAsync)
{
    MetaFsMemLibTester tester;
    MetaFsMemLib::EnableResourceUse();
    
    uint64_t src = 0x12121212F0F0F0F0;
    uint64_t dst = 0;

    MetaFsMemLib::MemCpyAsync((void*)&dst, (void*)&src, sizeof(src), &(tester.CallbackTest), &tester);

#if (1 == METAFS_INTEL_IOAT_EN)
    EXPECT_EQ(src, dst);
#else
    EXPECT_NE(src, dst);
#endif
}

TEST(MetaFsMemLib, MemorySetZero)
{
    MetaFsMemLibTester tester;
    MetaFsMemLib::EnableResourceUse();
    
    uint64_t src = 0x12121212F0F0F0F0;

    MetaFsMemLib::MemSetZero((void*)&src, sizeof(src), &(tester.CallbackTest), &tester);

#if (1 == METAFS_INTEL_IOAT_EN)
    EXPECT_EQ(src, 0);
#else
    EXPECT_NE(src, 0);
#endif
}
} // namespace pos
