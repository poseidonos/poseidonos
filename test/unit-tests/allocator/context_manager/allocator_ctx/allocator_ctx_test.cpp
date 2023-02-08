#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "src/include/meta_const.h"
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
TEST(AllocatorCtx, AfterLoad_testIfAllMetadataIsCopiedToContext)
{
    // given
    AllocatorCtxHeader header;

    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    char* bitmapAddr = new char[10 * BITMAP_ENTRY_SIZE];
    ON_CALL(*allocBitmap, GetMapAddr).WillByDefault(Return((uint64_t*)bitmapAddr));
    ON_CALL(*allocBitmap, GetNumEntry).WillByDefault(Return(10));

    NiceMock<MockAllocatorAddressInfo> addrInfo;
    ON_CALL(addrInfo, GetnumWbStripes).WillByDefault(Return(100));

    AllocatorCtx allocCtx(nullptr, &header, allocBitmap, &addrInfo);
    allocCtx.Init();

    // when
    char* buf = new char[allocCtx.GetTotalDataSize()];

    auto bufferOffset = allocCtx.GetSectionInfo(AC_HEADER).offset;
    AllocatorCtxHeader* headerPtr = reinterpret_cast<AllocatorCtxHeader*>(buf + bufferOffset);
    headerPtr->sig = SIG_ALLOCATOR_CTX;
    headerPtr->ctxVersion = 13;
    headerPtr->numValidWbLsid = 20;

    bufferOffset = allocCtx.GetSectionInfo(AC_CURRENT_SSD_LSID).offset;
    StripeId* currentSsdLsidPtr = reinterpret_cast<StripeId*>(buf + bufferOffset);
    *currentSsdLsidPtr = 1000;

    bufferOffset = allocCtx.GetSectionInfo(AC_ACTIVE_STRIPE_TAIL).offset;
    VirtualBlkAddr* vsaList = reinterpret_cast<VirtualBlkAddr*>(buf + bufferOffset);
    for (auto index = 0; index < 5; index++)
    {
        vsaList[index].stripeId = index;
        vsaList[index].offset = 0;
    }

    EXPECT_CALL(*allocBitmap, SetNumBitsSet(20));

    allocCtx.AfterLoad(buf);
    EXPECT_EQ(allocCtx.GetStoredVersion(), headerPtr->ctxVersion);
    EXPECT_EQ(allocCtx.GetCurrentSsdLsid(), 1000);

    for (auto index = 0; index < 5; index++)
    {
        auto tail = allocCtx.GetActiveStripeTail(index);
        EXPECT_EQ(tail.stripeId, index);
        EXPECT_EQ(tail.offset, 0);
    }

    delete[] buf;
    delete[] bitmapAddr;
}

TEST(AllocatorCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr);
    // when
    allocCtx.GetStoredVersion();
}

TEST(AllocatorCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr);
    // when
    allocCtx.ResetDirtyVersion();
}

TEST(AllocatorCtx, SetCurrentSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr);
    // when
    allocCtx.SetCurrentSsdLsid(10);
}

TEST(AllocatorCtx, SetNextSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetstripesPerSegment(100);

    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, &addrInfo);

    // when
    allocCtx.SetNextSsdLsid(0);
}

TEST(AllocatorCtx, GetCtxLock_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr);
    // when
    std::mutex& m = allocCtx.GetCtxLock();
}

TEST(AllocatorCtx, Init_InitAndClose)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    addrInfo.SetnumWbStripes(10);
    AllocatorCtx allocCtx(nullptr, &addrInfo);
    allocCtx.Dispose();

    // when
    allocCtx.Init();
    for (int i = 0; i < 10; i++)
    {
        VirtualBlkAddr vsa = allocCtx.GetActiveStripeTail(i);
        EXPECT_EQ(vsa, UNMAP_VSA);
    }
    allocCtx.Dispose();
}

TEST(AllocatorCtx, GetCurrentSsdLsid_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr);

    // when
    allocCtx.GetCurrentSsdLsid();
}

