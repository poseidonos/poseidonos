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

#include "src/network/transport_configuration.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/helper/spdk_rpc_client_mock.h"
#include "test/unit-tests/master_context/config_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgPointee;

namespace pos
{
class TransportConfigurationFixture : public ::testing::Test
{
public:
    TransportConfigurationFixture(void)
    {
    }

    virtual ~TransportConfigurationFixture(void)
    {
    }

protected:
    NiceMock<MockConfigManager> mockConfigManager;
    NiceMock<MockSpdkRpcClient>* mockSpdkRpcClient = new NiceMock<MockSpdkRpcClient>();
};

TEST_F(TransportConfigurationFixture, TransportConfiguration_ZeroArgument_Stack)
{
    // When: Create new object in stack
    TransportConfiguration transportConfiguration;
}

TEST_F(TransportConfigurationFixture, TransportConfiguration_OneArgument_Stack)
{
    // When: Create new object in stack
    TransportConfiguration transportConfiguration(&mockConfigManager);
}

TEST_F(TransportConfigurationFixture, TransportConfiguration_TwoArgument_Stack)
{
    // When: Create new object in stack
    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
}

TEST_F(TransportConfigurationFixture, TransportConfiguration_ZeroArgument_Heap)
{
    // When: Create new object in stack
    TransportConfiguration* transportConfiguration = new TransportConfiguration();

    delete transportConfiguration;
}

TEST_F(TransportConfigurationFixture, TransportConfiguration_OneArgument_Heap)
{
    // When: Create new object in stack
    TransportConfiguration* transportConfiguration = new TransportConfiguration(&mockConfigManager);

    delete transportConfiguration;
}

TEST_F(TransportConfigurationFixture, TransportConfiguration_TwoArgument_Heap)
{
    // When: Create new object in stack
    TransportConfiguration* transportConfiguration = new TransportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    delete transportConfiguration;
}

TEST_F(TransportConfigurationFixture, ReadConfig_Success)
{
    ON_CALL(mockConfigManager, GetValue(_, _, _, _)).WillByDefault(Return(EID(SUCCESS)));
    TransportConfiguration transportConfiguration;
    transportConfiguration.ReadConfig();
}

TEST_F(TransportConfigurationFixture, ReadConfig_GetTrasnportTypeFailed)
{
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).WillOnce(Return((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR));
    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    transportConfiguration.ReadConfig();
}

TEST_F(TransportConfigurationFixture, ReadConfig_GetBufCacheSizeFailed)
{
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).WillOnce(Return(EID(SUCCESS)))
                                                        .WillOnce(Return((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR))
                                                        .WillOnce(Return((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR));
    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    transportConfiguration.ReadConfig();
}

TEST_F(TransportConfigurationFixture, ReadConfig_GetNumSharedBufferFailed)
{
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).WillOnce(Return(EID(SUCCESS)))
                                                        .WillOnce(Return(EID(SUCCESS)))
                                                        .WillOnce(Return((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR));
    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    transportConfiguration.ReadConfig();
}

TEST_F(TransportConfigurationFixture, CreateTransport_Success)
{
    const int SUCCESS = 0;
    pair<int, std::string> value = make_pair(SUCCESS, "");
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).WillOnce([&](string module, string key, void* enabled, ConfigType type)
    {
        bool* targetToChange = static_cast<bool*>(enabled);
        *targetToChange = true;
        return SUCCESS;
    }).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*mockSpdkRpcClient, TransportCreate(_, _, _)).WillOnce(Return(value));

    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    transportConfiguration.CreateTransport();
}

TEST_F(TransportConfigurationFixture, CreateTransport_TransportConfigDisabled)
{
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).WillOnce(Return((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR));

    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    transportConfiguration.CreateTransport();
}

TEST_F(TransportConfigurationFixture, CreateTransport_CreateTransport_Fail)
{
    const int SUCCESS = 0;
    pair<int, std::string> value = make_pair(-1, "");
    EXPECT_CALL(mockConfigManager, GetValue(_, _, _, _)).WillOnce([&](string module, string key, void* enabled, ConfigType type)
    {
        bool* targetToChange = static_cast<bool*>(enabled);
        *targetToChange = true;
        return SUCCESS;
    }).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*mockSpdkRpcClient, TransportCreate(_, _, _)).WillOnce(Return(value));

    TransportConfiguration transportConfiguration(&mockConfigManager, mockSpdkRpcClient);
    transportConfiguration.CreateTransport();
}
} // namespace pos
