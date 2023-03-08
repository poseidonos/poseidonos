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
using testing::ReturnRef;

namespace pos
{
class VersionedSegmentCtxTestFixture : public testing::Test
{
public:
    VersionedSegmentCtxTestFixture(void) {};
    virtual ~VersionedSegmentCtxTestFixture(void) {};

    virtual void SetUp(void)
    {
        numLogGroups = 2;
        ON_CALL(config, GetNumLogGroups).WillByDefault(Return(numLogGroups));

        int numSegInfos = 3;
        segmentInfoData = new SegmentInfoData[numSegInfos];
        for (int i = 0; i < numSegInfos; ++i)
        {
            segmentInfoData[i].Set(0, 0, SegmentState::FREE);
        }
    }
    virtual void TearDown(void)
    {
        delete[] segmentInfoData;
    }

protected:
    SegmentInfoData* segmentInfoData;
    VersionedSegmentCtx versionedSegCtx;
    NiceMock<MockJournalConfiguration> config;
    int numLogGroups;
};

TEST_F(VersionedSegmentCtxTestFixture, Init_testIfInitWhenNumberOfLogGroupsIsTwo)
{
    // When
    versionedSegCtx.Init(&config, 3, 1024 /* not interested */);
}

TEST_F(VersionedSegmentCtxTestFixture, Dispose_testIfContextIsDeleted)
{
    // When : Dispose without Init
    versionedSegCtx.Dispose();

    // When : Init and Dispose
    versionedSegCtx.Init(&config, 3, 1024 /*not interested*/);
    versionedSegCtx.Dispose();
}

TEST_F(VersionedSegmentCtxTestFixture, IncreaseValidBlockCount_testIfValidBlockCountIsIncreasedAndDecreased)
{
    // Given
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseValidBlockCount(2, 3)).Times(1);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), DecreaseValidBlockCount(2, 1)).Times(1);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseValidBlockCount(1, 4)).Times(1);

    versionedSegCtx.IncreaseValidBlockCount(targetLogGroup, 2, 3);
    versionedSegCtx.DecreaseValidBlockCount(targetLogGroup, 2, 1);
    versionedSegCtx.IncreaseValidBlockCount(targetLogGroup, 1, 4);
}

TEST_F(VersionedSegmentCtxTestFixture, IncreaseOccupiedStripeCount_testIfOccupiedStripeCountIsIncreased)
{
    // Given
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);

    // When, Then
    uint32_t targetLogGroup = 0;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseOccupiedStripeCount(2)).Times(1);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), IncreaseOccupiedStripeCount(1)).Times(1);

    versionedSegCtx.IncreaseOccupiedStripeCount(targetLogGroup, 2);
    versionedSegCtx.IncreaseOccupiedStripeCount(targetLogGroup, 1);
}

TEST_F(VersionedSegmentCtxTestFixture, GetUpdatedInfoToFlush_testIfChangedValueIsReturned)
{
    // Given
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);

    // When
    int targetLogGroup = 0;

    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedValidBlkCount;
    changedValidBlkCount.emplace(0, 10);
    changedValidBlkCount.emplace(1, 2);
    changedValidBlkCount.emplace(2, 40);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedValidBlockCount).WillOnce(ReturnRef(changedValidBlkCount));

    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<uint32_t>> changedOccupiedStripeCount;
    changedOccupiedStripeCount.emplace(0, 1);
    changedOccupiedStripeCount.emplace(1, 1);
    changedOccupiedStripeCount.emplace(2, 0);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedOccupiedStripeCount).WillOnce(ReturnRef(changedOccupiedStripeCount));

    SegmentInfoData* result = versionedSegCtx.GetUpdatedInfoDataToFlush(targetLogGroup);

    // Then
    EXPECT_EQ(result[0].validBlockCount, 10);
    EXPECT_EQ(result[1].validBlockCount, 2);
    EXPECT_EQ(result[2].validBlockCount, 40);

    EXPECT_EQ(result[0].occupiedStripeCount, 1);
    EXPECT_EQ(result[1].occupiedStripeCount, 1);
    EXPECT_EQ(result[2].occupiedStripeCount, 0);
}