TEST(AllocatorCtx, BeforeFlush_testIfAllMetaIsCopiedToBuffer)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    char* bitmapAddr = new char[10 * BITMAP_ENTRY_SIZE];
    ON_CALL(*allocBitmap, GetMapAddr).WillByDefault(Return((uint64_t*)bitmapAddr));
    ON_CALL(*allocBitmap, GetNumEntry).WillByDefault(Return(10));

    NiceMock<MockAllocatorAddressInfo> addrInfo;
    ON_CALL(addrInfo, GetnumWbStripes).WillByDefault(Return(100));

    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, &addrInfo);
    allocCtx.Init();

    char* buf = new char[allocCtx.GetTotalDataSize()];

    EXPECT_CALL(*allocBitmap, GetNumBitsSetWoLock).WillOnce(Return(10));

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        allocCtx.SetActiveStripeTail(i, vsa);
    }

    std::mutex lock;
    EXPECT_CALL(*allocBitmap, GetLock).WillOnce(ReturnRef(lock));

    // when
    allocCtx.BeforeFlush((char*)buf);

    // then
    // AC_HEADER
    auto bufferOffset = allocCtx.GetSectionInfo(AC_HEADER).offset;
    AllocatorCtxHeader* header = reinterpret_cast<AllocatorCtxHeader*>(buf + bufferOffset);
    EXPECT_EQ(header->numValidWbLsid, 10);

    // AC_CURRENT_SSD_LSID
    bufferOffset = allocCtx.GetSectionInfo(AC_CURRENT_SSD_LSID).offset;
    StripeId currentSsdLsid = *reinterpret_cast<StripeId*>(buf + bufferOffset);
    EXPECT_EQ(currentSsdLsid, STRIPES_PER_SEGMENT - 1);

    // AC_ALLOCATE_WBLSID_BITMAP
    // TODO

    // AC_ACTIVE_STRIPE_TAIL
    bufferOffset = allocCtx.GetSectionInfo(AC_ACTIVE_STRIPE_TAIL).offset;
    VirtualBlkAddr* vList = reinterpret_cast<VirtualBlkAddr*>(buf + bufferOffset);
    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        EXPECT_EQ(i, vList[i].offset);
        EXPECT_EQ(i, vList[i].stripeId);
    }

    delete[] buf;
    delete[] bitmapAddr;
}

TEST(AllocatorCtx, AfterFlush_TestSimpleSetter)
{
    // given
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    ON_CALL(addrInfo, GetnumWbStripes).WillByDefault(Return(100));
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, &addrInfo);
    allocCtx.Init();

    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    buf->sig = SIG_ALLOCATOR_CTX;
    buf->ctxVersion = 12;
    AsyncMetaFileIoCtx ctx;
    ctx.SetIoInfo(MetaFsIoOpcode::Write, 0, sizeof(buf), (char*)buf);

    // when
    allocCtx.AfterFlush(ctx.GetBuffer());

    // then
    EXPECT_EQ(allocCtx.GetStoredVersion(), 12);

    delete buf;
}

TEST(AllocatorCtx, GetSectionInfo_TestGetEachSectionSize)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    ON_CALL(*allocBitmap, GetMapAddr).WillByDefault(Return((uint64_t*)100));
    ON_CALL(*allocBitmap, GetNumEntry).WillByDefault(Return(100));
    NiceMock<MockAllocatorAddressInfo> addrInfo;
    ON_CALL(addrInfo, GetnumWbStripes).WillByDefault(Return(100));
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, &addrInfo);
    allocCtx.Init();

    // when 1.
    uint64_t expectedOffset = 0;
    auto ret = allocCtx.GetSectionInfo(AC_HEADER);
    // then 1.
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(sizeof(AllocatorCtxHeader), ret.size);
    expectedOffset += sizeof(AllocatorCtxHeader);

    // when 2.
    ret = allocCtx.GetSectionInfo(AC_CURRENT_SSD_LSID);
    // then 2.
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(sizeof(StripeId), ret.size);
    expectedOffset += sizeof(StripeId);

    // when 3.
    ret = allocCtx.GetSectionInfo(AC_ALLOCATE_WBLSID_BITMAP);
    // then 3.
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(100 * BITMAP_ENTRY_SIZE, ret.size);
    expectedOffset += 100 * BITMAP_ENTRY_SIZE;

    // when 4.
    ret = allocCtx.GetSectionInfo(AC_ACTIVE_STRIPE_TAIL);
    // then 4.
    EXPECT_EQ(expectedOffset, ret.offset);
    EXPECT_EQ(ACTIVE_STRIPE_TAIL_ARRAYLEN * sizeof(VirtualBlkAddr), ret.size);
}

