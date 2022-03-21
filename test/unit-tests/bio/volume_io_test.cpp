#include "src/bio/volume_io.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/array_components/components_info.h"
#include "test/unit-tests/array_mgmt/interface/i_array_mgmt_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::Test;
namespace pos
{
TEST(VolumeIo, VolumeIo_WithArguments)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 8;
    int arrayId = 0;
    // When : Create new Volume Io with constructor
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    // Then : Nothing
}

TEST(VolumeIo, VolumeIo_CopyConstructor)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 8;
    int arrayId = 0;
    // When : Create new Volume Io with constructor
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    // Then : Call Copy Constructor
    VolumeIoSmartPtr newVolumeIo2(new VolumeIo(*newVolumeIo, nullptr));
}

TEST(VolumeIo, Split)
{
     // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io with constructor
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    // Then : split with removalFromTail as true
    // (rba of split volume Io should be modified)
    newVolumeIo->SetSectorRba(0x0);
    VolumeIoSmartPtr split = newVolumeIo->Split(0x8, true);
    // 20 - 8 = 12
    EXPECT_EQ(split->GetSectorRba(), 12);
    EXPECT_EQ(newVolumeIo->GetSectorRba(), 0x0);

    // Then : split with removalFromTail as false
    // (rba : existing volume Io should be modified)
    newVolumeIo->SetSectorRba(0x0);
    VolumeIoSmartPtr split2 = newVolumeIo->Split(0x8, false);
    EXPECT_EQ(split2->GetSectorRba(), 0x0);
    EXPECT_EQ(newVolumeIo->GetSectorRba(), 0x8);
}

TEST(VolumeIo, GetOriginVolumeIo)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io with constructor and Call copy constructor
    // Set newVolumeIo2 as newVolumeIo's origin
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    VolumeIoSmartPtr newVolumeIo2(new VolumeIo(*newVolumeIo, nullptr));
    newVolumeIo->SetOriginUbio(newVolumeIo2);
    // Then : Compare if GetVolumeIo and newVolumeIo2 is equal
    EXPECT_EQ(newVolumeIo->GetOriginVolumeIo(), newVolumeIo2);
}

TEST(VolumeIo, SetAndGetVolumeId)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io with constructor and Call copy constructor
    // Set volume Id
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    newVolumeIo->SetVolumeId(3);
    // Then : Compare if volume Id is what we expected
    EXPECT_EQ(newVolumeIo->GetVolumeId(), 3);
}

