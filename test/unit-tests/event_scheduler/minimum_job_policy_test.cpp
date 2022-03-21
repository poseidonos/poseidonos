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

#include "src/event_scheduler/minimum_job_policy.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MinimumJobPolicy, MinimumJobPolicy_Stack)
{
    // Given: Do nothing

    // When: Create MinimumJobPolicy
    // MinimumJobPolicy minimumJobPolicy {3};

    // Then: Do nothing
}

TEST(MinimumJobPolicy, MinimumJobPolicy_Heap)
{
    // Given: Do nothing

    // When: Create MinimumJobPolicy
    // MinimumJobPolicy* minimumJobPolicy = new MinimumJobPolicy {3};
    // delete minimumJobPolicy;

    // Then: Do nothing
}

TEST(MinimumJobPolicy, GetProperWorkerID_SimpleCall)
{
    // Given: MininumJobPolicy, MockEventScheduler
    /*NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, GetWorkerIDMinimumJobs(_)).WillByDefault(Return(4444));
    MinimumJobPolicy minimumJobPolicy {3, &mockEventScheduler};
    uint32_t actual, expected = 4444;

    // When: Call GetProperWorkerID
    actual = minimumJobPolicy.GetProperWorkerID(0);

    // Then: Expect to return 4444
    EXPECT_EQ(actual, expected);*/
}

} // namespace pos
