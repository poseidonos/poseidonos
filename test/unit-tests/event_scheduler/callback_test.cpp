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

#include "src/event_scheduler/callback.h"

#include <gtest/gtest.h>

#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/lib/system_timeout_checker_mock.h"
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class StubCallbackC : public Callback
{
public:
    StubCallbackC(bool isFrontEnd, CallbackType type = CallbackType_Unknown, uint32_t weight = 1,
        SystemTimeoutChecker* timeoutChecker = nullptr, EventScheduler* eventSchedulerArg = nullptr)
    : Callback(isFrontEnd, type, weight, timeoutChecker, eventSchedulerArg)
    {
    }
    virtual ~StubCallbackC(void) override
    {
    }
    void SetResultSpecificJob(bool value)
    {
        resultSpecificJob = value;
    }

private:
    virtual bool _DoSpecificJob(void) final
    {
        return resultSpecificJob;
    }
    bool resultSpecificJob{false};
};

TEST(Callback, Callback_Stack_DebugEnable)
{
    // Given: Set DumpSharedModuleInstanceEnable::debugLevelEnable true
    bool original_value = DumpSharedModuleInstanceEnable::debugLevelEnable;
    DumpSharedModuleInstanceEnable::debugLevelEnable = true;

    // When: Create StubCallbackC
    StubCallbackC callback{true};
    DumpSharedModuleInstanceEnable::debugLevelEnable = original_value;

    // Then: Do nothing
}

TEST(Callback, Callback_Stack_DebugDisable)
{
    // Given: Set DumpSharedModuleInstanceEnable::debugLevelEnable false
    bool original_value = DumpSharedModuleInstanceEnable::debugLevelEnable;
    DumpSharedModuleInstanceEnable::debugLevelEnable = false;

    // When: Create StubCallbackC
    StubCallbackC callback{true};
    DumpSharedModuleInstanceEnable::debugLevelEnable = original_value;

    // Then: Do nothing
}

TEST(Callback, Callback_Heap)
{
    // Given: Set DumpSharedModuleInstanceEnable::debugLevelEnable true, MockSystemTimeoutChecker
    bool original_value = DumpSharedModuleInstanceEnable::debugLevelEnable;
    DumpSharedModuleInstanceEnable::debugLevelEnable = true;
    MockSystemTimeoutChecker* mockTimeoutChecker = new MockSystemTimeoutChecker;
    ON_CALL(*mockTimeoutChecker, CheckTimeout()).WillByDefault(Return(true));
    EXPECT_CALL(*mockTimeoutChecker, CheckTimeout()).Times(1);

    // When: Create StubCallbackC
    StubCallbackC* callback = new StubCallbackC {true, CallbackType_Unknown, 1, mockTimeoutChecker};
    delete callback;

    DumpSharedModuleInstanceEnable::debugLevelEnable = original_value;

    // Then: Do nothing
}

TEST(Callback, SetTimeout)
{
    Callback::SetTimeout(50);
    Callback::SetTimeout(120);
    Callback::SetTimeout(5);
}

TEST(Callback, Execute_WithoutInvokeCallee)
{
    // Given: StubCallbackC, SetResultSpecificJob
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    StubCallbackC callback{true, CallbackType_Unknown, 1, nullptr, &mockEventScheduler};
    callback.SetResultSpecificJob(false);
    bool actual, expected = false;

    // When: Call Execute
    actual = callback.Execute();

    // Then: Expect to return false
    EXPECT_EQ(actual, expected);
}

TEST(Callback, Execute_WithInvokeCallee_NormalCallee1)
{
    // Given: StubCallbackC, callee
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    StubCallbackC callback{true, CallbackType_Unknown, 1, nullptr, &mockEventScheduler};
    callback.SetResultSpecificJob(true);
    auto callee = std::make_shared<StubCallbackC>(true);
    callee.get()->SetWaitingCount(1);
    callee.get()->SetResultSpecificJob(false);
    callback.SetCallee(callee);
    bool actual, expected = true;

    // When: Call Execute
    actual = callback.Execute();

    // Then: Expect to return true
    EXPECT_EQ(actual, expected);
}

