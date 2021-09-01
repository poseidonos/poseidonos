#include "src/mapper/vsamap/vsamap_content.h"

#include <gtest/gtest.h>

#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"
#include "test/unit-tests/mapper/map/map_header_mock.h"
#include "test/unit-tests/mapper/map/map_mock.h"
#include "test/unit-tests/metafs/include/metafs_service_mock.h"

using namespace ::testing;

namespace pos
{
TEST(VSAMapContent, VSAMapContent_)
{
    // function touched
}

TEST(VSAMapContent, Prepare_Default)
{
    // // Given
    // const int VOLID = 0;
    // const uint64_t BLKCNT = 1024 * 1024;    // 4K * 1M == 4GB

    // MockMetaFsService* mockMetaFsService = new MockMetaFsService;

    // MockMapHeader* mockMapHeader = new MockMapHeader;
    // EXPECT_CALL(*mockMapHeader, SetMpageSize);
    // EXPECT_CALL(*mockMapHeader, GetMpageSize).WillRepeatedly(Return(4032));

    // VSAMapContent sut;
    // sut.SetMapHeader(mockMapHeader);

    // // When
    // int reti = sut.Prepare(BLKCNT, VOLID);

    // // Then
    // EXPECT_EQ(reti, 0);
}

TEST(VSAMapContent, GetDirtyPages_Default)
{
    // Given
    const BlkAddr RBA_TEST = 0;
    uint64_t rbaCnt = 504 + 1;

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));

    VSAMapContent sut(0, 0, nullptr);
    sut.SetMapHeader(mockMapHeader);

    // When
    MpageList mpageList = sut.GetDirtyPages(RBA_TEST, rbaCnt);

    // Then
    EXPECT_EQ(mpageList.size(), 2);
}

TEST(VSAMapContent, InMemoryInit_Default)
{
#if 0
    // Given
    const int VOLID = 0;
    const uint64_t BLKCNT = 1024 * 1024; // 4K * 1M == 4GB

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));
    EXPECT_CALL(*mockMapHeader, GetMpageSize).WillRepeatedly(Return(4032));

    VSAMapContent sut(0, 0, nullptr);
    sut.SetMapHeader(mockMapHeader);

    // When
    int reti = sut.InMemoryInit(BLKCNT, VOLID);

    // Then
    EXPECT_EQ(reti, 0);
#endif
}

#if 0
TEST(VSAMapContent, GetEntry_NoMpage)
{
    
    // Given
    const int VOLID = 0;
    const uint64_t BLKCNT = 1024 * 1024; // 4K * 1M == 4GB
    const BlkAddr RBA_TEST = 0;

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));
    EXPECT_CALL(*mockMapHeader, GetMpageSize).WillRepeatedly(Return(4032));

    VSAMapContent sut(0, 0, nullptr);
    sut.SetMapHeader(mockMapHeader);
    sut.Init(BLKCNT / 504, sizeof(VirtualBlkAddr));

    // When
    VirtualBlkAddr rvsa = sut.GetEntry(RBA_TEST);

    // Then
    EXPECT_EQ(rvsa, UNMAP_VSA);
    
}

TEST(VSAMapContent, GetEntry_ValidMpage)
{
    // Given
    const int VOLID = 0;
    const uint64_t BLKCNT = 1024 * 1024; // 4K * 1M == 4GB
    const BlkAddr RBA_TEST = 0;
    const VirtualBlkAddr VSA = {.stripeId = 1, .offset = 2};

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));
    EXPECT_CALL(*mockMapHeader, GetMpageSize).WillRepeatedly(Return(4032));

    MockMap* mockMap = new MockMap;
    char* buf = new char[4032]();
    VirtualBlkAddr* mpageMap = (VirtualBlkAddr*)buf;
    mpageMap[RBA_TEST % 504] = VSA;
    EXPECT_CALL(*mockMap, GetMpage).WillRepeatedly(ReturnPointee(&buf));

    VSAMapContent sut(0, 0, nullptr);
    sut.SetMapHeader(mockMapHeader);
    sut.SetMap(mockMap);

    // When
    VirtualBlkAddr rvsa = sut.GetEntry(RBA_TEST);

    // Then
    EXPECT_EQ(rvsa, VSA);
}

