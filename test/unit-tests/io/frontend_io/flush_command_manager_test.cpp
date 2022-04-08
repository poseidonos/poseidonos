#include "src/io/frontend_io/flush_command_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/bio/flush_io_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(FlushCmdManager, FlushCmdManager_Constructor_Heap)
{
    // Given

    // When : Create FlushCmdManager with argument
    FlushCmdManager* flushCmdManager = new FlushCmdManager();
    delete flushCmdManager;

    // Then : Do nothing
}

TEST(FlushCmdManager, FlushCmdManager_Constructor_Stack)
{
    // Given

    // When : Create FlushCmdManager with argument
    FlushCmdManager flushCmdManager;

    // Then : Do nothing
}

TEST(FlushCmdManager, FlushCmdManager_IsFlushEnabled_Normal)
{
    // Given

    // When : IsFlushEnabled is called
    FlushCmdManager flushCmdManager;
    flushCmdManager.IsFlushEnabled();

    // Then : Compare return value
}

TEST(FlushCmdManager, FlushCmdManager_CanFlushMeta_CheckTrueFalse)
{
    // Given
    FlushCmdManager flushCmdManager;

    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(0);

    // When : CanFlushMeta is called first time
    bool actual = flushCmdManager.CanFlushMeta(flushIo);

    // Then : Check if return true
    EXPECT_EQ(actual, true);

    // When : CanFlushMeta is called again
    actual = flushCmdManager.CanFlushMeta(flushIo);

    // Then : Check if return false
    EXPECT_EQ(actual, false);
}

TEST(FlushCmdManager, FlushCmdManager_FinishMetaFlush_CheckWithCanFlushMeta)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(0);

    // When : FinishMetaFlush is called where flushEvents are empty
    FlushCmdManager flushCmdManager;
    flushCmdManager.FinishMetaFlush();

    // Then : CanFlushMeta should return true
    bool actual = flushCmdManager.CanFlushMeta(flushIo);
    ASSERT_EQ(actual, true);

    // When : FinishMetaFlush is called where flushEvents exist
    actual = flushCmdManager.CanFlushMeta(flushIo);
    ASSERT_EQ(actual, false);

    flushCmdManager.FinishMetaFlush();

    // Then : CanFlushMeta should return false
    actual = flushCmdManager.CanFlushMeta(flushIo);
    EXPECT_EQ(actual, false);
}

TEST(FlushCmdManager, FlushCmdManager_UpdateVSANewEntries_Normal)
{
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    FlushCmdManager flushCmdManager(&mockEventScheduler);
    flushCmdManager.UpdateVSANewEntries(0, 0);
}

TEST(FlushCmdManager, FlushCmdManager_IsInternalFlushEnabled_Normal)
{
    FlushCmdManager flushCmdManager;
    flushCmdManager.IsInternalFlushEnabled();
}

TEST(FlushCmdManager, FlushCmdManager_GetInternalFlushThreshold_Normal)
{
    FlushCmdManager flushCmdManager;
    flushCmdManager.GetInternalFlushThreshold();
}

TEST(FlushCmdManager, FlushCmdManager_TrySetFlushInProgress_Normal)
{
    FlushCmdManager flushCmdManager;
    flushCmdManager.TrySetFlushInProgress(0);
}

TEST(FlushCmdManager, FlushCmdManager_ResetFlushInProgress_Normal)
{
    FlushCmdManager flushCmdManager;
    flushCmdManager.ResetFlushInProgress(0, true);
}

} // namespace pos
