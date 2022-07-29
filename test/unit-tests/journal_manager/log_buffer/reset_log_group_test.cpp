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

#include "src/journal_manager/log_buffer/reset_log_group.h"

#include <gtest/gtest.h>

#include "src/journal_manager/log_buffer/log_group_footer_write_context.h"
#include "test/unit-tests/journal_manager/log_buffer/journal_log_buffer_mock.h"

using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
MATCHER_P(EqLogGroupFooterWriteContext, expected, "")
{
    return (*((LogGroupFooterWriteContext*)arg) == *((LogGroupFooterWriteContext*)expected));
}

TEST(ResetLogGroup, Execute_testIfLogBufferReseted)
{
    // Given
    int logGroupId = 0;
    NiceMock<MockJournalLogBuffer> logBuffer;
    LogGroupFooter footer;
    footer.isReseted = true;
    footer.resetedSequenceNumber = 1;
    uint64_t footerOffset = 0;

    ResetLogGroup event(&logBuffer, logGroupId, footer, footerOffset, nullptr);

    // When
    LogGroupFooterWriteContext context(logGroupId, nullptr);
    context.SetIoRequest(footerOffset, footer);
    EXPECT_CALL(logBuffer, InternalIo(EqLogGroupFooterWriteContext(&context)));
    bool result = event.Execute();

    // Then
    EXPECT_EQ(result, true);
}

} // namespace pos
