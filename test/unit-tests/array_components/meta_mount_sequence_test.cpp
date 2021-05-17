#include "src/array_components/meta_mount_sequence.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array_models/interface/i_mount_sequence_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(MetaMountSequence, MetaMountSequence_)
{
}

TEST(MetaMountSequence, Init_)
{
}

TEST(MetaMountSequence, Dispose_testIfAllComponentsAreDisposed)
{
    // Given: Array name and 3 mount sequence interfaces are given
    std::string arrayName = "POSArray";

    NiceMock<MockIMountSequence> mapper;
    NiceMock<MockIMountSequence> allocator;
    NiceMock<MockIMountSequence> journal;

    MetaMountSequence mountSequence(arrayName, &mapper, &allocator, &journal);

    // Then: All three should be disposed
    EXPECT_CALL(mapper, Dispose);
    EXPECT_CALL(allocator, Dispose);
    EXPECT_CALL(journal, Dispose);

    // When: MetaMountSequence is disposed
    mountSequence.Dispose();
}

TEST(MetaMountSequence, Shutdown_testIfAllComponentsAreDisposed)
{
    // Given: Array name and 3 mount sequence interfaces are given
    std::string arrayName = "POSArray";

    NiceMock<MockIMountSequence> mapper;
    NiceMock<MockIMountSequence> allocator;
    NiceMock<MockIMountSequence> journal;

    MetaMountSequence mountSequence(arrayName, &mapper, &allocator, &journal);

    // Then: All three should be shut down
    EXPECT_CALL(mapper, Shutdown);
    EXPECT_CALL(allocator, Shutdown);
    EXPECT_CALL(journal, Shutdown);

    // When: MetaMountSequence is shut down
    mountSequence.Shutdown();
}
} // namespace pos
