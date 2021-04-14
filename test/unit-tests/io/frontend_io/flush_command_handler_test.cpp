#include "src/io/frontend_io/flush_command_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/allocator/i_allocator_ctx.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/io_completer.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/event_framework_api.h"
#include "test/unit-tests/allocator/i_allocator_ctx_mock.h"
#include "test/unit-tests/allocator/i_block_allocator_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/io/frontend_io/flush_command_manager_mock.h"
#include "test/unit-tests/mapper/i_map_flush_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushCmdHandler, FlushCmdHandler_Constructor_WithOneArgument_Stack)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);

    // When: Try to Create New FlushCmdHandler object with 1 argument
    FlushCmdHandler flushCmdHandler(flushIo);

    // Then: Do nothing
}

TEST(FlushCmdHandler, FlushCmdHandler_Constructor_WithOneArgument_Heap)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);

    // When: Try to Create New FlushCmdHandler object with 1 argument
    FlushCmdHandler* flushCmdHandler = new FlushCmdHandler(flushIo);

    // Then: Release memory
    delete flushCmdHandler;
}

TEST(FlushCmdHandler, FlushCmdHandler_Execute_CaseBlockAllocation_BlockAllocatingReturnFalse)
{
    // Given: case FLUSH__BLOCK_ALLOCATION, iBlockAllocator->BlockAllocating(volumeId) == false
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    NiceMock<MockFlushCmdManager> mockFlushCmdManager;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIAllocatorCtx> mockIAllocatorCtx;
    NiceMock<MockIMapFlush> mockIMapFlush;
    FlushCmdHandler flushCmdHandler(flushIo, &mockFlushCmdManager, &mockIBlockAllocator,
        &mockIWBStripeAllocator, &mockIAllocatorCtx, &mockIMapFlush);

    ON_CALL(mockFlushCmdManager, IsFlushEnabled()).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, BlockAllocating(_)).WillByDefault(Return(false));

    bool actual, expected{false};

    // When: Execute
    actual = flushCmdHandler.Execute();

    // Then: return false
    ASSERT_EQ(expected, actual);
}

TEST(FlushCmdHandler, FlushCmdHandler_Execute_CasePendingWriteInProgress_WaitPendingWritesOnStripesReturnFalse)
{
    // Given: case FLUSH__PENDING_WRITE_IN_PROGRESS, iWBStripeAllocator->WaitPendingWritesOnStripes(volumeId) == false
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    NiceMock<MockFlushCmdManager> mockFlushCmdManager;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIAllocatorCtx> mockIAllocatorCtx;
    NiceMock<MockIMapFlush> mockIMapFlush;
    FlushCmdHandler flushCmdHandler(flushIo, &mockFlushCmdManager, &mockIBlockAllocator,
        &mockIWBStripeAllocator, &mockIAllocatorCtx, &mockIMapFlush);

    ON_CALL(mockFlushCmdManager, IsFlushEnabled()).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, BlockAllocating(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, GetAllActiveStripes(_)).WillByDefault(Return());
    ON_CALL(mockIWBStripeAllocator, WaitPendingWritesOnStripes(_)).WillByDefault(Return(false));

    bool actual, expected{false};

    // When: Execute
    actual = flushCmdHandler.Execute();

    // Then: return false
    ASSERT_EQ(expected, actual);
}

