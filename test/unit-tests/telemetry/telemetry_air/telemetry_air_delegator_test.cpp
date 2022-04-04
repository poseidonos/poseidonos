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

TEST(TelemetryAirDelegator, dataHandler_RunState_EmptyData)
{
    // Given: TelemetryAirDelegator, air_data
    TelemetryAirDelegator telAirDelegator {nullptr};
    auto& air_data = air::json("air_data");
    int actual, expected = 0;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns zero(success), Clear AIR data
    EXPECT_EQ(actual, expected);
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_RunState_InvalidData)
{
    // Given: TelemetryAirDelegator, air_data
    TelemetryAirDelegator telAirDelegator {nullptr};
    auto& air_data = air::json("air_data");
    auto& empty_node = air::json("empty_node");
    air_data["PERF_ARR_VOL"] = {empty_node};
    int actual, expected = 2;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns two(unexpected stop, err_data, exception path), Clear AIR data
    EXPECT_EQ(actual, expected);
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_EndState)
{
    // Given: TelemetryAirDelegator, air_data
    TelemetryAirDelegator telAirDelegator {nullptr};
    auto& air_data = air::json("air_data");
    telAirDelegator.SetState(TelemetryAirDelegator::State::END);
    int actual, expected = 1;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns one(normal stop, skip logic), Clear AIR data
    EXPECT_EQ(actual, expected);
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_RunState_PERF_ARR_VOL_Data)
{
    // Given: POSMetricVector, MockTelemetryPublisher, TelemetryAirDelegator, air_data
    POSMetricVector* posMetricVector {nullptr};
    NiceMock<MockTelemetryPublisher> mockTelPub;
    ON_CALL(mockTelPub, PublishMetricList(_)).WillByDefault(
        [&] (POSMetricVector* posMetricVectorArg)
        {
            posMetricVector = posMetricVectorArg;
            return 0;
        }
    );
    TelemetryAirDelegator telAirDelegator {&mockTelPub};
    auto& air_data = air::json("air_data");
    air_data["interval"] = {3};
    auto& perf_arr_vol = air::json("perf_arr_vol");
    auto& obj_read = air::json("obj_read");
    auto& obj_read_period = air::json("obj_read_period");
    obj_read["filter"] = {"AIR_READ"};
    obj_read_period["iops"] = {100};
    obj_read_period["bw"] = {409600};
    obj_read["period"] = {obj_read_period};
    obj_read["index"] = {0x0306};
    obj_read["target_id"] = {7824};
    obj_read["target_name"] = {"reactor_0"};
    auto& obj_write = air::json("obj_write");
    auto& obj_write_period = air::json("obj_write_period");
    obj_write["filter"] = {"AIR_WRITE"};
    obj_write_period["iops"] = {10};
    obj_write_period["bw"] = {1310720};
    obj_write["period"] = {obj_write_period};
    obj_write["index"] = {0x0306};
    obj_write["target_id"] = {7824};
    obj_write["target_name"] = {"reactor_0"};
    perf_arr_vol["objs"] = {obj_read, obj_write};
    air_data["PERF_ARR_VOL"] = {perf_arr_vol};
    int actual, expected = 0;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns zero(success), Clear AIR data
    EXPECT_EQ(actual, expected);
    EXPECT_EQ(posMetricVector->size(), 4);
    EXPECT_EQ(posMetricVector->at(0).GetName(), TEL50000_READ_IOPS);
    EXPECT_EQ(posMetricVector->at(0).GetGaugeValue(), 100);
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("thread_id"), "7824");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("thread_name"), "\"reactor_0\"");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("interval"), "3");
    EXPECT_EQ(posMetricVector->at(1).GetName(), TEL50001_READ_RATE_BYTES_PER_SECOND);
    EXPECT_EQ(posMetricVector->at(1).GetGaugeValue(), 409600);
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("thread_id"), "7824");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("thread_name"), "\"reactor_0\"");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("interval"), "3");
    EXPECT_EQ(posMetricVector->at(2).GetName(), TEL50010_WRITE_IOPS);
    EXPECT_EQ(posMetricVector->at(2).GetGaugeValue(), 10);
    EXPECT_EQ(posMetricVector->at(2).GetLabelList()->at("thread_id"), "7824");
    EXPECT_EQ(posMetricVector->at(2).GetLabelList()->at("thread_name"), "\"reactor_0\"");
    EXPECT_EQ(posMetricVector->at(2).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(2).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(2).GetLabelList()->at("interval"), "3");
    EXPECT_EQ(posMetricVector->at(3).GetName(), TEL50011_WRITE_RATE_BYTES_PER_SECOND);
    EXPECT_EQ(posMetricVector->at(3).GetGaugeValue(), 1310720);
    EXPECT_EQ(posMetricVector->at(3).GetLabelList()->at("thread_id"), "7824");
    EXPECT_EQ(posMetricVector->at(3).GetLabelList()->at("thread_name"), "\"reactor_0\"");
    EXPECT_EQ(posMetricVector->at(3).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(3).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(3).GetLabelList()->at("interval"), "3");
    delete posMetricVector;
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_RunState_LAT_ARR_VOL_READ_Data)
{
    // Given: POSMetricVector, MockTelemetryPublisher, TelemetryAirDelegator, air_data
    POSMetricVector* posMetricVector {nullptr};
    NiceMock<MockTelemetryPublisher> mockTelPub;
    ON_CALL(mockTelPub, PublishMetricList(_)).WillByDefault(
        [&] (POSMetricVector* posMetricVectorArg)
        {
            posMetricVector = posMetricVectorArg;
            return 0;
        }
    );
    TelemetryAirDelegator telAirDelegator {&mockTelPub};
    auto& air_data = air::json("air_data");
    auto& lat_arr_vol_read = air::json("lat_arr_vol_read");
    auto& obj = air::json("obj");
    auto& obj_period = air::json("obj_period");
    obj["index"] = {0x0306};
    obj_period["mean"] = {234};
    obj_period["max"] = {8472};
    obj_period["sample_cnt"] = {50};
    obj["period"] = {obj_period};
    lat_arr_vol_read["objs"] += {obj};
    air_data["LAT_ARR_VOL_READ"] = {lat_arr_vol_read};
    int actual, expected = 0;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns zero(success), Clear AIR data
    EXPECT_EQ(actual, expected);
    EXPECT_EQ(posMetricVector->size(), 2);
    EXPECT_EQ(posMetricVector->at(0).GetName(), TEL50002_READ_LATENCY_MEAN_NS);
    EXPECT_EQ(posMetricVector->at(0).GetGaugeValue(), 234);
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("sample_count"), "50");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("interval"), "0");
    EXPECT_EQ(posMetricVector->at(1).GetName(), TEL50003_READ_LATENCY_MAX_NS);
    EXPECT_EQ(posMetricVector->at(1).GetGaugeValue(), 8472);
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("sample_count"), "50");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("interval"), "0");
    delete posMetricVector;
    air::json_clear();
}

