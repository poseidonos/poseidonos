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

#include "src/journal_manager/checkpoint/checkpoint_submission.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/checkpoint_handler_mock.h"
#include "test/unit-tests/journal_manager/checkpoint/dirty_map_manager_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/buffer_write_done_notifier_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/callback_sequence_controller_mock.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(CheckpointSubmission, Execute_testIfCheckpointStartedSuccessfully)
{
    // Given
    NiceMock<MockDirtyMapManager> dirtyMapManager;
    NiceMock<MockCheckpointHandler> checkpointHandler;
    NiceMock<MockCallbackSequenceController> sequenceController;
    NiceMock<MockEventScheduler> eventScheduler;
    int flushingLogGroupId = 0;
    CheckpointSubmission submission(&dirtyMapManager, &checkpointHandler, &sequenceController, flushingLogGroupId);

    EXPECT_CALL(dirtyMapManager, GetDirtyList);

    // Then: Checkpoint should be started after acquiring approval,
    // and allow callback execution afterwards
    {
        InSequence s;
        EXPECT_CALL(sequenceController, GetCheckpointExecutionApproval);
        EXPECT_CALL(checkpointHandler, Start).WillOnce(Return(0));
        EXPECT_CALL(sequenceController, AllowCallbackExecution);
    }
    // When
    bool result = submission.Execute();

    // Then: Execussion result should be 0
    EXPECT_EQ(result, true);
}

} // namespace pos