TEST(FlushCmdHandler, FlushCmdHandler_Execute_CaseStripeFlushInProgress_WaitStripesFlushCompletionReturnFalse)
{
    // Given : case FLUSH__STRIPE_FLUSH_IN_PROGRESS, iWBStripeAllocator->WaitStripesFlushCompletion(volumeId) == false
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    NiceMock<MockFlushCmdManager> mockFlushCmdManager;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIAllocatorCtx> mockIAllocatorCtx;
    NiceMock<MockIMapFlush> mockIMapFlush;
    FlushCmdHandler flushCmdHandler(flushIo, &mockFlushCmdManager, &mockIBlockAllocator,
        &mockIWBStripeAllocator, &mockIAllocatorCtx, &mockIMapFlush);

    ON_CALL(mockFlushCmdManager, IsFlushEnabled()).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, BlockAllocating(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, GetAllActiveStripes(_)).WillByDefault(Return());
    ON_CALL(mockIWBStripeAllocator, WaitPendingWritesOnStripes(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, WaitStripesFlushCompletion(_)).WillByDefault(Return(false));

    bool actual, expected{false};

    // When: Execute
    actual = flushCmdHandler.Execute();

    // Then: return false
    ASSERT_EQ(expected, actual);
}

TEST(FlushCmdHandler, FlushCmdHandler_Execute_CaseVSAMAP_FlushDirtyMpagesReturnZeroAndCanFlushMetaReturnFalse)
{
    // Given: case FLUSH__VSAMAP, flushCmdManager->CanFlushMeta(flushIo->GetOriginCore(), flushIo) == false
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    NiceMock<MockFlushCmdManager> mockFlushCmdManager;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIAllocatorCtx> mockIAllocatorCtx;
    NiceMock<MockIMapFlush> mockIMapFlush;
    FlushCmdHandler flushCmdHandler(flushIo, &mockFlushCmdManager, &mockIBlockAllocator,
        &mockIWBStripeAllocator, &mockIAllocatorCtx, &mockIMapFlush);

    ON_CALL(mockFlushCmdManager, IsFlushEnabled()).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, BlockAllocating(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, GetAllActiveStripes(_)).WillByDefault(Return());
    ON_CALL(mockIWBStripeAllocator, WaitPendingWritesOnStripes(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, WaitStripesFlushCompletion(_)).WillByDefault(Return(true));
    ON_CALL(mockIMapFlush, FlushDirtyMpages(_, _, _)).WillByDefault(Return(0));
    ON_CALL(mockIBlockAllocator, UnblockAllocating(_)).WillByDefault(Return());
    ON_CALL(mockFlushCmdManager, CanFlushMeta(_, _)).WillByDefault(Return(false));

    bool actual, expected{true};

    // When: Execute
    actual = flushCmdHandler.Execute();

    // Then: return true
    ASSERT_EQ(expected, actual);
}

TEST(FlushCmdHandler, FlushCmdHandler_Execute_CaseMetaFlushInProgress_IsStripeMapAllocatorFlushCompleteReturnFalseAndIsStripeMapFlushCompleteReturnFalse)
{
    // Given: case FLUSH__META_FLUSH_IN_PROGRESS, flushIo->IsStripeMapAllocatorFlushComplete() == false
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    NiceMock<MockFlushCmdManager> mockFlushCmdManager;
    NiceMock<MockIBlockAllocator> mockIBlockAllocator;
    NiceMock<MockIWBStripeAllocator> mockIWBStripeAllocator;
    NiceMock<MockIAllocatorCtx> mockIAllocatorCtx;
    NiceMock<MockIMapFlush> mockIMapFlush;
    FlushCmdHandler flushCmdHandler(flushIo, &mockFlushCmdManager, &mockIBlockAllocator,
        &mockIWBStripeAllocator, &mockIAllocatorCtx, &mockIMapFlush);

    ON_CALL(mockFlushCmdManager, IsFlushEnabled()).WillByDefault(Return(true));
    ON_CALL(mockIBlockAllocator, BlockAllocating(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, GetAllActiveStripes(_)).WillByDefault(Return());
    ON_CALL(mockIWBStripeAllocator, WaitPendingWritesOnStripes(_)).WillByDefault(Return(true));
    ON_CALL(mockIWBStripeAllocator, WaitStripesFlushCompletion(_)).WillByDefault(Return(true));
    ON_CALL(mockIMapFlush, FlushDirtyMpages(_, _, _)).WillByDefault(Return(0));
    ON_CALL(mockIBlockAllocator, UnblockAllocating(_)).WillByDefault(Return());
    ON_CALL(mockFlushCmdManager, CanFlushMeta(_, _)).WillByDefault(Return(true));
    ON_CALL(mockIAllocatorCtx, FlushAllocatorCtxs(_)).WillByDefault(Return(0));

    bool actual, expected{false};

    // When: Execute
    actual = flushCmdHandler.Execute();

    // Then: return false
    ASSERT_EQ(expected, actual);
}

} // namespace pos

namespace pos
{
TEST(MapFlushCompleteEvent, MapFlushCompleteEvent_Constructor_WithTwoArguments_Stack)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    int mapId{0};

    // When: Try to Create New MapFlushCompleteEvent object with 2 arguments
    MapFlushCompleteEvent mapFlushCompleteEvent(mapId, flushIo);

    // Then: Do nothing
}

TEST(MapFlushCompleteEvent, MapFlushCompleteEvent_Constructor_WithTwoArguments_Heap)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    int mapId{0};

    // When: Try to Create New MapFlushCompleteEvent object with 2 arguments
    MapFlushCompleteEvent* mapFlushCompleteEvent = new MapFlushCompleteEvent(mapId, flushIo);

    // Then: Release memory
    delete mapFlushCompleteEvent;
}

TEST(MapFlushCompleteEvent, MapFlushCompleteEvent_Execute_MapIdIsStripeMapId)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    int mapId{STRIPE_MAP_ID};
    MapFlushCompleteEvent mapFlushCompleteEvent(mapId, flushIo);
    bool actual, expected{true};

    // When: Try to call Execute() with mapId = STRIPE_MAP_ID
    actual = mapFlushCompleteEvent.Execute();

    // Then 1: Receive result as true
    ASSERT_EQ(expected, actual);

    // Then 2: IsStripeMapFlushComplete has to return true
    actual = flushIo->IsStripeMapFlushComplete();
    ASSERT_EQ(expected, actual);
}

