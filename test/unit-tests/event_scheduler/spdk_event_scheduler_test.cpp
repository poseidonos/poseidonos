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

#include "src/event_scheduler/spdk_event_scheduler.h"

#include <gtest/gtest.h>

namespace pos
{

class StubEventSES : public Event
{
public:
    explicit StubEventSES(bool result)
    : Event()
    {
        resultExecute = result;
    }
    virtual bool Execute(void) final
    {
        return resultExecute;
    }

private:
    bool resultExecute;
};

TEST(SpdkEventScheduler, SendSpdkEvent_NullEvent)
{
    // Given: bool
    bool actual, expected = false;

    // When: Call SendSpdkEvent with nullptr
    actual = SpdkEventScheduler::SendSpdkEvent(0, nullptr);

    // Then: Expect to return false
    EXPECT_EQ(actual, expected);
}

TEST(SpdkEventScheduler, SendSpdkEvent_StubEvent)
{
    // Given: StubEventSES, bool
    auto event = std::make_shared<StubEventSES>(true);
    bool actual, expected = true;

    // When: Call SendSpdkEvent with StubEventSES
    actual = SpdkEventScheduler::SendSpdkEvent(0, event);

    // Then: Expect to return false
    EXPECT_EQ(actual, expected);
}

TEST(SpdkEventScheduler, ExecuteOrScheduleEvent_ExecuteDone)
{
    // Given: std::shared_ptr<StubEventSES>
    auto event = std::make_shared<StubEventSES>(true);

    // When: Call ExecuteOrScheduleEvent
    SpdkEventScheduler::ExecuteOrScheduleEvent(0, event);

    // Then: Do nothing
}

TEST(SpdkEventScheduler, ExecuteOrScheduleEvent_ExecuteNotDone)
{
    // Given: std::shared_ptr<StubEventSES>
    auto event = std::make_shared<StubEventSES>(false);

    // When: Call ExecuteOrScheduleEvent
    SpdkEventScheduler::ExecuteOrScheduleEvent(0, event);

    // Then: Do nothing
}

TEST(SpdkEventScheduler, InvokeEvent_EventPair)
{
    // Given: std::shared_ptr<StubEventSES>, EventSmartPtr
    auto event = std::make_shared<StubEventSES>(true);
    EventSmartPtr* arg = new EventSmartPtr(event);

    // When: Call InvokeEvent
    SpdkEventScheduler::InvokeEvent((void*)arg);

    // Then: Do nothing
}

} // namespace pos
