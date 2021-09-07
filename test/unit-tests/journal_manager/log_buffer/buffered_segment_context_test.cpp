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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/journal_manager/log_buffer/buffered_segment_context.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using testing::_;
using testing::NiceMock;
using testing::Return;

namespace pos
{
TEST(BufferedSegmentContext, IncreaseValidBlockCount_testIfValidBlockCountIsIncreasedDecreasedAndReseted)
{
    // Given
    BufferedSegmentContext bufferedSegCtx;

    // When
    bufferedSegCtx.IncreaseValidBlockCount(2, 3);
    bufferedSegCtx.DecreaseValidBlockCount(2, 1);
    bufferedSegCtx.IncreaseValidBlockCount(1, 4);

    // Then
    std::unordered_map<uint32_t, int> expectChangedValidCount;
    expectChangedValidCount[2] = 2;
    expectChangedValidCount[1] = 4;

    EXPECT_EQ(expectChangedValidCount, bufferedSegCtx.GetChangedValidBlockCount());

    // When
    bufferedSegCtx.Reset();

    // Then
    EXPECT_EQ(true, bufferedSegCtx.GetChangedValidBlockCount().empty());
}

TEST(BufferedSegmentContext, IncreaseOccupiedStripeCount_testIfOccupiedStripeCountIsIncreasedAndReseted)
{
    // Given
    BufferedSegmentContext bufferedSegCtx;

    // When
    std::unordered_map<uint32_t, uint32_t> expectChangedOccupiedCount;
    SegmentId targetSegment = 3;
    int increasedOccupiedCount = 5;
    for (int index = 0; index < increasedOccupiedCount; index++)
    {
        bufferedSegCtx.IncreaseOccupiedStripeCount(targetSegment);
        expectChangedOccupiedCount[targetSegment]++;
    }

    targetSegment = 2;
    increasedOccupiedCount = 3;
    for (int index = 0; index < increasedOccupiedCount; index++)
    {
        bufferedSegCtx.IncreaseOccupiedStripeCount(targetSegment);
        expectChangedOccupiedCount[targetSegment]++;
    }

    // Then
    EXPECT_EQ(expectChangedOccupiedCount, bufferedSegCtx.GetChangedOccupiedStripeCount());

    // When
    bufferedSegCtx.Reset();

    // Then
    EXPECT_EQ(true, bufferedSegCtx.GetChangedOccupiedStripeCount().empty());
}

} // namespace pos
