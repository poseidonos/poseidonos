#include "src/metafs/include/meta_storage_info.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(MetaStorageInfo, GetType_testIfReturnsOwnStorageType)
{
    const MetaStorageType EXPECT_TYPE = MetaStorageType::JOURNAL_SSD;
    MetaStorageInfo info(EXPECT_TYPE);
    EXPECT_EQ(EXPECT_TYPE, info.GetType());
}

TEST(MetaStorageInfo, GetCapacity_testIfReturnsExpectedCapacity)
{
    MetaStorageInfo info(MetaStorageType::JOURNAL_SSD);
    const uint64_t EXPECT_CAPACITY = 123456789;
    info.SetCapacity(EXPECT_CAPACITY);
    EXPECT_EQ(EXPECT_CAPACITY, info.GetCapacity());
}

TEST(MetaStorageInfo, GetValid_testIfReturnsExpectedValidity)
{
    MetaStorageInfo info(MetaStorageType::JOURNAL_SSD);
    EXPECT_EQ(false, info.IsValid());
    const uint64_t EXPECT_VALID = true;
    info.SetValid(EXPECT_VALID);
    EXPECT_EQ(EXPECT_VALID, info.IsValid());
}
} // namespace pos
