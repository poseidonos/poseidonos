#include "src/array_components/meta_mount_sequence.h"

#include <gtest/gtest.h>

#include "test/unit-tests/array_models/interface/i_mount_sequence_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(MetaMountSequence, MetaMountSequence_testContructor)
{
    // Given: nothing

    // When
    MetaMountSequence mMntSeq("mock-array", nullptr, nullptr, nullptr);

    // Then
}

// Disabled to avoid "segfault" at metafs_file_intf.cpp:161.
// TODO(yyu): follow up with munseop.lim
TEST(MetaMountSequence, DISABLED_Init_testIfEverySequenceIsInitialized)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    MetaMountSequence mMntSeq("mock-array", &mockSeq1, &mockSeq2, &mockSeq3);

    EXPECT_CALL(mockSeq1, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq2, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq3, Init).WillOnce(Return(0));

    // When
    int actual = mMntSeq.Init();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(MetaMountSequence, Init_testIfMapperIsRolledBack)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    MetaMountSequence mMntSeq("mock-array", &mockSeq1, &mockSeq2, &mockSeq3);

    int MAPPER_FAILURE = 123;
    EXPECT_CALL(mockSeq1, Init).WillOnce(Return(MAPPER_FAILURE));
    EXPECT_CALL(mockSeq1, Dispose).Times(1);

    // When
    int actual = mMntSeq.Init();

    // Then
    ASSERT_EQ(MAPPER_FAILURE, actual);
}

TEST(MetaMountSequence, Init_testIfMapperAndAllocatorAreRolledBack)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    MetaMountSequence mMntSeq("mock-array", &mockSeq1, &mockSeq2, &mockSeq3);

    int ALLOCATOR_FAILURE = 456;
    EXPECT_CALL(mockSeq1, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq2, Init).WillOnce(Return(ALLOCATOR_FAILURE));
    EXPECT_CALL(mockSeq2, Dispose).Times(1);
    EXPECT_CALL(mockSeq1, Dispose).Times(1);

    // When
    int actual = mMntSeq.Init();

    // Then
    ASSERT_EQ(ALLOCATOR_FAILURE, actual);
}

TEST(MetaMountSequence, Init_testIfMapperAndAllocatorAndJournalAreRolledBack)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    MetaMountSequence mMntSeq("mock-array", &mockSeq1, &mockSeq2, &mockSeq3);

    int JOURNAL_FAILURE = 456;
    EXPECT_CALL(mockSeq1, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq2, Init).WillOnce(Return(0));
    EXPECT_CALL(mockSeq3, Init).WillOnce(Return(JOURNAL_FAILURE));
    EXPECT_CALL(mockSeq3, Dispose).Times(1);
    EXPECT_CALL(mockSeq2, Dispose).Times(1);
    EXPECT_CALL(mockSeq1, Dispose).Times(1);

    // When
    int actual = mMntSeq.Init();

    // Then
    ASSERT_EQ(JOURNAL_FAILURE, actual);
}

TEST(MetaMountSequence, Dispose_testIfAllSequenceInvokeDispose)
{
    // Given
    MockIMountSequence mockSeq1, mockSeq2, mockSeq3;
    MetaMountSequence mMntSeq("mock-array", &mockSeq1, &mockSeq2, &mockSeq3);

    EXPECT_CALL(mockSeq1, Dispose).Times(1);
    EXPECT_CALL(mockSeq2, Dispose).Times(1);
    EXPECT_CALL(mockSeq3, Dispose).Times(1);

    // When
    mMntSeq.Dispose();

    // Then
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
