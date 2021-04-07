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

#include "src/io/general_io/rba_state_manager.h"

#include "gtest/gtest.h"

using namespace ibofos;

class RBAStateManagerFixture : public testing::Test
{
protected:
    virtual void
    SetUp(void)
    {
        target = RbaStateManagerSingleton::Instance();
    }

    virtual void
    TearDown(void)
    {
        RbaStateManagerSingleton::ResetInstance();
    }
    RBAStateManager* target;

private:
};

TEST_F(RBAStateManagerFixture, OwnershipProtection_RBAStateManager)
{
    const uint32_t VOLUME_ID = 8;
    const uint64_t RBA_AMOUNT = 10000;
    const BlkAddr RBA = RBA_AMOUNT / 2;

    target->CreateRBAState(VOLUME_ID, RBA_AMOUNT);

    EXPECT_EQ(true, target->AcquireOwnership(VOLUME_ID, RBA));
    EXPECT_EQ(false, target->AcquireOwnership(VOLUME_ID, RBA));
    target->ReleaseOwnership(VOLUME_ID, RBA);
    EXPECT_EQ(true, target->AcquireOwnership(VOLUME_ID, RBA));

    target->DeleteRBAState(VOLUME_ID);
}
