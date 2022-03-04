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

#include "src/journal_manager/log_buffer/versioned_segment_ctx.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/journal_manager/log_buffer/versioned_segment_info.h"
#include "test/unit-tests/journal_manager/config/journal_configuration_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/versioned_segment_info_mock.h"

using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(VersionedSegmentCtx, Init_testIfInitWhenNumberOfLogGroupsIsTwo)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;

    // When, Then
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    versionedSegCtx.Init(&config);
}

TEST(VersionedSegmentCtx, Dispose_testIfContextIsDeleted)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;

    // When
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    // When : Dispose without Init
    versionedSegCtx.Dispose();

    // When : Init and Dispose
    versionedSegCtx.Init(&config);
    versionedSegCtx.Dispose();
}

TEST(VersionedSegmentCtx, GetSegmentContext_testIfExecutedWithCorrectLogGroupId)
{
    // Given
    NiceMock<MockJournalConfiguration> config;
    VersionedSegmentCtx versionedSegCtx;

    // When
    int numLogGroups = 2;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    VersionedSegmentInfo** versionedSegmentInfo;
    versionedSegmentInfo = new VersionedSegmentInfo*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        versionedSegmentInfo[index] = new NiceMock<MockVersionedSegmentInfo>;
    }
    versionedSegCtx.Init(&config, versionedSegmentInfo);

    // Then
    uint32_t targetLogGroup = 0;
    EXPECT_EQ(versionedSegmentInfo[targetLogGroup], versionedSegCtx.GetSegmentInfo(targetLogGroup));
}

TEST(VersionedSegmentCtx, GetSegmentContext_testIfExecutedWithIncorrectLogGroupId)
{
    // Given
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));
    VersionedSegmentCtx versionedSegCtx;
    versionedSegCtx.Init(&config);

    // When, Then
    uint32_t targetLogGroup = 5;
    EXPECT_TRUE(versionedSegCtx.GetSegmentInfo(targetLogGroup) == nullptr);
}

TEST(VersionedSegmentCtx, UpdateSegmentContext_testIfOnlyOneOfTwoLogGroupsUpdated)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;

    // When
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    VersionedSegmentInfo** versionedSegmentInfo;
    versionedSegmentInfo = new VersionedSegmentInfo*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        versionedSegmentInfo[index] = new NiceMock<MockVersionedSegmentInfo>;
    }
    versionedSegCtx.Init(&config, versionedSegmentInfo);

    std::unordered_map<uint32_t, int> expectValidCount;
    expectValidCount[1] = 2;
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[0]), GetChangedValidBlockCount).WillOnce(Return(expectValidCount));

    std::unordered_map<uint32_t, uint32_t> expectOccupiedCount;
    expectOccupiedCount[3] = 1;
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[0]), GetChangedOccupiedStripeCount).WillOnce(Return(expectOccupiedCount));

    // Then
    uint32_t targetLogGroup = 0;
    versionedSegCtx.UpdateSegmentContext(targetLogGroup);

    // TODO (cheolho.kang): Get latest segment context
    // TODO (cheolho.kang): Compare the expected segment context with returned segment context
}

TEST(VersionedSegmentCtx, IncreaseValidBlockCount_testIfValidBlockCountIsIncreasedAndDecreased)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    VersionedSegmentInfo** versionedSegmentInfo;
    versionedSegmentInfo = new VersionedSegmentInfo*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        versionedSegmentInfo[index] = new NiceMock<MockVersionedSegmentInfo>;
    }
    versionedSegCtx.Init(&config, versionedSegmentInfo);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[targetLogGroup]), IncreaseValidBlockCount(2, 3)).Times(1);
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[targetLogGroup]), DecreaseValidBlockCount(2, 1)).Times(1);
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[targetLogGroup]), IncreaseValidBlockCount(1, 4)).Times(1);

    versionedSegCtx.IncreaseValidBlockCount(targetLogGroup, 2, 3);
    versionedSegCtx.DecreaseValidBlockCount(targetLogGroup, 2, 1);
    versionedSegCtx.IncreaseValidBlockCount(targetLogGroup, 1, 4);
}

TEST(VersionedSegmentCtx, IncreaseOccupiedStripeCount_testIfOccupiedStripeCountIsIncreased)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    VersionedSegmentInfo** versionedSegmentInfo;
    versionedSegmentInfo = new VersionedSegmentInfo*[numLogGroups];
    for (int index = 0; index < numLogGroups; index++)
    {
        versionedSegmentInfo[index] = new NiceMock<MockVersionedSegmentInfo>;
    }
    versionedSegCtx.Init(&config, versionedSegmentInfo);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[targetLogGroup]), IncreaseOccupiedStripeCount(2)).Times(1);
    EXPECT_CALL(*static_cast<MockVersionedSegmentInfo*>(versionedSegmentInfo[targetLogGroup]), IncreaseOccupiedStripeCount(1)).Times(1);

    versionedSegCtx.IncreaseOccupiedStripeCount(targetLogGroup, 2);
    versionedSegCtx.IncreaseOccupiedStripeCount(targetLogGroup, 1);
}
} // namespace pos
