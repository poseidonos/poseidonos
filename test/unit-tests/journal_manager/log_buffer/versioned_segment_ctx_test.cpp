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

#include "src/allocator/context_manager/segment_ctx/segment_info.h"
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

    SegmentInfo* segmentInfos = new SegmentInfo[3]();
    versionedSegCtx.Init(&config, segmentInfos, 3);

    delete[] segmentInfos;
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
    SegmentInfo* segmentInfos = new SegmentInfo[3]();
    versionedSegCtx.Init(&config, segmentInfos, 3);
    versionedSegCtx.Dispose();

    delete[] segmentInfos;
}

TEST(VersionedSegmentCtx, IncreaseValidBlockCount_testIfValidBlockCountIsIncreasedAndDecreased)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    SegmentInfo* segmentInfos = new SegmentInfo[3]();

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, segmentInfos, 3, versionedSegmentInfo);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseValidBlockCount(2, 3)).Times(1);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), DecreaseValidBlockCount(2, 1)).Times(1);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseValidBlockCount(1, 4)).Times(1);

    versionedSegCtx.IncreaseValidBlockCount(targetLogGroup, 2, 3);
    versionedSegCtx.DecreaseValidBlockCount(targetLogGroup, 2, 1);
    versionedSegCtx.IncreaseValidBlockCount(targetLogGroup, 1, 4);

    delete[] segmentInfos;
}

TEST(VersionedSegmentCtx, IncreaseOccupiedStripeCount_testIfOccupiedStripeCountIsIncreased)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    SegmentInfo* segmentInfos = new SegmentInfo[3]();

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, segmentInfos, 3, versionedSegmentInfo);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseOccupiedStripeCount(2)).Times(1);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseOccupiedStripeCount(1)).Times(1);

    versionedSegCtx.IncreaseOccupiedStripeCount(targetLogGroup, 2);
    versionedSegCtx.IncreaseOccupiedStripeCount(targetLogGroup, 1);

    delete[] segmentInfos;
}

TEST(VersionedSegmentCtx, GetUpdatedInfoToFlush_testIfChangedValueIsReturned)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    int numSegments = 3;
    SegmentInfo* segmentInfos = new SegmentInfo[numSegments]();

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, segmentInfos, 3, versionedSegmentInfo);

    // When
    int targetLogGroup = 0;

    tbb::concurrent_unordered_map<SegmentId, int> changedValidBlkCount;
    changedValidBlkCount.emplace(0, 10);
    changedValidBlkCount.emplace(1, 2);
    changedValidBlkCount.emplace(2, 40);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedValidBlockCount).WillOnce(Return(changedValidBlkCount));

    tbb::concurrent_unordered_map<SegmentId, uint32_t> changedOccupiedStripeCount;
    changedOccupiedStripeCount.emplace(0, 1);
    changedOccupiedStripeCount.emplace(1, 1);
    changedOccupiedStripeCount.emplace(2, 0);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedOccupiedStripeCount).WillOnce(Return(changedOccupiedStripeCount));

    SegmentInfo* result = versionedSegCtx.GetUpdatedInfoToFlush(targetLogGroup);

    // Then
    EXPECT_EQ(result[0].GetValidBlockCount(), 10);
    EXPECT_EQ(result[1].GetValidBlockCount(), 2);
    EXPECT_EQ(result[2].GetValidBlockCount(), 40);

    EXPECT_EQ(result[0].GetOccupiedStripeCount(), 1);
    EXPECT_EQ(result[1].GetOccupiedStripeCount(), 1);
    EXPECT_EQ(result[2].GetOccupiedStripeCount(), 0);

    delete[] segmentInfos;
}

TEST(VersionedSegmentCtx, GetUpdatedInfoToFlush_testIfChangedValueIsApplied)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    int numSegments = 3;
    SegmentInfo* segmentInfos = new SegmentInfo[numSegments]();
    segmentInfos[0].SetValidBlockCount(10);
    segmentInfos[1].SetValidBlockCount(9);
    segmentInfos[2].SetValidBlockCount(1);

    segmentInfos[0].SetOccupiedStripeCount(100);
    segmentInfos[1].SetOccupiedStripeCount(12);
    segmentInfos[2].SetOccupiedStripeCount(0);

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, segmentInfos, 3, versionedSegmentInfo);

    // When
    int targetLogGroup = 0;
    tbb::concurrent_unordered_map<SegmentId, int> changedValidBlkCount;
    changedValidBlkCount.emplace(0, -5);
    changedValidBlkCount.emplace(1, 2);
    changedValidBlkCount.emplace(2, 40);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), GetChangedValidBlockCount).WillOnce(Return(changedValidBlkCount));

    tbb::concurrent_unordered_map<SegmentId, uint32_t> changedOccupiedStripeCount;
    changedOccupiedStripeCount.emplace(0, 2);
    changedOccupiedStripeCount.emplace(1, 1);
    changedOccupiedStripeCount.emplace(2, 0);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), GetChangedOccupiedStripeCount).WillOnce(Return(changedOccupiedStripeCount));

    SegmentInfo* result = versionedSegCtx.GetUpdatedInfoToFlush(targetLogGroup);

    // Then
    EXPECT_EQ(result[0].GetValidBlockCount(), 10 - 5);
    EXPECT_EQ(result[1].GetValidBlockCount(), 9 + 2);
    EXPECT_EQ(result[2].GetValidBlockCount(), 1 + 40);

    EXPECT_EQ(result[0].GetOccupiedStripeCount(), 100 + 2);
    EXPECT_EQ(result[1].GetOccupiedStripeCount(), 12 + 1);
    EXPECT_EQ(result[2].GetOccupiedStripeCount(), 0 + 0);

    delete[] segmentInfos;
}

TEST(VersionedSegmentCtx, GetUpdatedInfoToFlush_testIfFailsWhenInvalidLogGroupIdIsProvided)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    int numSegments = 3;
    SegmentInfo* segmentInfos = new SegmentInfo[numSegments]();

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, segmentInfos, 3, versionedSegmentInfo);

    // When
    ASSERT_DEATH(versionedSegCtx.GetUpdatedInfoToFlush(5), "");

    delete[] segmentInfos;
}

TEST(VersionedSegmentCtx, ResetFlushedInfo_testIfInfoIsResetted)
{
    // Given
    VersionedSegmentCtx versionedSegCtx;
    int numLogGroups = 2;
    NiceMock<MockJournalConfiguration> config;
    ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

    int numSegments = 3;
    SegmentInfo* segmentInfos = new SegmentInfo[numSegments]();

    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, segmentInfos, 3, versionedSegmentInfo);

    int targetLogGroup = 0;
    tbb::concurrent_unordered_map<SegmentId, int> changedValidBlkCount;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedValidBlockCount).WillOnce(Return(changedValidBlkCount));

    tbb::concurrent_unordered_map<SegmentId, uint32_t> changedOccupiedStripeCount;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedOccupiedStripeCount).WillOnce(Return(changedOccupiedStripeCount));

    SegmentInfo* result = versionedSegCtx.GetUpdatedInfoToFlush(targetLogGroup);

    // When
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), Reset).Times(1);
    versionedSegCtx.ResetFlushedInfo(targetLogGroup);

    delete[] segmentInfos;
}

} // namespace pos
