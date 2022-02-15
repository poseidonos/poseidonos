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

#include "src/telemetry/telemetry_client/telemetry_publisher.h"

#include <gtest/gtest.h>

#include "test/unit-tests/telemetry/telemetry_client/i_global_publisher_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;
namespace pos
{
TEST(TelemetryPublisher, PublishData_TestUpdateData)
{
    // given
    TelemetryPublisher tp;
    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    tp.StartUsingDataPool();
    // given 1.
    tp.StopPublishing();
    // when 1.
    POSMetric m("a", MT_GAUGE);
    m.SetCountValue(1);
    m.SetGaugeValue(2);
    m.AddLabel("1", "tt");
    m.AddLabel("2", "cc");
    m.SetType(MT_GAUGE);
    POSMetricTypes tt = m.GetType();
    std::string temp = m.GetName();
    uint64_t cnt = m.GetCountValue();
    int64_t gg = m.GetGaugeValue();

    POSMetricValue v;
    v.gauge = 1;
    // given 1.
    int ret = tp.PublishData("cc", v, MT_GAUGE);
    // given 2.
    tp.StartPublishing();
    // when 2.
    ret = tp.PublishData("cc", v, MT_GAUGE);
    // when 3.
    ret = tp.PublishMetric(m);
    delete igp;
}

TEST(TelemetryPublisher, PublishData_TestExceedEntryLimit)
{
    // given
    TelemetryPublisher tp("aaa");
    tp.SetMaxEntryLimit(2);
    tp.StartPublishing();
    tp.StartUsingDataPool();
    std::string nnn = tp.GetName();

    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    POSMetric m("a", MT_GAUGE);
    m.SetCountValue(1);
    m.SetGaugeValue(2);
    m.AddLabel("1", "tt");
    m.AddLabel("2", "cc");
    m.SetType(MT_GAUGE);
    POSMetricTypes tt = m.GetType();
    std::string temp = m.GetName();
    uint64_t cnt = m.GetCountValue();
    int64_t gg = m.GetGaugeValue();

    POSMetricValue v;
    v.gauge = 1;
    // given 1.
    int ret = tp.PublishData("cc", v, MT_GAUGE);
    v.count = 0;
    // when 1.
    ret = tp.PublishData("cc", v, MT_COUNT);
    // when 2.
    ret = tp.PublishMetric(m);
    // when 3.
    std::vector<POSMetric>* list = tp.AllocatePOSMetricVector();
    list->push_back(m);
    m.SetName("aaa");
    list->push_back(m);
    // when 4.
    ret = tp.PublishMetricList(list);

    // USAGE SAMPLE FOR PUBLISH METRIC LIST
    POSMetricVector* metricList = tp.AllocatePOSMetricVector();
    for (int i = 0; i < 5; i++)
    {
        std::string name = "key_" + to_string(i);
        POSMetric metric(name, MT_COUNT);
        metric.SetCountValue(i);
        metric.AddLabel("label0", "k1"); // optional
        metric.AddLabel("label1", "k2"); // optional
        metricList->push_back(metric);
    }
    tp.PublishMetricList(metricList);
    ////////////////////////////////////////
    tp.StopUsingDataPool();
    delete igp;
}

TEST(TelemetryPublisher, PublishData_TestHistogramValue)
{
    // given
    TelemetryPublisher tp;
    NiceMock<MockIGlobalPublisher>* igp = new NiceMock<MockIGlobalPublisher>();
    tp.SetGlobalPublisher(igp);
    tp.StartUsingDataPool();
    tp.StopPublishing();

    POSHistogramValue histValue{std::vector<int64_t>{0, 4096, 8192, 16384, 32768, 1048576}};

    POSMetric m("transfer_size", MT_HISTOGRAM);
    m.SetHistogramValue(&histValue);
    m.AddLabel("label1", "label1_1");
    m.AddLabel("label2", "label2_2");
    m.SetType(MT_HISTOGRAM);

    m.GetHistogramValue()->Observe(512);
    ASSERT_EQ(m.GetHistogramValue()->GetSum(), 512);
    ASSERT_EQ(m.GetHistogramValue()->GetTotalCount(), 1);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[0], 0);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[1], 1);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[2], 1);

    m.GetHistogramValue()->Observe(4096);
    ASSERT_EQ(m.GetHistogramValue()->GetSum(), 4608);
    ASSERT_EQ(m.GetHistogramValue()->GetTotalCount(), 2);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[0], 0);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[1], 2);

    m.GetHistogramValue()->Observe(6144);
    ASSERT_EQ(m.GetHistogramValue()->GetSum(), 10752);
    ASSERT_EQ(m.GetHistogramValue()->GetTotalCount(), 3);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[0], 0);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[1], 2);
    ASSERT_EQ(m.GetHistogramValue()->GetBucketCount()[2], 3);

    tp.StartPublishing();
    int ret = tp.PublishMetric(m);
    delete igp;
}

} // namespace pos
