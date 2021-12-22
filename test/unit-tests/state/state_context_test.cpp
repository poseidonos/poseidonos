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

#include "src/state/state_context.h"

#include <gtest/gtest.h>

namespace pos
{

TEST(StateContext, StateContext_testConstructor)
{
    // Given: nothing. (it's trivial test)
    string sender = "mock-sender";
    SituationEnum situ = SituationEnum::DEFAULT;

    // When
    StateContext sc(sender, situ);

    // Then
    ASSERT_EQ(sender, sc.Owner());
    ASSERT_EQ(situ, sc.GetSituation());
}

TEST(StateContext, ToStateType_testIfConversionIsDoneAsExpected)
{
    // Given
    string sender = "mock_sender";

    // When & Then
    ASSERT_EQ(StateEnum::OFFLINE, StateContext(sender, SituationEnum::DEFAULT).ToStateType());
    ASSERT_EQ(StateEnum::NORMAL, StateContext(sender, SituationEnum::NORMAL).ToStateType());
    ASSERT_EQ(StateEnum::PAUSE, StateContext(sender, SituationEnum::TRY_MOUNT).ToStateType());
    ASSERT_EQ(StateEnum::BUSY, StateContext(sender, SituationEnum::DEGRADED).ToStateType());
    ASSERT_EQ(StateEnum::PAUSE, StateContext(sender, SituationEnum::TRY_UNMOUNT).ToStateType());
    ASSERT_EQ(StateEnum::PAUSE, StateContext(sender, SituationEnum::JOURNAL_RECOVERY).ToStateType());
    ASSERT_EQ(StateEnum::BUSY, StateContext(sender, SituationEnum::REBUILDING).ToStateType());
    ASSERT_EQ(StateEnum::STOP, StateContext(sender, SituationEnum::FAULT).ToStateType());
}

TEST(StateContext, GetPriority_testIfPriorityIsMatchedWithExpectation)
{
    // Given
    string sender = "mock_sender";
    auto inputAndExpected =
    {
        // tuple of (expected, input)
        std::make_tuple(SituationEnum::DEFAULT, 0),
        std::make_tuple(SituationEnum::NORMAL, 1),
        std::make_tuple(SituationEnum::TRY_MOUNT, 1),
        std::make_tuple(SituationEnum::DEGRADED, 10),
        std::make_tuple(SituationEnum::TRY_UNMOUNT, 20),
        std::make_tuple(SituationEnum::REBUILDING, 20),
        std::make_tuple(SituationEnum::JOURNAL_RECOVERY, 30),
        std::make_tuple(SituationEnum::FAULT, 99)
    };

    // When & Then
    for (auto& tup : inputAndExpected)
    {
        ASSERT_EQ(std::get<1>(tup), StateContext("mock_sender", std::get<0>(tup)).GetPriority());
    }
}

} // namespace pos
