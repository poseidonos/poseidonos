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

#include "src/event_scheduler/event_factory.h"

#include <gtest/gtest.h>

namespace pos
{

class StubEventFactoryEF : public EventFactory
{
public:
    StubEventFactoryEF(void)
    : EventFactory()
    {
    }
    virtual EventSmartPtr Create(UbioSmartPtr ubio) final
    {
        return nullptr;
    }
};

TEST(EventFactory, EventFactory_Stack)
{
    // Given: Do nothing

    // When: Create StubEventFactoryEF
    StubEventFactoryEF stubEventFactory;

    // Then: Do nothing
}

TEST(EventFactory, EventFactory_Heap)
{
    // Given: Do nothing

    // When: Create StubEventFactoryEF
    StubEventFactoryEF* stubEventFactory = new StubEventFactoryEF;
    delete stubEventFactory;

    // Then: Do nothing
}

TEST(EventFactory, Create_SimpleCall)
{
    // Given: StubEventFactoryEF, UbioSmartPtr;
    StubEventFactoryEF stubEventFactory;
    UbioSmartPtr ubio;

    // When: Call Create
    stubEventFactory.Create(ubio);
}

} // namespace pos
