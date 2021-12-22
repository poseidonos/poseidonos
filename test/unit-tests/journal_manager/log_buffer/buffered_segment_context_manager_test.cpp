/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/journal_manager/log_buffer/buffered_segment_context_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/journal_manager/log_buffer/buffered_segment_context.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffered_segment_context_mock.h"

using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(BufferedSegmentContextManager, Init_testIfInitWhenNumberOfLogGroupsIsTwo)
{
    // Given
    BufferedSegmentContextManager bufferedSegCtxManager;

    // When, Then
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    bufferedSegCtxManager.Init(&config);
}

TEST(BufferedSegmentContextManager, Dispose_testIfContextIsDeleted)
{
    // Given
    BufferedSegmentContextManager bufferedSegCtxManager;

    // When
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    // When : Dispose without Init
    bufferedSegCtxManager.Dispose();

    // When : Init and Dispose
    bufferedSegCtxManager.Init(&config);
    bufferedSegCtxManager.Dispose();
}

TEST(BufferedSegmentContextManager, GetSegmentContext_testIfExecutedWithCorrectLogGroupId)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    BufferedSegmentContextManager bufferedSegCtxManager;

    // When
    int numLogGroups = 2;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    BufferedSegmentContext** bufferedSegmentContext;
    bufferedSegmentContext = new BufferedSegmentContext*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        bufferedSegmentContext[index] = new NiceMock<MockBufferedSegmentContext>;
    }
    bufferedSegCtxManager.Init(&config, bufferedSegmentContext);

    // Then
    uint32_t targetLogGroup = 0;
    EXPECT_EQ(bufferedSegmentContext[targetLogGroup], bufferedSegCtxManager.GetSegmentContext(targetLogGroup));
}

TEST(BufferedSegmentContextManager, GetSegmentContext_testIfExecutedWithIncorrectLogGroupId)
{
    // Given
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    BufferedSegmentContextManager bufferedSegCtxManager;
    bufferedSegCtxManager.Init(&config);

    // When, Then
    uint32_t targetLogGroup = 5;
    EXPECT_TRUE(bufferedSegCtxManager.GetSegmentContext(targetLogGroup) == nullptr);
}

TEST(BufferedSegmentContextManager, UpdateSegmentContext_testIfOnlyOneOfTwoLogGroupsUpdated)
{
    // Given
    BufferedSegmentContextManager bufferedSegCtxManager;

    // When
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    BufferedSegmentContext** bufferedSegmentContext;
    bufferedSegmentContext = new BufferedSegmentContext*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        bufferedSegmentContext[index] = new NiceMock<MockBufferedSegmentContext>;
    }
    bufferedSegCtxManager.Init(&config, bufferedSegmentContext);

    std::unordered_map<uint32_t, int> expectValidCount;
    expectValidCount[1] = 2;
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[0]), GetChangedValidBlockCount).WillOnce(Return(expectValidCount));

    std::unordered_map<uint32_t, uint32_t> expectOccupiedCount;
    expectOccupiedCount[3] = 1;
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[0]), GetChangedOccupiedStripeCount).WillOnce(Return(expectOccupiedCount));

    // Then
    uint32_t targetLogGroup = 0;
    bufferedSegCtxManager.UpdateSegmentContext(targetLogGroup);

    // TODO (cheolho.kang): Get latest segment context
    // TODO (cheolho.kang): Compare the expected segment context with returned segment context
}

TEST(BufferedSegmentContextManager, IncreaseValidBlockCount_testIfValidBlockCountIsIncreasedAndDecreased)
{
    // Given
    BufferedSegmentContextManager bufferedSegCtxManager;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    BufferedSegmentContext** bufferedSegmentContext;
    bufferedSegmentContext = new BufferedSegmentContext*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        bufferedSegmentContext[index] = new NiceMock<MockBufferedSegmentContext>;
    }
    bufferedSegCtxManager.Init(&config, bufferedSegmentContext);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[targetLogGroup]), IncreaseValidBlockCount(2, 3)).Times(1);
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[targetLogGroup]), DecreaseValidBlockCount(2, 1)).Times(1);
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[targetLogGroup]), IncreaseValidBlockCount(1, 4)).Times(1);

    bufferedSegCtxManager.IncreaseValidBlockCount(targetLogGroup, 2, 3);
    bufferedSegCtxManager.DecreaseValidBlockCount(targetLogGroup, 2, 1);
    bufferedSegCtxManager.IncreaseValidBlockCount(targetLogGroup, 1, 4);
}

TEST(BufferedSegmentContextManager, IncreaseOccupiedStripeCount_testIfOccupiedStripeCountIsIncreased)
{
    // Given
    BufferedSegmentContextManager bufferedSegCtxManager;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    BufferedSegmentContext** bufferedSegmentContext;
    bufferedSegmentContext = new BufferedSegmentContext*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        bufferedSegmentContext[index] = new NiceMock<MockBufferedSegmentContext>;
    }
    bufferedSegCtxManager.Init(&config, bufferedSegmentContext);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[targetLogGroup]), IncreaseOccupiedStripeCount(2)).Times(1);
    EXPECT_CALL(*static_cast<MockBufferedSegmentContext*>(bufferedSegmentContext[targetLogGroup]), IncreaseOccupiedStripeCount(1)).Times(1);

    bufferedSegCtxManager.IncreaseOccupiedStripeCount(targetLogGroup, 2);
    bufferedSegCtxManager.IncreaseOccupiedStripeCount(targetLogGroup, 1);
}
} // namespace pos
