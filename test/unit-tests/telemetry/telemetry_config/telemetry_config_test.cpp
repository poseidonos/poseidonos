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

#include "src/telemetry/telemetry_config/telemetry_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "test/unit-tests/telemetry/common/config_observer_mock.h"

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
class TelemetryConfigFixture : public ::testing::Test
{
public:
    TelemetryConfigFixture(void)
    {
    }

    virtual ~TelemetryConfigFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        config = new TelemetryConfig(fileName);
    }

    virtual void
    TearDown(void)
    {
        delete config;
    }

protected:
    TelemetryConfig* config;
    std::string fileName = "./unit-tests/telemetry/telemetry_config/test.yaml";
};

TEST_F(TelemetryConfigFixture, Register_testIfTheSameObserverCanBeAddedTwiceWithDifferentKeys)
{
    MockConfigObserver* observer = new MockConfigObserver();
    MockConfigObserver* observer2 = new MockConfigObserver();
    std::multimap<std::string, ConfigObserver*>& oMap = config->GetObserversMap();

    std::string key1 = "key1";
    std::string key2 = "key2";
    std::string key3 = "key3";
    std::string key4 = "key4";

    EXPECT_EQ(config->Register(key1, observer), true);
    EXPECT_EQ(config->Register(key2, observer), true);
    EXPECT_EQ(config->Register(key3, observer), true);
    EXPECT_EQ(config->Register(key3, observer2), true);

    EXPECT_EQ(oMap.count(key1), 1);
    EXPECT_EQ(oMap.count(key2), 1);
    EXPECT_EQ(oMap.count(key3), 2);
    EXPECT_EQ(oMap.count(key4), 0);

    delete observer;
}

TEST_F(TelemetryConfigFixture, Register_testIfTheSameObserverCannotBeRegisteredTwice)
{
    MockConfigObserver* observer = new MockConfigObserver();
    std::multimap<std::string, ConfigObserver*>& oMap = config->GetObserversMap();

    std::string key = "key";

    EXPECT_EQ(config->Register(key, observer), true);
    EXPECT_EQ(config->Register(key, observer), false);

    delete observer;
}

TEST_F(TelemetryConfigFixture, Getter_testIfGrpcClientIsEnabledByDefault)
{
    EXPECT_EQ(config->GetClient().IsEnabled(), true);
}

TEST_F(TelemetryConfigFixture, Getter_testIfGrpcServerIsEnabledByDefault)
{
    EXPECT_EQ(config->GetServer().IsEnabled(), true);
}
} // namespace pos
