#include "src/journal_manager/log_write/volume_deleted_log_write_callback.h"
#include "test/unit-tests/journal_manager/log_write/journal_volume_event_handler_mock.h"
#include <gtest/gtest.h>

using ::testing::NiceMock;

namespace pos
{
TEST(VolumeDeletedLogWriteCallback, Execute_testIfExecutionSuccess)
{
    // Given: volumeEventHandler and volume id is given
    NiceMock<MockJournalVolumeEventHandler> volumeEventHandler;
    int volumeId = 1;

    // Then: Volume event handler should be called
    EXPECT_CALL(volumeEventHandler, VolumeDeletedLogWriteDone(volumeId));

    // When
    VolumeDeletedLogWriteCallback callback(&volumeEventHandler, volumeId);
    bool actual = callback.Execute();

    EXPECT_TRUE(actual == true);
}

} // namespace pos
