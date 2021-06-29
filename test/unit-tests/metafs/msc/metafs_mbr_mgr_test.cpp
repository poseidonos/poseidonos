#include "src/metafs/msc/metafs_mbr_mgr.h"
#include "test/unit-tests/metafs/common/meta_region_mock.h"
#include "test/unit-tests/metafs/msc/mbr/metafs_mbr_mock.h"

#include <gtest/gtest.h>

using namespace ::testing;

namespace pos
{
TEST(MetaFsMBRManager, MetaFsMBRManager_)
{
}

TEST(MetaFsMBRManager, Init_)
{
}

TEST(MetaFsMBRManager, Bringup_)
{
}

TEST(MetaFsMBRManager, SaveContent_)
{
}

TEST(MetaFsMBRManager, GetRegionSizeInLpn_)
{
}

TEST(MetaFsMBRManager, Finalize_)
{
}

TEST(MetaFsMBRManager, IsValidMBRExist_)
{
}

TEST(MetaFsMBRManager, GetEpochSignature_)
{
}

TEST(MetaFsMBRManager, LoadMBR_)
{
}

TEST(MetaFsMBRManager, BuildMBR_)
{
}

TEST(MetaFsMBRManager, CreateMBR_)
{
}

TEST(MetaFsMBRManager, RegisterVolumeGeometry_Normal)
{
    // Given
    const std::string arrayName = "TestArray";

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayName, mockMetaFsMBR);

    // When
    MetaStorageInfo mediaInfo {.media = MetaStorageType::SSD};
    metaFsMBRManagerSUT.RegisterVolumeGeometry(mediaInfo);

    // Then
}

TEST(MetaFsMBRManager, GetAllStoragePartitionInfo_Normal)
{
    // Given
    const std::string arrayName = "TestArray";

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayName, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.GetAllStoragePartitionInfo();

    // Then
}

TEST(MetaFsMBRManager, SetPowerStatus_Normal)
{
    // Given
    const std::string arrayName = "TestArray";

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, SetPORStatus);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayName, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.SetPowerStatus(true);

    // Then
}

TEST(MetaFsMBRManager, GetPowerStatus_Normal)
{
    // Given
    bool retb = false;
    const std::string arrayName = "TestArray";

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, GetPORStatus).WillRepeatedly(Return(true));

    MetaFsMBRManager metaFsMBRManagerSUT(arrayName, mockMetaFsMBR);

    // When
    retb = metaFsMBRManagerSUT.GetPowerStatus();

    // Then
    EXPECT_EQ(retb, true);
}

TEST(MetaFsMBRManager, InvalidMBR_mbrIsValid)
{
    // Given
    const std::string arrayName = "TestArray";

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, InvalidMBRSignature);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayName, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.InvalidMBR();

    // Then
}

} // namespace pos
