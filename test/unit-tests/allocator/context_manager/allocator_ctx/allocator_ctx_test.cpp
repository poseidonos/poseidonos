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
    header.sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    header.ctxVersion = 12;
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, &header, allocBitmap, nullptr);

    EXPECT_CALL(*allocBitmap, SetNumBitsSet(100));

    // when
    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    buf->numValidWbLsid = 100;

    allocCtx.AfterLoad((char*)buf);
    EXPECT_EQ(allocCtx.GetStoredVersion(), header.ctxVersion);

    delete buf;
    delete allocBitmap;
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

TEST(AllocatorCtx, BeforeFlush_TestACHeader)
{
    // given
    NiceMock<MockBitMapMutex> allocBitmap(100);
    AllocatorCtx allocCtx(nullptr, nullptr, &allocBitmap, nullptr);
    char* buf = new char[sizeof(AllocatorCtxHeader) + ACTIVE_STRIPE_TAIL_ARRAYLEN * sizeof(VirtualBlkAddr)];

    EXPECT_CALL(allocBitmap, GetNumBitsSetWoLock).WillOnce(Return(10));

    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        VirtualBlkAddr vsa;
        vsa.offset = i;
        vsa.stripeId = i;
        allocCtx.SetActiveStripeTail(i, vsa);
    }

    std::mutex lock;
    EXPECT_CALL(allocBitmap, GetLock).WillOnce(ReturnRef(lock));

    // when
    allocCtx.BeforeFlush((char*)buf);

    // then
    AllocatorCtxHeader* header = reinterpret_cast<AllocatorCtxHeader*>(buf);
    EXPECT_EQ(header->numValidWbLsid, 10);

    VirtualBlkAddr* vList = reinterpret_cast<VirtualBlkAddr*>(buf + sizeof(AllocatorCtxHeader));
    for (int i = 0; i < ACTIVE_STRIPE_TAIL_ARRAYLEN; i++)
    {
        EXPECT_EQ(i, vList[i].offset);
        EXPECT_EQ(i, vList[i].stripeId);
    }

    delete[] buf;
}

TEST(AllocatorCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr, nullptr, nullptr);

    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    buf->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    buf->ctxVersion = 12;
    AsyncMetaFileIoCtx ctx;
    ctx.buffer = (char*)buf;

    // when
    allocCtx.FinalizeIo(&ctx);

    // then
    EXPECT_EQ(allocCtx.GetStoredVersion(), 12);

    delete buf;
}

TEST(AllocatorCtx, GetSectionAddr_TestGetEachSectionAddr)
{
    // given
    NiceMock<MockBitMapMutex> allocBitmap(100);
    AllocatorCtx allocCtx(nullptr, nullptr, &allocBitmap, nullptr);

    // when 1.
    char* ret = allocCtx.GetSectionAddr(AC_HEADER);

    // when 2.
    ret = allocCtx.GetSectionAddr(AC_CURRENT_SSD_LSID);

    // when 3.
    EXPECT_CALL(allocBitmap, GetMapAddr).WillOnce(Return((uint64_t*)100));
    // when 1.
    ret = allocCtx.GetSectionAddr(AC_ALLOCATE_WBLSID_BITMAP);
    // then
    EXPECT_EQ((char*)(100), ret);

    // when 4.
    ret = allocCtx.GetSectionAddr(AC_ACTIVE_STRIPE_TAIL);
}

TEST(AllocatorCtx, GetSectionSize_TestGetEachSectionSize)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    NiceMock<MockBitMapMutex>* allocBitmap = new NiceMock<MockBitMapMutex>(100);
    AllocatorCtx allocCtx(nullptr, nullptr, allocBitmap, &addrInfo);

    // when 1.
    int ret = allocCtx.GetSectionSize(AC_HEADER);
    // then 1.
    EXPECT_EQ(sizeof(AllocatorCtxHeader), ret);

    // when 2.
    ret = allocCtx.GetSectionSize(AC_CURRENT_SSD_LSID);
    // then 2.
    EXPECT_EQ(sizeof(StripeId), ret);

    // when 3.
    EXPECT_CALL(*allocBitmap, GetNumEntry).WillOnce(Return(100));
    ret = allocCtx.GetSectionSize(AC_ALLOCATE_WBLSID_BITMAP);
    // then 3.
    EXPECT_EQ(100 * BITMAP_ENTRY_SIZE, ret);

    // when 4.
    ret = allocCtx.GetSectionSize(AC_ACTIVE_STRIPE_TAIL);
    // then 4.
    EXPECT_EQ(ACTIVE_STRIPE_TAIL_ARRAYLEN * sizeof(VirtualBlkAddr), ret);

    delete allocBitmap;
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