TEST(VSAMapContent, SetEntry_VsaMapSetFailure)
{
    // Given
    const BlkAddr RBA_TEST = 0;
    const VirtualBlkAddr VSA = {.stripeId = 1, .offset = 2};

    MockMap* mockMap = new MockMap;
    EXPECT_CALL(*mockMap, GetMpageLock);
    EXPECT_CALL(*mockMap, GetMpage).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*mockMap, AllocateMpage).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*mockMap, ReleaseMpageLock);

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));

    VSAMapContent sut(0, 0, nullptr);
    sut.SetMapHeader(mockMapHeader);
    sut.SetMap(mockMap);

    // When
    int reti = sut.SetEntry(RBA_TEST, VSA);

    // Then
    EXPECT_EQ(reti, -EID(VSAMAP_SET_FAILURE));
}

TEST(VSAMapContent, SetEntry_AllocAndSet)
{
    // Given
    const BlkAddr RBA_TEST = 0;
    const VirtualBlkAddr VSA = {.stripeId = 1, .offset = 2};

    MockMap* mockMap = new MockMap;
    EXPECT_CALL(*mockMap, GetMpageLock);
    EXPECT_CALL(*mockMap, GetMpage).WillRepeatedly(Return(nullptr));
    char* buf = new char[4032]();
    EXPECT_CALL(*mockMap, AllocateMpage).WillRepeatedly(ReturnPointee(&buf));
    EXPECT_CALL(*mockMap, ReleaseMpageLock);

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));
    EXPECT_CALL(*mockMapHeader, SetMapAllocated);

    MockBitMap* mockBitMap = new MockBitMap(1024);
    EXPECT_CALL(*mockBitMap, SetBit);
    EXPECT_CALL(*mockMapHeader, GetTouchedMpages).WillRepeatedly(ReturnPointee(&mockBitMap));

    VSAMapContent sut(0, 0, nullptr);
    sut.SetMapHeader(mockMapHeader);
    sut.SetMap(mockMap);

    // When
    int reti = sut.SetEntry(RBA_TEST, VSA);

    // Then
    EXPECT_EQ(reti, 0);
    delete mockBitMap;
}

TEST(VSAMapContent, GetNumUsedBlocks_CountOnePage)
{
    // // Given
    // MockMap* mockMap = new MockMap;
    // char* buf = new char[4032]();
    // EXPECT_CALL(*mockMap, GetMpageWithLock).WillRepeatedly(ReturnPointee(&buf));

    // MockBitMap* mockBitMap = new MockBitMap(1024);
    // EXPECT_CALL(*mockBitMap, FindFirstSet).WillOnce(Return(0)).WillOnce(Return(16));
    // EXPECT_CALL(*mockBitMap, GetNumBits).WillRepeatedly(Return(16));

    // MockMapHeader* mockMapHeader = new MockMapHeader;
    // EXPECT_CALL(*mockMapHeader, GetMpageMap).WillRepeatedly(ReturnPointee(&mockBitMap));
    // EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));

    // VSAMapContent sut;
    // sut.SetMapHeader(mockMapHeader);
    // sut.SetMap(mockMap);

    // // When
    // int64_t reti = sut.GetNumUsedBlocks();

    // // Then
    // EXPECT_EQ(reti, 504);
    // delete mockBitMap;
}

TEST(VSAMapContent, InvalidateAllBlocks_Default)
{
    // Given
    MockIBlockAllocator* mockIBlockAllocator = new MockIBlockAllocator;
    EXPECT_CALL(*mockIBlockAllocator, InvalidateBlks).WillRepeatedly(Return());

    MockMap* mockMap = new MockMap;
    EXPECT_CALL(*mockMap, GetMpageLock);
    EXPECT_CALL(*mockMap, ReleaseMpageLock);
    char* buf = new char[4032]();
    EXPECT_CALL(*mockMap, GetMpage).WillRepeatedly(ReturnPointee(&buf));

    MockBitMap* mockBitMap = new MockBitMap(1024);
    EXPECT_CALL(*mockBitMap, FindFirstSet).WillOnce(Return(0)).WillOnce(Return(16));
    EXPECT_CALL(*mockBitMap, GetNumBits).WillRepeatedly(Return(16));

    MockMapHeader* mockMapHeader = new MockMapHeader;
    EXPECT_CALL(*mockMapHeader, GetMpageMap).WillRepeatedly(ReturnPointee(&mockBitMap));
    EXPECT_CALL(*mockMapHeader, GetEntriesPerMpage).WillRepeatedly(Return(504));

    VSAMapContent sut(0, 0, mockIBlockAllocator);
    sut.SetMapHeader(mockMapHeader);
    sut.SetMap(mockMap);

    // When
    int reti = sut.InvalidateAllBlocks();

    // Then
    EXPECT_EQ(reti, 0);
    delete mockIBlockAllocator;
    delete mockBitMap;
}

TEST(VSAMapContent, _GetNumValidEntries_)
{
    // touched function
}
#endif
} // namespace pos
