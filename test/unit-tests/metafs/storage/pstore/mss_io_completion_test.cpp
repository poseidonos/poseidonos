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

#include "src/metafs/storage/pstore/mss_io_completion.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test/unit-tests/metafs/storage/pstore/mss_aio_cb_cxt_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MssIoCompletion, Execute)
{
    NiceMock<MockMssAioCbCxt> cb;
    MssIoCompletion completion(&cb);

    EXPECT_CALL(cb, SaveIOStatus);
    EXPECT_CALL(cb, InvokeCallback);

    completion.Execute();
}

TEST(MssIoCompletion, GetEventType_testIfTheEventTypeIsDifferentDependingOnTheParameter_NoJournal)
{
    NiceMock<MockMssAioCbCxt> cb;
    MssIoCompletion completion(&cb);

    EXPECT_EQ(completion.GetEventType(), BackendEvent_MetaIO);
}

TEST(MssIoCompletion, GetEventType_testIfTheEventTypeIsDifferentDependingOnTheParameter_Journal)
{
    NiceMock<MockMssAioCbCxt> cb;
    MssIoCompletion completion(&cb, true);

    EXPECT_EQ(completion.GetEventType(), BackendEvent_JournalIO);
}
} // namespace pos
