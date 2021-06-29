
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include <iostream>

#include "preprocessor_test.h"
#include "process_manager_test.h"
#include "processor_test.h"
#include "src/config/ConfigInterface.h"
#include "src/lib/Protocol.h"
#include "src/lib/StringView.h"
#include "src/lib/Type.h"

TEST_F(ProcessorTest, ThreadAwareStreamData)
{
    int tid = 123;

    air::string_view node_name_perf1 = "PERF_BENCHMARK";
    perf_processor->StreamData(node_name_perf1, tid, "thread0", nullptr, air::ProcessorType::PERFORMANCE, 1, 32, 32);
    air::string_view node_name_perf2 = "PERF_BENCHMARK";
    perf_processor->StreamData(node_name_perf2, tid, "thread0", fake_perf_data, air::ProcessorType::PERFORMANCE, 1, 32, 32);
    air::string_view node_name_perf3 = "PERF_BENCHMARK";
    perf_processor->StreamData(node_name_perf3, tid, "thread0", fake_perf_data, air::ProcessorType::PERFORMANCE, 1, 32, 32);

    // Queue type
    air::string_view node_name_q1 = "Q_SUBMISSION";
    q_processor->StreamData(node_name_q1, tid, "thread0", fake_q_data, air::ProcessorType::QUEUE, 1, 32, 32);
    air::string_view node_name_q2 = "Q_COMPLETION";
    q_processor->StreamData(node_name_q2, tid, "thread0", fake_q_data, air::ProcessorType::QUEUE, 1, 32, 32);
}

TEST_F(ProcessorTest, QueueProcessData)
{
    lib::QueueData* q_data = (lib::QueueData*)fake_q_data->GetAirData(0, 0);
    lib::AccQueueData* acc_q_data = (lib::AccQueueData*)fake_q_data->GetAccData(0, 0);

    q_data->num_req = 12;
    q_data->sum_depth = 120;
    q_data->access = true;
    acc_q_data->need_erase = false;
    air::string_view node_name_q1 = "Q_SUBMISSION";
    q_processor->StreamData(node_name_q1, 0, "thread0", fake_q_data, air::ProcessorType::QUEUE, 1, 32, 32);
    EXPECT_EQ(true, ((10.1f > acc_q_data->depth_total_avg) && (9.9f < acc_q_data->depth_total_avg)));

    q_processor->StreamData(node_name_q1, 0, "thread0", fake_q_data, air::ProcessorType::QUEUE, 1, 32, 32); // for swap

    q_data->num_req = 12;
    q_data->sum_depth = 240;
    q_data->access = true;
    acc_q_data->need_erase = false;
    air::string_view node_name_q2 = "Q_SUBMISSION";
    q_processor->StreamData(node_name_q2, 0, "thread0", fake_q_data, air::ProcessorType::QUEUE, 1, 32, 32);
    EXPECT_EQ(true, ((15.1f > acc_q_data->depth_total_avg) && (14.9f < acc_q_data->depth_total_avg)));
}

TEST_F(ProcessorTest, UtilizationProcessData)
{
    lib::UtilizationData* util_data = (lib::UtilizationData*)fake_util_data->GetAirData(0, 0);
    lib::AccUtilizationData* util_acc = (lib::AccUtilizationData*)fake_util_data->GetAccData(0, 0);

    util_data->usage = 400;
    util_data->access = true;
    air::string_view node_name_util1 = "UTIL_SUBMIT_THR";
    util_processor->StreamData(node_name_util1, 0, "thread0", fake_util_data, air::ProcessorType::UTILIZATION, 1, 32, 32);

    EXPECT_EQ(400, util_acc->total_usage);
}

TEST_F(ProcessorTest, CountProcessData_Case1)
{
    lib::CountData* count_data = (lib::CountData*)fake_count_data->GetAirData(1, 0);
    lib::AccCountData* count_acc = (lib::AccCountData*)fake_count_data->GetAccData(1, 0);

    count_data->access = true;
    count_data->count_positive = 100;
    count_data->num_req_positive = 45;
    count_data->count_negative = 1000;
    count_data->num_req_negative = 23;

    count_acc->negative = 0;
    count_acc->total_count = 0;
    count_acc->total_num_req_positive = 0;
    count_acc->total_num_req_negative = 0;

    air::string_view node_name_count1 = "CNT_TEST_EVENT";
    count_processor->StreamData(node_name_count1, 0, "thread0", fake_count_data, air::ProcessorType::COUNT, 1, 32, 32);

    EXPECT_EQ(1, count_acc->negative);
    EXPECT_EQ(900, count_acc->total_count);
    EXPECT_EQ(45, count_acc->total_num_req_positive);
    EXPECT_EQ(23, count_acc->total_num_req_negative);
}

