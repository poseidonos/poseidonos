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

#include "src/event_scheduler/event.h"

#include <gtest/gtest.h>

#include "src/cpu_affinity/affinity_manager.h"

#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

class StubEventE : public Event
{
public:
    StubEventE(bool isFrontEndEvent = false, BackendEvent event = BackendEvent_Unknown,
        AffinityManager* affinityManager = nullptr)
    : Event(isFrontEndEvent, event, affinityManager)
    {
    }
    virtual bool Execute(void) final
    {
        return true;
    }
};

TEST(Event, Event_Stack)
{
    // Given: Do nothing

    // When: Create StubEventE
    StubEventE stubEvent;

    // Then: Do nothing
}

TEST(Event, Event_Heap)
{
    // Given: Do nothing

    // When: Create StubEventE
    StubEventE* stubEvent = new StubEventE;
    delete stubEvent;

    // Then: Do nothing
}

TEST(Event, GetEventType_DefaultValue)
{
    // Given: StubEventE
    StubEventE stubEvent;
    BackendEvent actual, expected = BackendEvent_Unknown;

    // When: Call GetEventType
    actual = stubEvent.GetEventType();

    // Then: Expect to return BackendEvent_Unknown
    EXPECT_EQ(actual, expected);
}

TEST(Event, SetEventType_GC)
{
    // Given: StubEventE
    StubEventE stubEvent;
    BackendEvent actual, expected = BackendEvent_GC;

    // When: Call SetEventType
    stubEvent.SetEventType(BackendEvent_GC);
    actual = stubEvent.GetEventType();

    // Then: Expect to return BackendEvent_GC
    EXPECT_EQ(actual, expected);
}

TEST(Event, Execute_SimpleCall)
{
    // Given: StubEventE
    StubEventE stubEvent;
    bool actual, expected = true;

    // When: Call Execute
    actual = stubEvent.Execute();

    // Then: Expect to return true
    EXPECT_EQ(actual, expected);
}

TEST(Event, IsFrontEnd_DefaultValue)
{
    // Given: StubEventE
    StubEventE stubEvent;
    bool actual, expected = false;

    // When: Call IsFrontEnd
    actual = stubEvent.IsFrontEnd();

    // Then: Expect to return false
    EXPECT_EQ(actual, expected);
}

TEST(Event, SetFrontEnd_True)
{
    // Given: StubEventE
    StubEventE stubEvent;
    bool actual, expected = true;

    // When: Call SetFrontEnd true
    stubEvent.SetFrontEnd(true);
    actual = stubEvent.IsFrontEnd();

    // Then: Expect to return true
    EXPECT_EQ(actual, expected);
}

TEST(Event, GetNumaId_NormalNuma)
{
    // Given: StubEventE, MockAffinityManager
    NiceMock<MockAffinityManager> mockAffinityManager;
    ON_CALL(mockAffinityManager, GetNumaIdFromCurrentThread()).WillByDefault(Return(4444));
    StubEventE stubEvent {true, BackendEvent_MetaIO, &mockAffinityManager};
    uint32_t actual, expected = 4444;

    // When: Call GetNumaId
    actual = stubEvent.GetNumaId();

    // Then: Expect to return 4444
    EXPECT_EQ(actual, expected);
}

TEST(Event, GetNumaId_InvalidNuma)
{
    // Given: StubEventE, MockAffinityManager
    NiceMock<MockAffinityManager> mockAffinityManager;
    ON_CALL(mockAffinityManager, GetNumaIdFromCurrentThread()).WillByDefault(Return(INVALID_NUMA));
    StubEventE stubEvent {true, BackendEvent_MetaIO, &mockAffinityManager};
    uint32_t actual, expected = 0;

    // When: Call GetNumaId
    actual = stubEvent.GetNumaId();

    // Then: Expect to return 0 (Invalid case)
    EXPECT_EQ(actual, expected);
}

} // namespace pos
