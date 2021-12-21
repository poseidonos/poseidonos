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

#include "src/telemetry/telemetry_config/telemetry_config.h"
#include "src/helper/file/directory.h"
#include "src/helper/file/file.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "test/unit-tests/telemetry/common/config_observer_mock.h"

using ::testing::_;
using ::testing::Matcher;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace std;

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
    SetUp(void) override
    {
        config = make_shared<TelemetryConfig>(PATH, FILE_NAME);
    }

    virtual void
    TearDown(void) override
    {
    }

protected:
    shared_ptr<TelemetryConfig> config;
    const string PATH = "./unit-tests/telemetry/telemetry_config/";
    const string FILE_NAME = "test.yaml";
};

TEST(TelemetryConfig, testIfThereIsNoConfigurationFile)
{
    EXPECT_DEATH(new TelemetryConfig("invalid", "path"), "")
        << "1. is the file config reader valid?\n2. is the conf file valid?";
}

TEST_F(TelemetryConfigFixture, Register_testIfTheSameObserverCanBeAddedTwiceWithDifferentKeys)
{
    const shared_ptr<MockConfigObserver> observer = make_shared<MockConfigObserver>();
    const shared_ptr<MockConfigObserver> observer2 = make_shared<MockConfigObserver>();
    multimap<string, shared_ptr<ConfigObserver>>& oMap = config->GetObserversMap();

    const string key1 = "key1";
    const string key2 = "key2";
    const string key3 = "key3";
    const string key4 = "key4";

    EXPECT_EQ(config->Register(TelemetryConfigType::Client, key1, observer), true);
    EXPECT_EQ(config->Register(TelemetryConfigType::Client, key2, observer), true);
    EXPECT_EQ(config->Register(TelemetryConfigType::Client, key3, observer), true);
    EXPECT_EQ(config->Register(TelemetryConfigType::Client, key3, observer2), true);

    EXPECT_EQ(oMap.count(config->GetCompositeKey(TelemetryConfigType::Client, key1)), 1);
    EXPECT_EQ(oMap.count(config->GetCompositeKey(TelemetryConfigType::Client, key2)), 1);
    EXPECT_EQ(oMap.count(config->GetCompositeKey(TelemetryConfigType::Client, key3)), 2);
    EXPECT_EQ(oMap.count(config->GetCompositeKey(TelemetryConfigType::Client, key4)), 0);
}

TEST_F(TelemetryConfigFixture, Register_testIfTheObserverCanBeReceivedNotification)
{
    const shared_ptr<MockConfigObserver> observer = make_shared<MockConfigObserver>();
    ClientConfig client;
    const string key = "enabled";

    EXPECT_EQ(TelemetryConfigSingleton::Instance(PATH, FILE_NAME)->Register(TelemetryConfigType::Client, key, observer), true);

    // config won't call the observer due to last flag
    client.UpdateConfig(TelemetryConfigType::Client, key, true, false);

    EXPECT_CALL(*observer, Notify);

    // config will call the observer
    client.UpdateConfig(TelemetryConfigType::Client, key, true, true);

    TelemetryConfigSingleton::ResetInstance();
}

TEST_F(TelemetryConfigFixture, Register_testIfTheSameObserverCannotBeRegisteredTwice)
{
    const shared_ptr<MockConfigObserver> observer = make_shared<MockConfigObserver>();
    multimap<string, shared_ptr<ConfigObserver>>& oMap = config->GetObserversMap();

    const string key = "key";

    EXPECT_EQ(config->Register(TelemetryConfigType::Client, key, observer), true);
    EXPECT_EQ(config->Register(TelemetryConfigType::Client, key, observer), false);
}

TEST_F(TelemetryConfigFixture, Getter_testIfGrpcClientIsEnabledByDefault)
{
    EXPECT_EQ(config->GetClient().IsEnabled(), true);
}

TEST_F(TelemetryConfigFixture, ConfigurationFileName_testIfThePathIsCorrect)
{
    EXPECT_EQ(0, config->ConfigurationFileName().compare("telemetry_default.yaml"));
}
} // namespace pos
