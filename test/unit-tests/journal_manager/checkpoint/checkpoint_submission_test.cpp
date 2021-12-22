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

#include "src/journal_manager/checkpoint/checkpoint_submission.h"

#include <gtest/gtest.h>

#include "test/unit-tests/journal_manager/checkpoint/checkpoint_manager_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(CheckpointSubmission, Execute_testIfCheckpointStartedSuccessfully)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;

    int logGroupId = 1;
    CheckpointSubmission submission(&checkpointManager, nullptr, logGroupId);

    // When
    EXPECT_CALL(checkpointManager, RequestCheckpoint(logGroupId, _)).WillOnce(Return(0));
    bool result = submission.Execute();

    // Then: Execussion result should be true
    EXPECT_EQ(result, true);
}

TEST(CheckpointSubmission, Execute_testIfCheckpointSubmissionFailsWhenRequestFails)
{
    // Given
    NiceMock<MockCheckpointManager> checkpointManager;

    int logGroupId = 1;
    CheckpointSubmission submission(&checkpointManager, nullptr, logGroupId);

    // When
    EXPECT_CALL(checkpointManager, RequestCheckpoint(logGroupId, _)).WillOnce(Return(-1));
    bool result = submission.Execute();

    // Then: Execussion result should be false
    EXPECT_EQ(result, false);
}
} // namespace pos
