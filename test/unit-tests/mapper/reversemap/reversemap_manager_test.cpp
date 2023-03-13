#include "src/mapper/reversemap/reversemap_manager.h"

#include <gtest/gtest.h>

#include "src/mapper/address/mapper_address_info.h"
#include "test/unit-tests/mapper/address/mapper_address_info_mock.h"
#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/i_vsamap_mock.h"
#include "test/unit-tests/mapper/reversemap/reverse_map_mock.h"
#include "test/unit-tests/volume/i_volume_info_manager_mock.h"

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
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
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
    EXPECT_CALL(addrInfo, IsUT).WillRepeatedly(Return(true));
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();

    // When: Fail to get volume size through volume manager
    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    std::map<uint64_t, BlkAddr> revMapInfos;

    // ReconstructMap should be failed
    EXPECT_EQ(0, revMap.ReconstructReverseMap(0, NUM_BLKS_PER_STRIPE, 0, 0, 0, revMapInfos, nullptr));
}

TEST(ReverseMapManager, ReconstructReverseMap_testIfExecutedSuccesfullyWithEmptyReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is empty
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 1;
    StripeId lsid = 3;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructMap_testIfFailToGetMapInfoWithEmptyReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();

    // When: Fail to get vsa map
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);
    ON_CALL(ivsaMap, GetVSAWithSyncOpen).WillByDefault(Return(UNMAP_VSA));

    StripeId vsid = 100;
    StripeId lsid = 1000;
    uint32_t volumeId = 1;

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    // Then: ReverseMap should be unmmap
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = UINT64_MAX;
        BlkAddr expectVolumeId = (1 << 10) - 1;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(expectVolumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithFullReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is set fully
    uint64_t startOffset = 0;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfoStartWithZero)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = 0;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < 2; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfoUntilEnd)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 2;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));
    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
    revMap.Init();
    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 3;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructMap_testIfExecutedSuccesfullyWithSeveralPartialReverseMapInfo)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    ON_CALL(addrInfo, IsUT).WillByDefault(Return(true));
    ON_CALL(addrInfo, GetNumWbStripes).WillByDefault(Return(1003));
    ON_CALL(addrInfo, GetMaxVSID).WillByDefault(Return(103));
    ON_CALL(addrInfo, GetArrayId).WillByDefault(Return(0));
    ON_CALL(addrInfo, GetBlksPerStripe).WillByDefault(Return(344400));
    ON_CALL(addrInfo, GetMpageSize).WillByDefault(Return(4032));
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

    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack->GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }

    delete revMapPack;
}

TEST(ReverseMapManager, ReconstructReverseMap_testIfExecutedSuccesfully)
{
    // Given
    NiceMock<MockMapperAddressInfo> addrInfo;
    NiceMock<MockIVolumeInfoManager> volumeManager;
    NiceMock<MockIVSAMap> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;
    ReverseMapManager revMap(&ivsaMap, &iStripeMap, &volumeManager, &addrInfo, nullptr);
    EXPECT_CALL(addrInfo, IsUT).WillRepeatedly(Return(true));
    EXPECT_CALL(addrInfo, GetMaxVSID).WillRepeatedly(Return(103));
    EXPECT_CALL(addrInfo, GetArrayId).WillOnce(Return(0));
    EXPECT_CALL(addrInfo, GetBlksPerStripe).WillOnce(Return(344400));
    revMap.Init();

    // When: ReverseMapInfo is set partially
    std::map<uint64_t, BlkAddr> revMapInfos;
    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    ReverseMapPack* revMapPack = revMap.AllocReverseMapPack(vsid, lsid);
    EXPECT_EQ(expectRetCode, revMap.ReconstructReverseMap(volumeId, NUM_BLKS_PER_STRIPE, lsid, vsid, NUM_BLKS_PER_STRIPE, revMapInfos, revMapPack));

    auto invertedMap = revMap.GetInvertedMap()[volumeId];
    size_t size = 0;
    for (const auto& m : invertedMap)
    {
        size += m.second.size();
    }
    ASSERT_EQ(NUM_BLKS_PER_STRIPE, size);

    for (uint64_t offset = 0, expectRba = 0; offset < NUM_BLKS_PER_STRIPE; ++offset, ++expectRba)
    {
        auto result = invertedMap[vsid].find((uint64_t)offset);
        ASSERT_NE(result, invertedMap[vsid].end());
        EXPECT_EQ(expectRba, result->second);

        auto entry = revMapPack->GetReverseMapEntry(offset);
        EXPECT_EQ(expectRba, std::get<0>(entry));
        EXPECT_EQ(volumeId, std::get<1>(entry));
    }
}

} // namespace pos
