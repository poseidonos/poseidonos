#include "src/io/backend_io/rebuild_io/rebuild_read.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/bio/ubio_mock.h"
#include "src/include/pos_event_id.h"

using ::testing::Return;
namespace pos
{
TEST(RebuildRead, RebuildRead_stack)
{
    // Given

    // When : Create RebuildRead object on stack
    RebuildRead rebuildRead;

    // Then :
}

TEST(RebuildRead, RebuildRead_Recover_IsRetry)
{
    // Given
    auto ubio = std::make_shared<MockUbio>(nullptr, 0, 0);

    // When : Set retry
    ON_CALL(*ubio.get(), IsRetry()).WillByDefault(Return(true));
    EXPECT_CALL(*ubio.get(), IsRetry()).Times(1);
    RebuildRead rebuildRead;
    auto actual = rebuildRead.Recover(ubio, nullptr);

    // Then : Check return value
    EXPECT_EQ(actual, EID(IO_RECOVER_DEBUG_MSG));
}

} // namespace pos
