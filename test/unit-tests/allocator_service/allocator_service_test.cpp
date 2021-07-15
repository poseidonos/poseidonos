#include "src/allocator_service/allocator_service.h"

#include <gtest/gtest.h>

#include "src/allocator/i_context_replayer.h"
#include "test/unit-tests/allocator/context_manager/context_manager_mock.h"
#include "test/unit-tests/allocator/context_manager/context_replayer_mock.h"
#include "test/unit-tests/allocator/i_allocator_wbt_mock.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(AllocatorService, RegisterAllocator_TestFunc)
{
    // given
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    // when
    allocService.RegisterAllocator("aaa", 0, nullptr, nullptr, nullptr, nullptr, ctxReplayer);
    allocService.RegisterAllocator("bbb", 1, nullptr, nullptr, nullptr, nullptr, ctxReplayer);

    // then
    IContextReplayer* ret = allocService.GetIContextReplayer("aaa");
    EXPECT_EQ(ctxReplayer, ret);
    ret = allocService.GetIContextReplayer("bbb");
    EXPECT_EQ(ctxReplayer, ret);

    delete ctxReplayer;
}

TEST(AllocatorService, UnregisterAllocator_TestSuccess)
{
    // given
    NiceMock<MockContextReplayer>* ctxReplayer = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    allocService.RegisterAllocator("aaa", 0, nullptr, nullptr, nullptr, nullptr, ctxReplayer);
    allocService.RegisterAllocator("bbb", 1, nullptr, nullptr, nullptr, nullptr, ctxReplayer);
    // when
    allocService.UnregisterAllocator("aaa");
    allocService.UnregisterAllocator("bbb");
    // then
    IContextReplayer* ret = allocService.GetIContextReplayer("aaa");
    EXPECT_EQ(nullptr, ret);

    delete ctxReplayer;
}

TEST(AllocatorService, UnregisterAllocator_TestFail)
{
    AllocatorService allocService;
    allocService.UnregisterAllocator("abc");
}

TEST(AllocatorService, GetInterfaceFunc_TestSimpleGetter)
{
    // given
    NiceMock<MockIAllocatorWbt>* allWbt = new NiceMock<MockIAllocatorWbt>();
    NiceMock<MockIBlockAllocator>* blkAlloc = new NiceMock<MockIBlockAllocator>();
    NiceMock<MockIWBStripeAllocator>* wbAlloc = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockIContextManager>* ctxMan = new NiceMock<MockIContextManager>();
    NiceMock<MockContextReplayer>* ctxRep = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    allocService.RegisterAllocator("", 0, blkAlloc, wbAlloc, allWbt, ctxMan, ctxRep);
    // when
    allocService.GetIContextReplayer("");
    allocService.GetIContextManager("");
    allocService.GetIWBStripeAllocator("");
    allocService.GetIBlockAllocator("");
    allocService.GetIAllocatorWbt("");

    allocService.GetIContextReplayer(0);
    allocService.GetIContextManager(0);
    allocService.GetIWBStripeAllocator(0);
    allocService.GetIBlockAllocator(0);
    allocService.GetIAllocatorWbt(0);
    delete allWbt;
    delete blkAlloc;
    delete wbAlloc;
    delete ctxMan;
    delete ctxRep;
}

TEST(AllocatorService, GetInterfaceFunc_TestSimpleGetterFail)
{
    // given
    NiceMock<MockIAllocatorWbt>* allWbt = new NiceMock<MockIAllocatorWbt>();
    NiceMock<MockIBlockAllocator>* blkAlloc = new NiceMock<MockIBlockAllocator>();
    NiceMock<MockIWBStripeAllocator>* wbAlloc = new NiceMock<MockIWBStripeAllocator>();
    NiceMock<MockIContextManager>* ctxMan = new NiceMock<MockIContextManager>();
    NiceMock<MockContextReplayer>* ctxRep = new NiceMock<MockContextReplayer>();
    AllocatorService allocService;
    allocService.RegisterAllocator("", 0, blkAlloc, wbAlloc, allWbt, ctxMan, ctxRep);
    // when
    allocService.GetIContextReplayer("a");
    allocService.GetIContextManager("a");
    allocService.GetIWBStripeAllocator("a");
    allocService.GetIBlockAllocator("a");
    allocService.GetIAllocatorWbt("a");

    delete allWbt;
    delete blkAlloc;
    delete wbAlloc;
    delete ctxMan;
    delete ctxRep;
}

} // namespace pos
