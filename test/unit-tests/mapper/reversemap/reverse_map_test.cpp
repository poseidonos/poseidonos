#include "src/mapper/reversemap/reverse_map.h"

#include <gtest/gtest.h>

#include "test/unit-tests/mapper/i_stripemap_mock.h"
#include "test/unit-tests/mapper/vsamap/vsamap_manager_mock.h"
#include "test/unit-tests/volume/i_volume_manager_mock.h"

using ::testing::_;
using testing::NiceMock;
using ::testing::Return;

namespace pos
{
} // namespace pos

namespace pos
{
} // namespace pos

namespace pos
{
} // namespace pos

namespace pos
{
} // namespace pos

namespace pos
{
const uint64_t NUM_BLKS_PER_STRIPE = 64;

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
ExpectGetMapInfo(MockIVolumeManager& volumeManager, MockVSAMapManager& ivsaMap, MockIStripeMap& iStripeMap,
    uint32_t volumeId, StripeId vsid, StripeId lsid)
{
    const uint64_t volumeSize = 128 * BLOCK_SIZE;

    EXPECT_CALL(volumeManager, GetVolumeSize).WillOnce([](int volId, uint64_t& volSize) -> int {
        volSize = volumeSize;
        return (int)POS_EVENT_ID::SUCCESS;
    });
    for (BlkOffset offset = 0; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        BlkAddr startRba = offset;
        VirtualBlkAddr vsaToCheck{
            .stripeId = vsid,
            .offset = offset};
        ON_CALL(ivsaMap, GetVSAWoCond(volumeId, startRba)).WillByDefault(Return(vsaToCheck));
    }
    StripeAddr lsaToCheck{
        .stripeLoc = IN_WRITE_BUFFER_AREA,
        .stripeId = lsid};
    ON_CALL(iStripeMap, GetLSA(vsid)).WillByDefault(Return(lsaToCheck));
}

TEST(ReverseMapPack, ReverseMapPack_)
{
}

TEST(ReverseMapPack, Init_)
{
}

TEST(ReverseMapPack, LinkVsid_)
{
}

TEST(ReverseMapPack, UnLinkVsid_)
{
}

TEST(ReverseMapPack, Load_)
{
}

TEST(ReverseMapPack, Flush_)
{
}

TEST(ReverseMapPack, SetReverseMapEntry_)
{
}

TEST(ReverseMapPack, ReconstructMap_testIfFailToGetVolumeSize)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: Fail to get volume size through volume manager
    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;

    EXPECT_CALL(volumeManager, GetVolumeSize).WillOnce(Return(-1));
    std::map<uint64_t, BlkAddr> revMapInfos;
    int expectRetCode = -(int)EID(GET_VOLUMESIZE_FAILURE);

    // ReconstructMap should be failed
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(0, 0, 0, 0, revMapInfos));
}

TEST(ReverseMapPack, ReconstructMap_testIfExecutedSuccesfullyWithEmptyReverseMapInfo)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: ReverseMapInfo is empty
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);
    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapPack, ReconstructMap_testIfFailToGetMapInfoWithEmptyReverseMapInfo)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(252 * 16, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: Fail to get vsa map
    uint64_t startOffset = 0;
    uint64_t endOffset = 0;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    const uint64_t volumeSize = 128 * BLOCK_SIZE;
    EXPECT_CALL(volumeManager, GetVolumeSize).WillOnce([](int volId, uint64_t& volSize) -> int {
        volSize = volumeSize;
        return (int)POS_EVENT_ID::SUCCESS;
    });
    ON_CALL(ivsaMap, GetVSAWoCond).WillByDefault(Return(UNMAP_VSA));

    StripeId vsid = 100;
    StripeId lsid = 1000;
    int expectRetCode = 0;
    uint32_t volumeId = 1;
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be unmmap
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = UINT64_MAX;
        BlkAddr expectVolumeId = (1 << 10) - 1;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(expectVolumeId, get<1>(actual));
    }
}
TEST(ReverseMapPack, ReconstructMap_testIfExecutedSuccesfullyWithFullReverseMapInfo)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: ReverseMapInfo is set fully
    uint64_t startOffset = 0;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapPack, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfoStartWithZero)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = 0;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapPack, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfoUntilEnd)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 2;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapPack, ReconstructMap_testIfExecutedSuccesfullyWithPartialReverseMapInfo)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

    // When: ReverseMapInfo is set partially
    uint64_t startOffset = NUM_BLKS_PER_STRIPE / 3;
    uint64_t endOffset = NUM_BLKS_PER_STRIPE / 2;
    std::map<uint64_t, BlkAddr> revMapInfos = GenerateReverseMapInfo(startOffset, endOffset);

    uint32_t volumeId = 1;
    StripeId vsid = 100;
    StripeId lsid = 1000;
    ExpectGetMapInfo(volumeManager, ivsaMap, iStripeMap, volumeId, vsid, lsid);

    int expectRetCode = 0;
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}

TEST(ReverseMapPack, ReconstructMap_testIfExecutedSuccesfullyWithSeveralPartialReverseMapInfo)
{
    // Given
    ReverseMapPack revMapPack;

    NiceMock<MockIVolumeManager> volumeManager;
    NiceMock<MockVSAMapManager> ivsaMap;
    NiceMock<MockIStripeMap> iStripeMap;

    const uint64_t numMpagesPerStripe = 1024;
    revMapPack.Init(0, numMpagesPerStripe, nullptr, "POSArray0");
    revMapPack.Init(&volumeManager, 0, &ivsaMap, &iStripeMap);

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
    EXPECT_EQ(expectRetCode, revMapPack.ReconstructMap(volumeId, vsid, lsid, NUM_BLKS_PER_STRIPE, revMapInfos));

    // Then: ReverseMap should be set
    for (BlkOffset offset = startOffset; offset < NUM_BLKS_PER_STRIPE; offset++)
    {
        std::tuple<BlkAddr, uint32_t> actual = revMapPack.GetReverseMapEntry(offset);
        BlkAddr expectRba = offset;
        EXPECT_EQ(expectRba, get<0>(actual));
        EXPECT_EQ(volumeId, get<1>(actual));
    }
}
TEST(ReverseMapPack, GetReverseMapEntry_)
{
}

TEST(ReverseMapPack, IsAsyncIoDone_)
{
}

TEST(ReverseMapPack, GetIoError_)
{
}

TEST(ReverseMapPack, WbtFileSyncIo_)
{
}

TEST(ReverseMapPack, _HeaderInit_)
{
}

TEST(ReverseMapPack, _SetTimeToHeader_)
{
}

TEST(ReverseMapPack, _ReverseMapGeometry_)
{
}

TEST(ReverseMapPack, _GetCurrentTime_)
{
}

TEST(ReverseMapPack, _RevMapPageIoDone_)
{
}

TEST(ReverseMapPack, _FindRba_)
{
}

} // namespace pos