TEST(TelemetryAirDelegator, dataHandler_RunState_LAT_ARR_VOL_WRITE_Data)
{
    // Given: POSMetricVector, MockTelemetryPublisher, TelemetryAirDelegator, air_data
    POSMetricVector* posMetricVector {nullptr};
    NiceMock<MockTelemetryPublisher> mockTelPub;
    ON_CALL(mockTelPub, PublishMetricList(_)).WillByDefault(
        [&] (POSMetricVector* posMetricVectorArg)
        {
            posMetricVector = posMetricVectorArg;
            return 0;
        }
    );
    TelemetryAirDelegator telAirDelegator {&mockTelPub};
    auto& air_data = air::json("air_data");
    auto& lat_arr_vol_write = air::json("lat_arr_vol_write");
    auto& obj = air::json("obj");
    auto& obj_period = air::json("obj_period");
    obj["index"] = {0x0306};
    obj_period["mean"] = {36804};
    obj_period["max"] = {7362942};
    obj_period["sample_cnt"] = {50};
    obj["period"] = {obj_period};
    lat_arr_vol_write["objs"] += {obj};
    air_data["LAT_ARR_VOL_WRITE"] = {lat_arr_vol_write};
    int actual, expected = 0;

    // When: Call dataHandler
    actual = telAirDelegator.dataHandler(air_data);

    // Then: Expect dataHandler returns zero(success), Clear AIR data
    EXPECT_EQ(actual, expected);
    EXPECT_EQ(posMetricVector->size(), 2);
    EXPECT_EQ(posMetricVector->at(0).GetName(), TEL50012_WRITE_LATENCY_MEAN_NS);
    EXPECT_EQ(posMetricVector->at(0).GetGaugeValue(), 36804);
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("sample_count"), "50");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(0).GetLabelList()->at("interval"), "0");
    EXPECT_EQ(posMetricVector->at(1).GetName(), TEL50013_WRITE_LATENCY_MAX_NS);
    EXPECT_EQ(posMetricVector->at(1).GetGaugeValue(), 7362942);
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("sample_count"), "50");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("array_id"), "3");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("volume_id"), "6");
    EXPECT_EQ(posMetricVector->at(1).GetLabelList()->at("interval"), "0");
    delete posMetricVector;
    air::json_clear();
}

} // namespace pos
