#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"

#include <gtest/gtest.h>

#include "src/allocator/address/allocator_address_info.h"
#include "test/unit-tests/allocator/address/allocator_address_info_mock.h"
#include "test/unit-tests/allocator/context_manager/allocator_ctx/allocator_ctx_mock.h"

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
    AllocatorCtx allocCtx(&header, nullptr);

    // when
    allocCtx.AfterLoad(nullptr);
}

TEST(AllocatorCtx, GetStoredVersion_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    // when
    allocCtx.GetStoredVersion();
}

TEST(AllocatorCtx, ResetDirtyVersion_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    // when
    allocCtx.ResetDirtyVersion();
}

TEST(AllocatorCtx, UpdatePrevLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    // when
    allocCtx.UpdatePrevLsid();
}

TEST(AllocatorCtx, SetCurrentSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    // when
    allocCtx.SetCurrentSsdLsid(10);
}

TEST(AllocatorCtx, RollbackCurrentSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    // when
    allocCtx.RollbackCurrentSsdLsid();
}

TEST(AllocatorCtx, SetPrevSsdLsid_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
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

    AllocatorCtx allocCtx(nullptr, &addrInfo);

    // when
    allocCtx.SetNextSsdLsid(0);
}

TEST(AllocatorCtx, GetAllocatorCtxLock_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    // when
    std::mutex& m = allocCtx.GetAllocatorCtxLock();
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

TEST(AllocatorCtx, GetPrevSsdLsid_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);
    allocCtx.SetPrevSsdLsid(10);
    // when
    int ret = allocCtx.GetPrevSsdLsid();
    // then
    EXPECT_EQ(10, ret);
}

TEST(AllocatorCtx, GetCurrentSsdLsid_TestSimpleGetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);

    // when
    allocCtx.GetCurrentSsdLsid();
}

TEST(AllocatorCtx, BeforeFlush_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);

    // when
    allocCtx.BeforeFlush(0, nullptr);
}

TEST(AllocatorCtx, FinalizeIo_TestSimpleSetter)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);

    AllocatorCtxHeader* buf = new AllocatorCtxHeader();
    buf->sig = AllocatorCtx::SIG_ALLOCATOR_CTX;
    AsyncMetaFileIoCtx ctx;
    ctx.buffer = (char*)buf;

    // when
    allocCtx.FinalizeIo(&ctx);

    delete buf;
}

TEST(AllocatorCtx, GetSectionAddr_TestGetEachSectionAddr)
{
    // given
    AllocatorCtx allocCtx(nullptr, nullptr);

    // when 1.
    char* ret = allocCtx.GetSectionAddr(AC_HEADER);

    // when 2.
    ret = allocCtx.GetSectionAddr(AC_CURRENT_SSD_LSID);

}

TEST(AllocatorCtx, GetSectionSize_TestGetEachSectionSize)
{
    // given
    AllocatorAddressInfo addrInfo;
    addrInfo.SetnumUserAreaSegments(10);
    AllocatorCtx allocCtx(nullptr, &addrInfo);

    // when 1.
    int ret = allocCtx.GetSectionSize(AC_HEADER);
    // then 1.
    EXPECT_EQ(sizeof(AllocatorCtxHeader), ret);

    // when 2.
    ret = allocCtx.GetSectionSize(AC_CURRENT_SSD_LSID);
    // then 2.
    EXPECT_EQ(sizeof(StripeId), ret);
}

} // namespace pos
