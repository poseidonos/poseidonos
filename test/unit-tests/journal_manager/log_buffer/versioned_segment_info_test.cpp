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

#include "src/journal_manager/log_buffer/versioned_segment_info.h"
#include "tbb/concurrent_unordered_map.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(VersionedSegmentInfo, IncreaseValidBlockCount_testIfValidBlockCountIsIncreasedDecreasedAndReseted)
{
    // Given
    VersionedSegmentInfo versionedSegInfo;

    // When
    versionedSegInfo.IncreaseValidBlockCount(2, 3);
    versionedSegInfo.DecreaseValidBlockCount(2, 1);
    versionedSegInfo.IncreaseValidBlockCount(1, 4);

    // Then
    tbb::concurrent_unordered_map<SegmentId, int> expectChangedValidCount;
    expectChangedValidCount[2] = 2;
    expectChangedValidCount[1] = 4;

    auto var = versionedSegInfo.GetChangedValidBlockCount();

    EXPECT_EQ(expectChangedValidCount[1], var[1]);
    EXPECT_EQ(expectChangedValidCount[2], var[2]);

    // When
    versionedSegInfo.Reset();

    // Then
    EXPECT_EQ(true, versionedSegInfo.GetChangedValidBlockCount().empty());
}

TEST(VersionedSegmentInfo, IncreaseOccupiedStripeCount_testIfOccupiedStripeCountIsIncreasedAndReseted)
{
    // Given
    VersionedSegmentInfo versionedSegInfo;

    // When
    tbb::concurrent_unordered_map<SegmentId, uint32_t> expectChangedOccupiedCount;
    SegmentId targetSegment = 3;
    int increasedOccupiedCount = 5;
    for (int index = 0; index < increasedOccupiedCount; index++)
    {
        versionedSegInfo.IncreaseOccupiedStripeCount(targetSegment);
        expectChangedOccupiedCount[targetSegment]++;
    }

    targetSegment = 2;
    increasedOccupiedCount = 3;
    for (int index = 0; index < increasedOccupiedCount; index++)
    {
        versionedSegInfo.IncreaseOccupiedStripeCount(targetSegment);
        expectChangedOccupiedCount[targetSegment]++;
    }

    // Then
    auto var = versionedSegInfo.GetChangedOccupiedStripeCount();

    EXPECT_EQ(expectChangedOccupiedCount[3], var[3]);
    EXPECT_EQ(expectChangedOccupiedCount[2], var[2]);

    // When
    versionedSegInfo.Reset();

    // Then
    EXPECT_EQ(true, versionedSegInfo.GetChangedOccupiedStripeCount().empty());
}

} // namespace pos