TEST(VolumeIo, IsPollingNecessary)
{
    // Given : buffer, unitCount, arrayId is given, wt off
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    NiceMock<MockIArrayMgmt> mockIArrayMgmt;
    NiceMock<MockIArrayInfo>* mockIArrayInfo = new NiceMock<MockIArrayInfo>();
    ComponentsInfo info{mockIArrayInfo, nullptr};
    // When : Create new Volume Io with Read direction
    ON_CALL(mockIArrayMgmt, GetInfo(arrayId)).WillByDefault(Return(&info));
    EXPECT_CALL(*mockIArrayInfo, IsWriteThroughEnabled()).WillOnce(Return(false));
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, &mockIArrayMgmt));
    newVolumeIo->dir = UbioDir::Read;

    // Then : Check if polling necessary is true
    EXPECT_EQ(newVolumeIo->IsPollingNecessary(), true);

    // When : Create new Volume Io with Write direction,
    // and its rba start is aligned as block size (4K)

    // 24 is multiple of 8, and 4k is 8 times of 512 bytes.
    // So, 24 is "block(4k) aligned"
    unitCount = 24;
    EXPECT_CALL(*mockIArrayInfo, IsWriteThroughEnabled()).WillOnce(Return(false));
    VolumeIoSmartPtr newVolumeIo2(new VolumeIo(buffer, unitCount, arrayId, &mockIArrayMgmt));
    newVolumeIo2->dir = UbioDir::Write;
    newVolumeIo2->SetSectorRba(0);
    // Then : Check if polling necessary is false
    EXPECT_EQ(newVolumeIo2->IsPollingNecessary(), false);

    // When : Create new Volume Io with Write direction,
    // but its rba start is NOT aligned as block size (4K)
    unitCount = 1;
    EXPECT_CALL(*mockIArrayInfo, IsWriteThroughEnabled()).WillOnce(Return(false))
                                                         .WillOnce(Return(false));
    VolumeIoSmartPtr newVolumeIo3(new VolumeIo(buffer, unitCount, arrayId, &mockIArrayMgmt));
    newVolumeIo3->dir = UbioDir::Write;
    // 8 is aligned to block size. but start lba : 2 is unaligned.
    newVolumeIo3->SetSectorRba(2);
    // Then : Check if polling necessary is false
    EXPECT_EQ(newVolumeIo3->IsPollingNecessary(), true);
    // 8 is aligned to block size. but its unitcount (1) is unaligned
    newVolumeIo3->SetSectorRba(8);
    // Then : Check if polling necessary is false
    EXPECT_EQ(newVolumeIo3->IsPollingNecessary(), true);

    // When : Create new Volume Io with Write direction with WT mode on
    EXPECT_CALL(*mockIArrayInfo, IsWriteThroughEnabled()).WillOnce(Return(true));
    VolumeIoSmartPtr newVolumeIo4(new VolumeIo(buffer, unitCount, arrayId, &mockIArrayMgmt));
    newVolumeIo3->dir = UbioDir::Write;

    // Then: always true
    EXPECT_EQ(newVolumeIo3->IsPollingNecessary(), true);
    delete mockIArrayInfo;
}

TEST(VolumeIo, GetOriginCore)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    // Then : Check if any segfault is not triggered
    newVolumeIo->GetOriginCore();
}

TEST(VolumeIo, SetLsidEntry)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io and Set lsid entry
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    StripeAddr stripeAddr = {.stripeLoc = StripeLoc::IN_USER_AREA,
                            .stripeId = 8216};
    newVolumeIo->SetLsidEntry(stripeAddr);
    // Then : Check if same Lsid is obtained from volumeIo
    StripeAddr stripeAddrReturned = newVolumeIo->GetLsidEntry();
    EXPECT_EQ(stripeAddr == stripeAddrReturned, true);
}

TEST(VolumeIo, SetOldLsidEntry)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io and Set lsid old entry
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    StripeAddr stripeAddr = {.stripeLoc = StripeLoc::IN_USER_AREA,
                            .stripeId = 8216};
    newVolumeIo->SetOldLsidEntry(stripeAddr);
    // Then : Check if same Lsid is obtained from volumeIo
    StripeAddr stripeAddrReturned = newVolumeIo->GetOldLsidEntry();
    EXPECT_EQ(stripeAddr == stripeAddrReturned, true);
}

TEST(VolumeIo, SetVsa)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    // When : Create new Volume Io and Set lsid old entry
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    VirtualBlkAddr virtualBlkAddr = {.stripeId = 8216,
                                    .offset = 15};
    newVolumeIo->SetVsa(virtualBlkAddr);
    VirtualBlkAddr virtualBlkAddr2 = newVolumeIo->GetVsa();
    EXPECT_EQ(virtualBlkAddr == virtualBlkAddr2, true);
}


TEST(VolumeIo, SetUserLsid)
{
    // Given : buffer, unitCount, arrayId is given
    void* buffer = nullptr;
    uint32_t unitCount = 20;
    int arrayId = 0;
    
    // When : Create new Volume Io and Set WbLsid
    VolumeIoSmartPtr newVolumeIo(new VolumeIo(buffer, unitCount, arrayId, nullptr));
    StripeId stripeId = 0;
    newVolumeIo->SetUserLsid(stripeId);
    StripeId stripeId2 = newVolumeIo->GetUserLsid();
    EXPECT_EQ(stripeId == stripeId2, true);    
}

} // namespace pos
