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

#include "src/resource_checker/resource_checker.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ResourceChecker, Repeated_Cons_Des)
{
    // Given, When
    for (uint32_t iter = 0; iter < 10; iter++)
    {
        ResourceChecker* resChecker = new ResourceChecker();
        resChecker->SetSleepTime(0);
        resChecker->Enable();

        // Then
        delete resChecker;
        resChecker = nullptr;
    }
}

TEST(ResourceChecker, Enable)
{
    // Given, When
    ResourceChecker* resChecker = new ResourceChecker();
    resChecker->SetSleepTime(0);
    resChecker->Enable();
    uint32_t count = 0;
    do
    {
        count = resChecker->GetIterationCount();
        if (100 <= count)
        {
            break;
        }
    } while (true);

    // Then
    EXPECT_EQ(count >= 100, true);

    delete resChecker;
    resChecker = nullptr;
}

TEST(ResourceChecker, forced_delete_under_execution)
{
    // Given, When
    ResourceChecker* resChecker = new ResourceChecker();
    resChecker->SetSleepTime(0);
    resChecker->Enable();
    do
    {
        if (10 <= resChecker->GetIterationCount())
        {
            delete resChecker;
            break;
        }
    } while (true);

    // Then
    // ....?

    resChecker = nullptr;
}

TEST(ResourceChecker, forced_stop_and_reStart)
{
    // Given, When
    ResourceChecker* resChecker = new ResourceChecker();
    resChecker->SetSleepTime(0); // 1ms
    resChecker->Enable();
    do
    {
        if (10 <= resChecker->GetIterationCount())
        {
            delete resChecker;
            break;
        }
    } while (true);

    resChecker = new ResourceChecker();
    resChecker->SetSleepTime(0);
    resChecker->Enable();
    do
    {
        if (5 <= resChecker->GetIterationCount())
        {
            delete resChecker;
            break;
        }
    } while (true);

    // Then
    EXPECT_EQ(resChecker->GetIterationCount() >= 5, true);

    resChecker = nullptr;
}
} // namespace pos
