#include "src/io/frontend_io/flush_command_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/bio/flush_io_mock.h"

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
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);

    // When : CanFlushMeta is called first time
    bool actual = flushCmdManager.CanFlushMeta(0, flushIo);

    // Then : Check if return true
    EXPECT_EQ(actual, true);

    // When : CanFlushMeta is called again
    actual = flushCmdManager.CanFlushMeta(0, flushIo);

    // Then : Check if return false
    EXPECT_EQ(actual, false);
}

TEST(FlushCmdManager, FlushCmdManager_FinishMetaFlush_CheckWithCanFlushMeta)
{
    // Given
    std::string arr_name = "arr_name";
    FlushIoSmartPtr flushIo = std::make_shared<FlushIo>(arr_name);

    // When : FinishMetaFlush is called where flushEvents are empty
    FlushCmdManager flushCmdManager;
    flushCmdManager.FinishMetaFlush();

    // Then : CanFlushMeta should return true
    bool actual = flushCmdManager.CanFlushMeta(0, flushIo);
    ASSERT_EQ(actual, true);

    // When : FinishMetaFlush is called where flushEvents exist
    actual = flushCmdManager.CanFlushMeta(0, flushIo);
    ASSERT_EQ(actual, false);

    flushCmdManager.FinishMetaFlush();

    // Then : CanFlushMeta should return false
    actual = flushCmdManager.CanFlushMeta(0, flushIo);
    EXPECT_EQ(actual, false);
}

} // namespace pos
