#include "src/metafs/common/meta_region.h"
#include "src/metafs/common/meta_region_content.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;

namespace pos
{
enum class TestRegionType
{
    First = 0,
    Test1 = First,
    Test2,
    Last = Test2,

    Max,
};

class TestContent : public MetaRegionContent
{
public:
    uint64_t signature = 3;
};

class TestClass : public MetaRegion<TestRegionType, TestContent>
{
public:
    TestClass(void)
    : MetaRegion<TestRegionType, TestContent>()
    {
    }
    TestClass(MetaStorageType mediaType, TestRegionType regionType, MetaLpnType baseLpn, uint32_t mirrorCount = 0)
    : MetaRegion<TestRegionType, TestContent>(mediaType, regionType, baseLpn, mirrorCount)
    {
    }
};

TEST(MetaRegion, CheckContent0)
{
    TestClass obj;

    EXPECT_TRUE(obj.GetContent() == nullptr);
}

TEST(MetaRegion, CheckContent1)
{
    TestClass obj;
    TestContent content;

    obj.SetContent(&content);

    EXPECT_EQ(obj.GetContent(), &content);
}

TEST(MetaRegion, GetContent)
{
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    EXPECT_TRUE(obj.GetContent() != nullptr);
    EXPECT_TRUE(obj.GetContent()->signature == 3);
}

TEST(MetaRegion, GetContentSize)
{
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    EXPECT_EQ(obj.GetSizeOfContent(), sizeof(TestContent));
}

TEST(MetaRegion, CheckBaseLpn)
{
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 100, 0);

    EXPECT_EQ(obj.GetBaseLpn(), 100);
}

TEST(MetaRegion, CheckContentAddr0)
{
    TestClass obj;
    TestContent content;

    obj.SetContent(&content);

    EXPECT_EQ(obj.GetDataBuf(), &content);
}

TEST(MetaRegion, CheckContentAddr1)
{
    TestClass obj;
    TestContent content;

    obj.SetContent(&content);

    EXPECT_EQ(obj.GetDataBuf(1), (char*)&content + 4096);
}

TEST(MetaRegion, CheckLpnCount)
{
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    EXPECT_EQ(obj.GetLpnCntOfRegion(), 1);
}

TEST(MetaRegion, CheckResetContent)
{
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    EXPECT_TRUE(obj.GetContent()->signature == 3);

    obj.ResetContent();

    EXPECT_TRUE(obj.GetContent()->signature == 0);
}

TEST(MetaRegion, CheckLoad_Positive0)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, ReadPage).WillOnce(Return(EID(SUCCESS)));

    EXPECT_TRUE(obj.Load());
}

TEST(MetaRegion, CheckLoad_Negative0)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, ReadPage).WillOnce(Return(EID(MFS_META_STORAGE_NOT_READY)));

    EXPECT_FALSE(obj.Load());
}

TEST(MetaRegion, CheckLoad_Positive1)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, ReadPage).WillOnce(Return(EID(SUCCESS)));

    EXPECT_TRUE(obj.Load(MetaStorageType::SSD, 0, 0, 0));
}

TEST(MetaRegion, CheckLoad_Negative1)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, ReadPage).WillOnce(Return(EID(MFS_META_STORAGE_NOT_READY)));

    EXPECT_FALSE(obj.Load(MetaStorageType::SSD, 0, 0, 0));
}

TEST(MetaRegion, CheckStore_Positive0)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, WritePage).WillOnce(Return(EID(SUCCESS)));

    EXPECT_TRUE(obj.Store());
}

TEST(MetaRegion, CheckStore_Negative0)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, WritePage).WillOnce(Return(EID(MFS_META_STORAGE_NOT_READY)));

    EXPECT_FALSE(obj.Store());
}

TEST(MetaRegion, CheckStore_Positive1)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, WritePage).WillOnce(Return(EID(SUCCESS)));

    EXPECT_TRUE(obj.Store(MetaStorageType::SSD, 0, 0, 0));
}

TEST(MetaRegion, CheckStore_Negative1)
{
    MockMetaStorageSubsystem mss(0);
    TestClass obj(MetaStorageType::SSD, TestRegionType::Test1, 0, 0);

    obj.SetMss(&mss);

    EXPECT_CALL(mss, WritePage).WillOnce(Return(EID(MFS_META_STORAGE_NOT_READY)));

    EXPECT_FALSE(obj.Store(MetaStorageType::SSD, 0, 0, 0));
}
} // namespace pos