TEST(AllocatorCtx, AllocWbStripeWithParam_TestSimpleInterfaceFunc)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);
    EXPECT_CALL(*allocBitmap, SetBit(10));
    // when
    allocCtx.AllocWbStripe(10);
    delete allocBitmap;
}

TEST(AllocatorCtx, AllocFreeWbStripe_TestAllocStripeWithConditions)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    // given 1.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(true));
    // when 1.
    int ret = allocCtx.AllocFreeWbStripe();
    // then 1.
    EXPECT_EQ(10, ret);

    // given 2.
    EXPECT_CALL(*allocBitmap, SetNextZeroBit).WillOnce(Return(10));
    EXPECT_CALL(*allocBitmap, IsValidBit).WillOnce(Return(false));
    // when 2.
    ret = allocCtx.AllocFreeWbStripe();
    // then 2.
    EXPECT_EQ(UNMAP_STRIPE, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, ReleaseWbStripe_TestSimpleSetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, ClearBit);

    // when
    allocCtx.ReleaseWbStripe(10);

    delete allocBitmap;
}

TEST(AllocatorCtx, SetAllocatedWbStripeCount_TestSimpleSetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, SetNumBitsSet);

    // when
    allocCtx.SetAllocatedWbStripeCount(50);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetAllocatedWbStripeCount_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, GetNumBitsSet).WillOnce(Return(50));

    // when
    int ret = allocCtx.GetAllocatedWbStripeCount();
    // then
    EXPECT_EQ(50, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetNumTotalWbStripe_TestSimpleGetter)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, GetNumBits).WillOnce(Return(100));

    // when
    int ret = allocCtx.GetNumTotalWbStripe();
    // then
    EXPECT_EQ(100, ret);

    delete allocBitmap;
}

TEST(AllocatorCtx, GetActiveStripeTail_TestGetDataAndCheckData)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        allocCtx.SetActiveStripeTail(i, vsa);
    }

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        // when
        VirtualBlkAddr vsa = allocCtx.GetActiveStripeTail(i);
        // then
        EXPECT_EQ(i, vsa.offset);
        EXPECT_EQ(i, vsa.stripeId);
    }

    delete allocBitmap;
}

TEST(AllocatorCtx, SetActiveStripeTail_TestSetDataAndCheckData)
{
    // given
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, nullptr);

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        allocCtx.SetActiveStripeTail(i, vsa);
    }

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        // when
        VirtualBlkAddr vsa = allocCtx.GetActiveStripeTail(i);
        // then
        EXPECT_EQ(i, vsa.offset);
        EXPECT_EQ(i, vsa.stripeId);
    }

    delete allocBitmap;
}

TEST(AllocatorCtx, GetActiveStripeTailLock_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(10);
    AllocatorCtx allocCtx(nullptr, &addrInfo);

    // when
    std::mutex& m = allocCtx.GetActiveStripeTailLock(0);
}

TEST(AllocatorCtx, GetAllActiveStripeTail_TestSimpleGetter)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumWbStripes(10);
    AllocatorCtx allocCtx(nullptr, &addrInfo);

    for (int i = 0; i < 10; i++)
    {
        VirtualBlkAddr vsa;
        vsa.stripeId = i;
        vsa.offset = i;
        allocCtx.SetActiveStripeTail(i, vsa);
    }

    // when
    int cnt = 0;
    std::vector<VirtualBlkAddr> ret = allocCtx.GetAllActiveStripeTail();
    // then
    for (auto i = ret.begin(); cnt < 10; i++, cnt++)
    {
        EXPECT_EQ(cnt, i->stripeId);
        EXPECT_EQ(cnt, i->offset);
    }
}

} // namespace pos