TEST_F(VersionedSegmentCtxTestFixture, GetUpdatedInfoToFlush_testIfChangedValueIsApplied)
{
    // Given
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);

    segmentInfoData[0].validBlockCount = 10;
    segmentInfoData[1].validBlockCount = 9; 
    segmentInfoData[2].validBlockCount = 1;

    segmentInfoData[0].occupiedStripeCount = 100; 
    segmentInfoData[1].occupiedStripeCount = 12;
    segmentInfoData[2].occupiedStripeCount = 0;

    versionedSegCtx.Load(segmentInfoData);

    // When
    int targetLogGroup = 0;
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedValidBlkCount;
    changedValidBlkCount.emplace(0, -5);
    changedValidBlkCount.emplace(1, 2);
    changedValidBlkCount.emplace(2, 40);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), GetChangedValidBlockCount).WillOnce(ReturnRef(changedValidBlkCount));

    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<uint32_t>> changedOccupiedStripeCount;
    changedOccupiedStripeCount.emplace(0, 2);
    changedOccupiedStripeCount.emplace(1, 1);
    changedOccupiedStripeCount.emplace(2, 0);
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), GetChangedOccupiedStripeCount).WillOnce(ReturnRef(changedOccupiedStripeCount));

    SegmentInfoData* result = versionedSegCtx.GetUpdatedInfoDataToFlush(targetLogGroup);

    // Then
    EXPECT_EQ(result[0].validBlockCount, 10 - 5);
    EXPECT_EQ(result[1].validBlockCount, 9 + 2);
    EXPECT_EQ(result[2].validBlockCount, 1 + 40);

    EXPECT_EQ(result[0].occupiedStripeCount, 100 + 2);
    EXPECT_EQ(result[1].occupiedStripeCount, 12 + 1);
    EXPECT_EQ(result[2].occupiedStripeCount, 0 + 0);
}

TEST_F(VersionedSegmentCtxTestFixture, GetUpdatedInfoToFlush_testIfFailsWhenInvalidLogGroupIdIsProvided)
{
    // Given
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);

    // When
    ASSERT_DEATH(versionedSegCtx.GetUpdatedInfoDataToFlush(5), "");
}

TEST_F(VersionedSegmentCtxTestFixture, LogBufferReseted_testIfInfoIsResetted)
{
    // Given
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);

    int targetLogGroup = 0;
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedValidBlkCount;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedValidBlockCount).WillOnce(ReturnRef(changedValidBlkCount));

    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<uint32_t>> changedOccupiedStripeCount;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedOccupiedStripeCount).WillOnce(ReturnRef(changedOccupiedStripeCount));

    SegmentInfoData* result = versionedSegCtx.GetUpdatedInfoDataToFlush(targetLogGroup);

    // When
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]), Reset).Times(1);
    versionedSegCtx.LogBufferReseted(targetLogGroup);
}

TEST_F(VersionedSegmentCtxTestFixture, NotifySegmentFreed_testIfSegmentFreed)
{
    // Make precondition
    std::vector<std::shared_ptr<VersionedSegmentInfo>> versionedSegmentInfo;
    for (int index = 0; index < numLogGroups; index++)
    {
        std::shared_ptr<VersionedSegmentInfo> input(new NiceMock<MockVersionedSegmentInfo>);
        versionedSegmentInfo.push_back(input);
    }
    versionedSegCtx.Init(&config, 3, versionedSegmentInfo);
    
    segmentInfoData[0].occupiedStripeCount = 100;
    segmentInfoData[1].occupiedStripeCount = 12; 
    segmentInfoData[2].occupiedStripeCount = 0;
    versionedSegCtx.Load(segmentInfoData);

    int targetLogGroup = 0;
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<int>> changedValidBlkCount;
    tbb::concurrent_unordered_map<SegmentId, tbb::atomic<uint32_t>> changedOccupiedStripeCount;
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedValidBlockCount).WillRepeatedly(ReturnRef(changedValidBlkCount));
    EXPECT_CALL(*std::static_pointer_cast<MockVersionedSegmentInfo>(versionedSegmentInfo[targetLogGroup]),
        GetChangedOccupiedStripeCount).WillRepeatedly(ReturnRef(changedOccupiedStripeCount));

    SegmentInfoData* result = versionedSegCtx.GetUpdatedInfoDataToFlush(targetLogGroup);

    EXPECT_EQ(result[0].occupiedStripeCount, 100);
    EXPECT_EQ(result[1].occupiedStripeCount, 12);
    EXPECT_EQ(result[2].occupiedStripeCount, 0);

    // Test
    versionedSegCtx.NotifySegmentFreed(0);
    result = versionedSegCtx.GetUpdatedInfoDataToFlush(targetLogGroup);
    EXPECT_EQ(result[0].occupiedStripeCount, 0);
    EXPECT_EQ(result[1].occupiedStripeCount, 12);

    versionedSegCtx.NotifySegmentFreed(1);
    result = versionedSegCtx.GetUpdatedInfoDataToFlush(targetLogGroup);
    EXPECT_EQ(result[0].occupiedStripeCount, 0);
    EXPECT_EQ(result[1].occupiedStripeCount, 0);
}
} // namespace pos