TEST(MapFlushCompleteEvent, MapFlushCompleteEvent_Execute_MapIdIsNotStripeMapId)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    int mapId{123}; // != STRIPE_MAP_ID
    MapFlushCompleteEvent mapFlushCompleteEvent(mapId, flushIo);
    bool actual, expected{true};

    // When: Try to call Execute() with mapId = STRIPE_MAP_ID
    actual = mapFlushCompleteEvent.Execute();

    // Then 1: Receive result as true
    ASSERT_EQ(expected, actual);

    // Then 2: IsVsaMapFlushComplete has to return true
    actual = flushIo->IsVsaMapFlushComplete();
    ASSERT_EQ(expected, actual);
}

} // namespace pos

namespace pos
{
TEST(AllocatorFlushDoneEvent, AllocatorFlushDoneEvent_Constructor_WithOneArgument_Stack)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);

    // When: Try to Create New AllocatorFlushDoneEvent object with 1 argument
    AllocatorFlushDoneEvent allocatorFlushDoneEvent(flushIo);

    // Then: Do nothing
}

TEST(AllocatorFlushDoneEvent, AllocatorFlushDoneEvent_Constructor_WithOneArgument_Heap)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);

    // When: Try to Create New AllocatorFlushDoneEvent object with 1 argument
    AllocatorFlushDoneEvent* allocatorFlushDoneEvent = new AllocatorFlushDoneEvent(flushIo);

    // Then: Release memory
    delete allocatorFlushDoneEvent;
}

TEST(AllocatorFlushDoneEvent, AllocatorFlushDoneEvent_Execute_NormalCase)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);
    AllocatorFlushDoneEvent allocatorFlushDoneEvent(flushIo);
    bool actual, expected{true};

    // When: Try to call Execute()
    actual = allocatorFlushDoneEvent.Execute();

    // Then: Receive result as true
    ASSERT_EQ(expected, actual);
}

} // namespace pos