TEST_F(ProcessorTest, CountProcessData_Case2)
{
    lib::CountData* count_data = (lib::CountData*)fake_count_data->GetAirData(1, 0);
    lib::AccCountData* count_acc = (lib::AccCountData*)fake_count_data->GetAccData(1, 0);

    count_data->access = true;
    count_data->count_positive = 100;
    count_data->num_req_positive = 45;
    count_data->count_negative = 1000;
    count_data->num_req_negative = 23;

    count_acc->negative = 0;
    count_acc->total_count = 10000;
    count_acc->total_num_req_positive = 10;
    count_acc->total_num_req_negative = 30;

    air::string_view node_name_count1 = "CNT_TEST_EVENT";
    count_processor->StreamData(node_name_count1, 0, "thread0", fake_count_data, air::ProcessorType::COUNT, 1, 32, 32);

    EXPECT_EQ(0, count_acc->negative);
    EXPECT_EQ(9100, count_acc->total_count);
    EXPECT_EQ(55, count_acc->total_num_req_positive);
    EXPECT_EQ(53, count_acc->total_num_req_negative);
}

TEST_F(ProcessorTest, CountProcessData_Case3)
{
    lib::CountData* count_data = (lib::CountData*)fake_count_data->GetAirData(1, 0);
    lib::AccCountData* count_acc = (lib::AccCountData*)fake_count_data->GetAccData(1, 0);

    count_data->access = true;
    count_data->count_positive = 100;
    count_data->num_req_positive = 45;
    count_data->count_negative = 0;
    count_data->num_req_negative = 0;

    count_acc->negative = 0;
    count_acc->total_count = 10000;
    count_acc->total_num_req_positive = 10;
    count_acc->total_num_req_negative = 30;

    air::string_view node_name_count1 = "CNT_TEST_EVENT";
    count_processor->StreamData(node_name_count1, 0, "thread0", fake_count_data, air::ProcessorType::COUNT, 1, 32, 32);

    EXPECT_EQ(0, count_acc->negative);
    EXPECT_EQ(10100, count_acc->total_count);
    EXPECT_EQ(55, count_acc->total_num_req_positive);
    EXPECT_EQ(30, count_acc->total_num_req_negative);
}

TEST_F(ProcessorTest, CountProcessData_Case4)
{
    lib::CountData* count_data = (lib::CountData*)fake_count_data->GetAirData(1, 0);
    lib::AccCountData* count_acc = (lib::AccCountData*)fake_count_data->GetAccData(1, 0);

    count_data->access = true;
    count_data->count_positive = 100;
    count_data->num_req_positive = 45;
    count_data->count_negative = 90000;
    count_data->num_req_negative = 323;

    count_acc->negative = 0;
    count_acc->total_count = 10000;
    count_acc->total_num_req_positive = 10;
    count_acc->total_num_req_negative = 30;

    air::string_view node_name_count1 = "CNT_TEST_EVENT";
    count_processor->StreamData(node_name_count1, 0, "thread0", fake_count_data, air::ProcessorType::COUNT, 1, 32, 32);

    EXPECT_EQ(1, count_acc->negative);
    EXPECT_EQ(79900, count_acc->total_count);
    EXPECT_EQ(55, count_acc->total_num_req_positive);
    EXPECT_EQ(353, count_acc->total_num_req_negative);
}

TEST_F(ProcessManagerTest, Init)
{
    EXPECT_EQ((int)cfg::GetSentenceCount(config::ParagraphType::NODE), process_manager->Init());
}

TEST_F(PreprocessorTest, Run)
{
    lib::Data* data = fake_node_manager->nda_map[123]->node[1]->GetUserDataByHashIndex(0, 0);
    lib::LatencyData* lat_data_0 = static_cast<lib::LatencyData*>(data);
    lat_data_0->access = true;
    data = fake_node_manager->nda_map[123]->node[1]->GetUserDataByHashIndex(0, 1);
    lib::LatencyData* lat_data_1 = static_cast<lib::LatencyData*>(data);
    data = fake_node_manager->nda_map[123]->node[1]->GetUserDataByHashIndex(0, 2);
    lib::LatencyData* lat_data_2 = static_cast<lib::LatencyData*>(data);

    timespec ts;
    lat_data_0->start_v.push_back({123, ts});
    lat_data_0->start_v.push_back({124, ts});
    lat_data_0->start_state = lib::TimeLogState::STOP;
    lat_data_0->start_match_count = 0;

    lat_data_1->end_v.push_back({123, ts});
    lat_data_1->end_v.push_back({125, ts});
    lat_data_1->end_state = lib::TimeLogState::STOP;
    lat_data_1->end_match_count = 0;

    lat_data_1->start_state = lib::TimeLogState::RUN;

    lat_data_2->end_state = lib::TimeLogState::RUN;

    preprocessor->Run(0);

    EXPECT_EQ((unsigned int)1, lat_data_0->start_match_count);
    EXPECT_EQ((unsigned int)1, lat_data_1->end_match_count);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
