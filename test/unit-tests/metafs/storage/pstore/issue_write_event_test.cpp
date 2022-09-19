/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/storage/pstore/issue_write_event.h"

#include <gtest/gtest.h>

#include <functional>

#include "test/unit-tests/metafs/storage/pstore/mss_on_disk_mock.h"

namespace pos
{
class MssAioCbCxt;

using MssRequestFunction = std::function<POS_EVENT_ID(const IODirection, MssAioCbCxt*)>;

class IssueWriteEventTester
{
public:
    IssueWriteEventTester(void)
    : result(false)
    {
    }
    ~IssueWriteEventTester(void) = default;
    POS_EVENT_ID RequestFunction(const IODirection direction, MssAioCbCxt* cxt)
    {
        return result;
    }
    void SetExpectedResult(const int expected)
    {
        result = expected;
    }

private:
    int result;
};

TEST(IssueWriteEvent, Execute_testIfExecutionResultIsTrueWhenSUCCESS)
{
    IssueWriteEventTester tester;
    MssRequestFunction handler = std::bind(&IssueWriteEventTester::RequestFunction, &tester, std::placeholders::_1, std::placeholders::_2);
    IssueWriteEvent event(handler, nullptr);

    bool expected = true;
    tester.SetExpectedResult(EID(SUCCESS));

    EXPECT_EQ(event.Execute(), expected);
}

TEST(IssueWriteEvent, Execute_testIfExecutionResultIsTrueWhenMFS_IO_FAILED_DUE_TO_STOP_STATE)
{
    IssueWriteEventTester tester;
    MssRequestFunction handler = std::bind(&IssueWriteEventTester::RequestFunction, &tester, std::placeholders::_1, std::placeholders::_2);
    IssueWriteEvent event(handler, nullptr);

    bool expected = true;
    tester.SetExpectedResult(EID(MFS_IO_FAILED_DUE_TO_STOP_STATE));

    EXPECT_EQ(event.Execute(), expected);
}

TEST(IssueWriteEvent, Execute_testIfExecutionResultIsFalseWhenMFS_IO_FAILED_DUE_TO_TRYLOCK_FAIL)
{
    IssueWriteEventTester tester;
    MssRequestFunction handler = std::bind(&IssueWriteEventTester::RequestFunction, &tester, std::placeholders::_1, std::placeholders::_2);
    IssueWriteEvent event(handler, nullptr);

    bool expected = false;
    tester.SetExpectedResult(EID(MFS_IO_FAILED_DUE_TO_TRYLOCK_FAIL));

    EXPECT_EQ(event.Execute(), expected);
}

TEST(IssueWriteEvent, Execute_testIfExecutionResultIsFalseWhenMFS_IO_FAILED_DUE_TO_FAIL)
{
    IssueWriteEventTester tester;
    MssRequestFunction handler = std::bind(&IssueWriteEventTester::RequestFunction, &tester, std::placeholders::_1, std::placeholders::_2);
    IssueWriteEvent event(handler, nullptr);

    bool expected = false;
    tester.SetExpectedResult(EID(MFS_IO_FAILED_DUE_TO_FAIL));

    EXPECT_EQ(event.Execute(), expected);
}
} // namespace pos
