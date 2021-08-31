#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"
#include "test/unit-tests/lib/bitmap_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(AllocatorCtx, AfterLoad_TestCheckingSignatureSuccess)
{
    // given
    AllocatorCtxHeader header;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    header.sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    AllocatorCtx allocCtx(&header, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, SetNumBitsSet).WillOnce(Return());
    // when
    allocCtx.AfterLoad(nullptr);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr);
    // when
    allocCtx.GetStoredVersion();
}

TEST(AllocatorCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr);
    // when
    allocCtx.ResetDirtyVersion();
}

TEST(AllocatorCtx, UpdatePrevLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr);
    // when
    allocCtx.UpdatePrevLsid();
}

TEST(AllocatorCtx, SetCurrentSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr);
    // when
    allocCtx.SetCurrentSsdLsid(10);
}

TEST(AllocatorCtx, RollbackCurrentSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr);
    // when
    allocCtx.RollbackCurrentSsdLsid();
}

TEST(AllocatorCtx, SetPrevSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr);
    // when
    allocCtx.SetPrevSsdLsid(10);
    // then
    int ret = allocCtx.GetPrevSsdLsid();
    EXPECT_EQ(10, ret);
}

TEST(AllocatorCtx, SetNextSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, &addrInfo);

    // when
    allocCtx.SetNextSsdLsid(0);

    delete allocBitmap;
}

TEST(AllocatorCtx, AllocateSegment_TestSimpleInterfaceFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, SetBit);
    // when
    allocCtx.AllocateSegment(10);

    delete allocBitmap;
}

TEST(AllocatorCtx, ReleaseSegment_TestSimpleInterfaceFunc)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, ClearBit);
    // when
    allocCtx.ReleaseSegment(10);

    delete allocBitmap;
}

TEST(AllocatorCtx, AllocateFreeSegment_TestAllocSegmentWithCheckingConditions)
{
    // given
    AllocatorAddressInfo addrInfo;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    // given 1.
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).WillOnce(Return(22));
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 1.
    int ret = allocCtx.AllocateFreeSegment(10);
    // then 1.
    EXPECT_EQ(22, ret);

    // given 2.
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).WillOnce(Return(22));
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = allocCtx.AllocateFreeSegment(10);
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    // given 3.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 3.
    ret = allocCtx.AllocateFreeSegment(UNMAP_SEGMENT);
    // then 3.
    EXPECT_EQ(10, ret);

    // given 4.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, SetFirstZeroBit).Times(0);
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 4.
    ret = allocCtx.AllocateFreeSegment(UNMAP_SEGMENT);
    // then 4.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetNumOfFreeSegment_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);
    EXPECT_CALL(*allocBitmap, GetNumBits).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, GetNumBitsSet).WillOnce(Return(3));
    int ret = allocCtx.GetNumOfFreeSegment();
    EXPECT_EQ(7, ret);

    EXPECT_CALL(*allocBitmap, GetNumBits).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, GetNumBitsSetWoLock).WillOnce(Return(3));
    ret = allocCtx.GetNumOfFreeSegmentWoLock();
    EXPECT_EQ(7, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, SetAllocatedSegmentCount_TestSimpleSEtter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, SetNumBitsSet);
    // when 1.
    allocCtx.SetAllocatedSegmentCount(10);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetAllocatedSegmentCount_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, GetNumBitsSet).WillOnce(Return(10));
    // when
    int ret = allocCtx.GetAllocatedSegmentCount();
    EXPECT_EQ(10, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetTotalSegmentsCount_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, GetNumBits).WillOnce(Return(100));
    // when
    int ret = allocCtx.GetTotalSegmentsCount();
    EXPECT_EQ(100, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetAllocatorCtxLock_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);
    // when
    std::mutex& m = allocCtx.GetAllocatorCtxLock();

    delete allocBitmap;
}

TEST(AllocatorCtx, Init_InitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    AllocatorCtx allocCtx(&addrInfo);
    // when
    allocCtx.Init();
    allocCtx.Dispose();
}

TEST(AllocatorCtx, GetUsedSegment_TestGetAllocatedSegment)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    // given 1.
    EXPECT_CALL(*allocBitmap, FindFirstSetBit).WillOnce(Return(5));
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 1.
    int ret = allocCtx.GetUsedSegment(0);
    // then 1.
    EXPECT_EQ(5, ret);

    // given 2.
    EXPECT_CALL(*allocBitmap, FindFirstSetBit).WillOnce(Return(5));
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = allocCtx.GetUsedSegment(0);
    // then 2.
    EXPECT_EQ(UNMAP_SEGMENT, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetPrevSsdLsid_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);
    allocCtx.SetPrevSsdLsid(10);
    // when
    int ret = allocCtx.GetPrevSsdLsid();
    // then
    EXPECT_EQ(10, ret);
    delete allocBitmap;
}

TEST(AllocatorCtx, GetCurrentSsdLsid_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    // when
    allocCtx.GetCurrentSsdLsid();

    delete allocBitmap;
}

TEST(AllocatorCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);
    EXPECT_CALL(*allocBitmap, GetNumBitsSet);
    // when
    allocCtx.BeforeFlush(0, nullptr);
    delete allocBitmap;
}

TEST(AllocatorCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    buf->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    AsyncMetaFileIoCtx ctx;
    ctx.buffer = (char*)buf;

    // when
    allocCtx.FinalizeIo(&ctx);

    delete allocBitmap;
    delete buf;
}

TEST(AllocatorCtx, GetSectionAddr_TestGetEachSectionAddr)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, nullptr);

    // when 1.
    char* ret = allocCtx.GetSectionAddr(AC_HEADER);
    // given 2.
    EXPECT_CALL(*allocBitmap, GetMapAddr).WillOnce(Return((uint64_t*)100));
    // when 2.
    ret = allocCtx.GetSectionAddr(AC_ALLOCATE_SEGMENT_BITMAP);
    // then 2.
    EXPECT_EQ(reinterpret_cast<char*>(100), ret);
    // when 3.
    ret = allocCtx.GetSectionAddr(AC_CURRENT_SSD_LSID);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetSectionSize_TestGetEachSectionSize)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, allocBitmap, &addrInfo);

    // when 1.
    int ret = allocCtx.GetSectionSize(AC_HEADER);
    // then 1.
    EXPECT_EQ(sizeof(AllocatorCtxHeader), ret);

    // given 2.
    EXPECT_CALL(*allocBitmap, GetNumEntry).WillOnce(Return(10));
    // when 2.
    ret = allocCtx.GetSectionSize(AC_ALLOCATE_SEGMENT_BITMAP);
    // then 2.
    EXPECT_EQ((10 * BITMAP_ENTRY_SIZE), ret);

    // when 3.
    ret = allocCtx.GetSectionSize(AC_CURRENT_SSD_LSID);
    // then 3.
    EXPECT_EQ(sizeof(StripeId), ret);

    delete allocBitmap;
}

} // namespace pos
