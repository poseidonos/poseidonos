#include "src/mapper/reversemap/reversemap_manager.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;

const uint64_t NUM_BLKS_PER_STRIPE = 64;

namespace pos
{
std::map<uint64_t, BlkAddr>
GenerateReverseMapInfo(int startOffset, int endOffset)
{
    std::map<uint64_t, BlkAddr> revMapInfos;
    for (BlkOffset offset = startOffset; offset < endOffset; offset++)
    {
        BlkAddr expectRba = offset;
        revMapInfos[offset] = expectRba;
    }
    return revMapInfos;
}

void
ExpectGetMapInfo(MockIVolumeInfoManager& volumeManager, MockIVSAMap& ivsaMap, MockIStripeMap& iStripeMap,
    uint32_t volumeId, StripeId vsid, StripeId lsid)
{
    const uint64_t volumeSize = 128 * BLOCK_SIZE;
    for (BlkOffset offset = 0; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        BlkAddr startRba = offset;
        VirtualBlkAddr vsaToCheck{
            .stripeId = vsid,
            .offset = offset};
        ON_CALL(ivsaMap, GetVSAWithSyncOpen(volumeId, startRba)).WillByDefault(Return(vsaToCheck));
    }
    StripeAddr lsaToCheck{
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = lsid};
    ON_CALL(iStripeMap, GetLSA(vsid)).WillByDefault(Return(lsaToCheck));
}

TEST(ReverseMapManager, ReverseMapManager_TestInit)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    ReverseMapManager* revMap = new ReverseMapManager(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    revMap->Init();
    delete revMap;
}

TEST(ReverseMapManager, ReconstructMap_testIfFailToGetVolumeSize)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1003));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(103));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    revMap.Init();

    // When: Fail to get volume size through volume manager
    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    std::map<uint64_t, BlkAddr> revMapInfos;

    // ReconstructMap should be failed
    EXPECT_EQ(0, revMap.ReconstructReverseMap(0, NUM_BLKS_PER_STRIPE, 0, 0, 0, revMapInfos));
}

TEST(ReverseMapManager, ReconstructReverseMap_testIfExecutedSuccesfullyWithEmptyReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1003));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(103));

    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    revMap.Init();

    // When: ReverseMapInfo is empty
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 1;
    StripeId lsid = 3;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);
    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapManager, ReconstructMap_testIfFailToGetMapInfoWithEmptyReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1003));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(103));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    EXPECT_CALL(addrInfo, GetMpageSize).WillOnce(Return(4032));
    revMap.Init();

    // When: Fail to get vsa map
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);
    ON_CALL(ivsaMap, GetVSAWithSyncOpen).WillByDefault(Return(UNMAP_VSA));

    StripeId vsid = 100;
    StripeId lsid = 1000;
    int expectRetCode = 0;
    uint32_t volumeId = 1;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be unmmap
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = UINT64_MAX;
        BlkAddr expectVolumeId = (1 << 10) - 1;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(expectVolumeId, get<1>(actual));
    }
}
TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithFullReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1005));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(105));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    EXPECT_CALL(addrInfo, GetMpageSize).WillOnce(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is set fully
    uint64_t startOffset = 0;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfoStartWithZero)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1005));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(105));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    EXPECT_CALL(addrInfo, GetMpageSize).WillOnce(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = 0;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < 2; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfoUntilEnd)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1005));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(105));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    EXPECT_CALL(addrInfo, GetMpageSize).WillOnce(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 2;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);
    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));
    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1003));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(103));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    EXPECT_CALL(addrInfo, GetMpageSize).WillOnce(Return(4032));
    revMap.Init();
    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 3;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithSeveralPartialReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillOnce(Return(true));
    EXPECT_CALL(addrInfo, GetNumWbStripes).WillOnce(Return(1003));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillOnce(Return(103));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    EXPECT_CALL(addrInfo, GetMpageSize).WillOnce(Return(4032));
    revMap.Init();
    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 5;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 4;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    startOffset = NUM_BLKS_PER_STRIPE / 3;
    endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos2 = GenerateReverseMapInfo(startOffset, endOffset);
    revMapInfos.insert(revMapInfos2.begin(), revMapInfos2.end());

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMap.GetReverseMapEntry(nullptr, lsid, offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

} // namespace pos
