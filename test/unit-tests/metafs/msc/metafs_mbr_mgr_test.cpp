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
    const int arrayId = 0;

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    MetaStorageInfo mediaInfo {.media = MetaStorageType::SSD};
    metaFsMBRManagerSUT.RegisterVolumeGeometry(mediaInfo);

    // Then
}

TEST(MetaFsMBRManager, GetAllStoragePartitionInfo_Normal)
{
    // Given
    const int arrayId = 0;

    MetaFsMBRContent* metaFsMBRContent = new (1) MetaFsMBRContent;  // meta_region.h:88
    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    mockMetaFsMBR->SetContent(metaFsMBRContent);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.GetAllStoragePartitionInfo();

    // Then
}

TEST(MetaFsMBRManager, SetPowerStatus_Normal)
{
    // Given
    const int arrayId = 0;

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, SetPORStatus);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.SetPowerStatus(true);

    // Then
}

TEST(MetaFsMBRManager, GetPowerStatus_Normal)
{
    // Given
    bool retb = false;
    const int arrayId = 0;

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, GetPORStatus).WillRepeatedly(Return(true));

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    retb = metaFsMBRManagerSUT.GetPowerStatus();

    // Then
    EXPECT_EQ(retb, true);
}

TEST(MetaFsMBRManager, InvalidMBR_mbrIsValid)
{
    // Given
    const int arrayId = 0;

    MockMetaFsMBR* mockMetaFsMBR = new MockMetaFsMBR;
    EXPECT_CALL(*mockMetaFsMBR, InvalidMBRSignature);

    MetaFsMBRManager metaFsMBRManagerSUT(arrayId, mockMetaFsMBR);

    // When
    metaFsMBRManagerSUT.InvalidMBR();

    // Then
}

} // namespace pos
