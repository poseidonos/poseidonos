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

#include "src/telemetry/telemetry_air/telemetry_air_delegator.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using namespace pos;
using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{

TEST(TelemetryAirDelegator, TelemetryAirDelegator_Stack)
{
    // Given: Do nothing

    // When: Create TelemetryAirDelegator
    TelemetryAirDelegator telAirDelegator {nullptr};

    // Then: Do nothing
}

TEST(TelemetryAirDelegator, TelemetryAirDelegator_Heap)
{
    // Given: Do nothing

    // When: Create TelemetryAirDelegator
    TelemetryAirDelegator* telAirDelegator = new TelemetryAirDelegator {nullptr};
    delete telAirDelegator;

    // Then: Do nothing
}

TEST(TelemetryAirDelegator, SetState_SimpleCall)
{
    // Given: TelemetryAirDelegator
    TelemetryAirDelegator telAirDelegator {nullptr};

    // When: Call SetState
    telAirDelegator.SetState(TelemetryAirDelegator::State::END);

    // Then: Do nothing
}

TEST(TelemetryAirDelegator, RegisterAirEvent_SimpleCall)
{
    // Given: TelemetryAirDelegator
    TelemetryAirDelegator telAirDelegator {nullptr};

    // When: Call RegisterAirEvent
    telAirDelegator.RegisterAirEvent();

    // Then: Do nothing
}

TEST(TelemetryAirDelegator, dataHandler_RunState_ValidData)
{
    // Given: MockTelemetryPublisher, TelemetryAirDelegator, air_data
    NiceMock<MockTelemetryPublisher> mockTelPub;
    ON_CALL(mockTelPub, PublishData).WillByDefault(Return(0));
    TelemetryAirDelegator telAirDelegator {&mockTelPub};
    auto& air_data = air::json("air_data");
    auto& node_arr_vol = air::json("arr_vol");
    auto& obj_1 = air::json("obj_1");
    obj_1["filter"] = {"AIR_READ"};
    obj_1["iops"] = {1234};
    auto& obj_2 = air::json("obj_2");
    obj_2["filter"] = {"AIR_WRITE"};
    obj_2["iops"] = {5678};
    node_arr_vol["objs"] = {obj_1, obj_2};
    air_data["PERF_ARR_VOL"] = {node_arr_vol};
    int actual, expected = 0;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns zero(success), Clear AIR data
    EXPECT_EQ(actual, expected);
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_RunState_InvalidData)
{
    // Given: MockTelemetryPublisher, TelemetryAirDelegator, air_data
    NiceMock<MockTelemetryPublisher> mockTelPub;
    ON_CALL(mockTelPub, PublishData).WillByDefault(Return(0));
    TelemetryAirDelegator telAirDelegator {&mockTelPub};
    auto& air_data = air::json("air_data");
    auto& node_arr_vol = air::json("arr_vol");
    air_data["INVALID_NODE"] = {node_arr_vol};
    int actual, expected = 2;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns two(unexpected stop, err_data, exception path), Clear AIR data
    EXPECT_EQ(actual, expected);
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_EndState)
{
    // Given: MockTelemetryPublisher, TelemetryAirDelegator, air_data
    NiceMock<MockTelemetryPublisher> mockTelPub;
    ON_CALL(mockTelPub, PublishData).WillByDefault(Return(0));
    TelemetryAirDelegator telAirDelegator {&mockTelPub};
    auto& air_data = air::json("air_data");
    telAirDelegator.SetState(TelemetryAirDelegator::State::END);
    int actual, expected = 1;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns one(normal stop, skip logic), Clear AIR data
    EXPECT_EQ(actual, expected);
    air::json_clear();
}

} // namespace pos
