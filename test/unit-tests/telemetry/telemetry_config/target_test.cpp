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

#include "src/telemetry/telemetry_config/target.h"

#include <gtest/gtest.h>

#include <string>

#include "yaml-cpp/yaml.h"

namespace pos
{
TEST(Target, CheckUpdate0)
{
    Target target;

    std::string test1 = "ip";
    std::string test2 = "port";

    EXPECT_EQ(target.UpdateConfig(test1, test1), true);
    EXPECT_EQ(target.UpdateConfig(test2, test2), true);

    EXPECT_EQ(target.GetIp(), test1);
    EXPECT_EQ(target.GetPort(), test2);
}

TEST(Target, CheckUpdate1)
{
    Target* target = new Target();

    std::string test1 = "ip";
    std::string test2 = "port";

    EXPECT_EQ(target->UpdateConfig(test1, test1), true);
    EXPECT_EQ(target->UpdateConfig(test2, test2), true);

    EXPECT_EQ(target->GetIp(), test1);
    EXPECT_EQ(target->GetPort(), test2);

    delete target;
}

TEST(Target, CheckValue_Negative)
{
    Target target;

    EXPECT_EQ(target.GetIp(), "");
}
} // namespace pos