TEST(Callback, Execute_WithInvokeCallee_NormalCallee2)
{
    // Given: StubCallbackC, callee
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    StubCallbackC callback{true, CallbackType_Unknown, 1, nullptr, &mockEventScheduler};
    callback.SetResultSpecificJob(true);
    auto callee = std::make_shared<StubCallbackC>(true);
    callee.get()->SetWaitingCount(1);
    callee.get()->SetResultSpecificJob(true);
    callback.SetCallee(callee);
    bool actual, expected = true;

    // When: Call Execute
    actual = callback.Execute();

    // Then: Expect to return true
    EXPECT_EQ(actual, expected);
}

TEST(Callback, Execute_WithInvokeCallee_ErrorCallee)
{
    // Given: StubCallbackC, callee
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    StubCallbackC callback{true, CallbackType_Unknown, 3, nullptr, &mockEventScheduler};

    callback.SetResultSpecificJob(true);
    callback.InformError(IOErrorType::GENERIC_ERROR);
    auto callee = std::make_shared<StubCallbackC>(true);
    callback.SetCallee(callee);
    bool actual, expected = true;

    // When: Call Execute
    actual = callback.Execute();

    // Then: Expect to return true
    EXPECT_EQ(actual, expected);
}

TEST(Callback, Execute_WithInvokeCallee_OverwaitingCallee)
{
    // Given: StubCallbackC, callee
    NiceMock<MockEventScheduler> mockEventScheduler;
    ON_CALL(mockEventScheduler, EnqueueEvent(_)).WillByDefault(Return());
    StubCallbackC callback{true, CallbackType_Unknown, 1, nullptr, &mockEventScheduler};

    callback.SetResultSpecificJob(true);
    auto callee = std::make_shared<StubCallbackC>(true);
    callee.get()->SetWaitingCount(10);
    callback.SetCallee(callee);
    bool actual, expected = true;

    // When: Call Execute
    actual = callback.Execute();

    // Then: Expect to return true
    EXPECT_EQ(actual, expected);
}

TEST(Callback, SetWaitingCount_SimpleCall)
{
    // Given: StubCallbackC
    StubCallbackC callback{true};

    // When: Call SetWaitingCount
    callback.SetWaitingCount(10);

    // Then: Do nothing
}

TEST(Callback, SetCallee_InvalidCallee)
{
    // Given: StubCallbackC
    StubCallbackC callback{true};
    CallbackSmartPtr callee;

    // When: Call SetCallee
    callback.SetCallee(nullptr);
    callback.SetCallee(callee);

    // Then: Do nothing
}

TEST(Callback, SetCallee_ValidCallee_DebugEnable)
{
    // Given: StubCallbackC
    bool original_value = DumpSharedModuleInstanceEnable::debugLevelEnable;
    DumpSharedModuleInstanceEnable::debugLevelEnable = true;
    StubCallbackC callback{true};
    auto callee = std::make_shared<StubCallbackC>(true);

    // When: Call SetCallee
    callback.SetCallee(callee);
    DumpSharedModuleInstanceEnable::debugLevelEnable = original_value;

    // Then: Do nothing
}

TEST(Callback, SetCallee_ValidCallee_DebugDisable)
{
    // Given: StubCallbackC
    bool original_value = DumpSharedModuleInstanceEnable::debugLevelEnable;
    DumpSharedModuleInstanceEnable::debugLevelEnable = false;
    StubCallbackC callback{true};
    auto callee = std::make_shared<StubCallbackC>(true);

    // When: Call SetCallee
    callback.SetCallee(callee);
    DumpSharedModuleInstanceEnable::debugLevelEnable = original_value;

    // Then: Do nothing
}

TEST(Callback, InformError_Success)
{
    // Given: StubCallbackC
    StubCallbackC callback{true};

    // When: Call SetWaitingCount
    callback.InformError(IOErrorType::SUCCESS);

    // Then: Do nothing
}

TEST(Callback, InformError_DeviceError)
{
    // Given: StubCallbackC
    StubCallbackC callback{true};

    // When: Call SetWaitingCount
    callback.InformError(IOErrorType::DEVICE_ERROR);

    // Then: Do nothing
}

} // namespace pos
