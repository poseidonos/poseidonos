#include "src/allocator_service/allocator_service.h"

#include <gtest/gtest.h>

#include "src/allocator/i_context_replayer.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/i_allocator_wbt_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(AllocatorService, AllocatorService_)
{
}

TEST(AllocatorService, RegisterAllocator_)
{
    // given
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    // when
    allocService.RegisterAllocator("aaa", ctxReplayer);
    allocService.RegisterAllocator("bbb", ctxReplayer);
    // then
    IContextReplayer* ret = allocService.GetIContextReplayer("aaa");
    EXPECT_EQ(ctxReplayer, ret);
    ret = allocService.GetIContextReplayer("bbb");
    EXPECT_EQ(ctxReplayer, ret);

    delete ctxReplayer;
}

TEST(AllocatorService, UnregisterAllocator_)
{
    // given
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    allocService.RegisterAllocator("aaa", ctxReplayer);
    allocService.RegisterAllocator("bbb", ctxReplayer);
    // when
    allocService.UnregisterAllocator("aaa");
    allocService.UnregisterAllocator("bbb");
    allocService.UnregisterAllocator("aaa");
    // then
    IContextReplayer* ret = allocService.GetIContextReplayer("aaa");
    EXPECT_EQ(nullptr, ret);

    delete ctxReplayer;
}

TEST(AllocatorService, GetIContextReplayer_TestSimpleGetter)
{
    // given
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    allocService.RegisterAllocator("", ctxReplayer);
    // when
    allocService.GetIContextReplayer("");
    delete ctxReplayer;
}

TEST(AllocatorService, GetIAllocatorWbt_TestSimpleGetter)
{
    AllocatorService allocService;
    allocService.GetIAllocatorWbt("");
}

} // namespace pos
