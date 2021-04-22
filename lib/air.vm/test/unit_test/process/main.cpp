
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "preprocessor_test.h"
#include "process_manager_test.h"
#include "processor_test.h"
#include "src/config/ConfigInterface.h"
#include "src/config/ConfigParser.cpp"
#include "src/lib/Protocol.h"
#include "src/lib/Type.h"

TEST_F(ProcessorTest, IsIdle)
{
    lib::Data data;
    perf_processor->IsIdle(&data);
    perf_processor->IsIdle(&data);
    perf_processor->IsIdle(&data);
    perf_processor->IsIdle(&data);
    perf_processor->IsIdle(&data);
}

TEST_F(ProcessorTest, ThreadAwareStreamData)
{
    int tid = 123;

    EXPECT_EQ(false, perf_processor->StreamData("PERF_TEST1", tid, "thread0", nullptr, air::ProcessorType::PERFORMANCE, 1, 32));
    EXPECT_EQ(true, perf_processor->StreamData("PERF_TEST2", tid, "thread0", fake_perf_thread, air::ProcessorType::PERFORMANCE, 1, 32));
    fake_perf_thread->SetIsLogging(false);
    EXPECT_EQ(false, perf_processor->StreamData("PERF_TEST3", tid, "thread0", fake_perf_thread, air::ProcessorType::PERFORMANCE, 1, 32));
    fake_perf_thread->SetIsLogging(true);

    // Performance type
    EXPECT_EQ(true, perf_processor->StreamData("PERF_TEST4", tid, "thread0", fake_perf_thread, air::ProcessorType::PERFORMANCE, 1, 32));
    fake_perf_thread->SetEnable();
    EXPECT_EQ(true, perf_processor->StreamData("PERF_TEST5", tid, "thread0", fake_perf_thread, air::ProcessorType::PERFORMANCE, 2, 32));
    fake_perf_thread->SetIdle();
    fake_perf_thread->SetEnable();
    EXPECT_EQ(true, perf_processor->StreamData("PERF_TEST6", tid, "thread0", fake_perf_thread, air::ProcessorType::PERFORMANCE, 2, 32));

    // Latency type
    EXPECT_EQ(false, lat_processor->StreamData("LAT_TEST1", nullptr, 0));
    EXPECT_EQ(false, lat_processor->StreamData("LAT_TEST2", fake_node_manager->GetAccLatData(0, 0), 0));
    lib::AccLatencySeqData* seq_data = &(fake_node_manager->GetAccLatData(0, 0)->seq_data[0]);

    seq_data->sample_count = 0;
    EXPECT_EQ(false, lat_processor->StreamData("LAT_TEST3", fake_node_manager->GetAccLatData(0, 0), 0));

    seq_data->sample_count = 100;
    EXPECT_EQ(true, lat_processor->StreamData("LAT_TEST4", fake_node_manager->GetAccLatData(0, 0), 0));

    seq_data->sample_count = 100;
    seq_data->mean = 2;
    seq_data->min = 1;
    seq_data->max = 3;
    lib::AccLatencySeqDataBucket* lat_bucket = new lib::AccLatencySeqDataBucket;
    seq_data->bucket.push_back(lat_bucket);
    seq_data->bucket_count++;
    seq_data->bucket[0]->time_lag_size = 3;
    fake_node_manager->GetAccLatData(0, 0)->need_erase = 1;
    EXPECT_EQ(true, lat_processor->StreamData("LAT_TEST5", fake_node_manager->GetAccLatData(0, 0), 0));

    seq_data->total_sample_count = 0x7FFFFFFFFFFFFFFF;
    seq_data->sample_count = 3333;
    EXPECT_EQ(true, lat_processor->StreamData("LAT_TEST6", fake_node_manager->GetAccLatData(0, 0), 0));

    // Queue type
    EXPECT_EQ(true, q_processor->StreamData("Q_TEST1", tid, "thread0", fake_q_thread, air::ProcessorType::QUEUE, 1, 32));
    fake_q_thread->SetIdle();
    EXPECT_EQ(true, q_processor->StreamData("Q_TEST2", tid, "thread0", fake_q_thread, air::ProcessorType::QUEUE, 1, 32));
}

TEST_F(ProcessorTest, QueueProcessData)
{
    lib::QueueData* q_data = (lib::QueueData*)fake_q_thread->GetAirData(0);
    lib::AccQueueData* acc_q_data = (lib::AccQueueData*)fake_q_thread->GetAccData(0);

    q_data->num_req = 12;
    q_data->sum_depth = 120;
    q_data->access = true;
    acc_q_data->need_erase = false;
    q_processor->StreamData("Q_TEST1", 0, "thread0", fake_q_thread,
        air::ProcessorType::QUEUE, 1, 32);
    EXPECT_EQ(true, ((10.1f > acc_q_data->depth_total_avg) && (9.9f < acc_q_data->depth_total_avg)));

    q_data->num_req = 12;
    q_data->sum_depth = 240;
    q_data->access = true;
    acc_q_data->need_erase = false;
    q_processor->StreamData("Q_TEST2", 0, "thread0", fake_q_thread,
        air::ProcessorType::QUEUE, 1, 32);
    EXPECT_EQ(true, ((15.1f > acc_q_data->depth_total_avg) && (14.9f < acc_q_data->depth_total_avg)));
}

TEST_F(ProcessManagerTest, Init)
{
    EXPECT_EQ((int)cfg::GetArrSize(config::ConfigType::NODE), process_manager->Init());
}

TEST_F(ProcessManagerTest, StreamData)
{
    int num_aid = 32;
    // request json_stringify module
    // to write profiling result to json file
    process_manager->Init();

    lib::AccLatencySeqData* seq_data = &(fake_node_manager->GetAccLatData(0, 0)->seq_data[0]);
    seq_data->total_sample_count = 100;

    process_manager->StreamData();
    EXPECT_EQ(num_aid, fake_node_manager->GetNumAid());
}

TEST_F(ProcessManagerTest, SetTimeSlot)
{
    process_manager->Init();

    lib::Data* data = fake_node_manager->thread_map[123].node[1]->GetUserDataByAidIndex(0);
    lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(data);
    lat_data->access = true;
    lib::LatencySeqData* seq_data_0 = &(lat_data->seq_data[0]);
    lib::LatencySeqData* seq_data_1 = &(lat_data->seq_data[1]);

    seq_data_0->start_match_count = 150;
    seq_data_0->start_size = 200;

    seq_data_1->end_size = 400;
    seq_data_1->end_match_count = 350;

    process_manager->SetTimeSlot();
}

TEST_F(PreprocessorTest, Run)
{
    lib::Data* data = fake_node_manager->thread_map[123].node[1]->GetUserDataByAidIndex(0);
    lib::LatencyData* lat_data = static_cast<lib::LatencyData*>(data);
    lat_data->access = true;
    lib::LatencySeqData* seq_data_0 = &(lat_data->seq_data[0]);
    lib::LatencySeqData* seq_data_1 = &(lat_data->seq_data[1]);
    lib::LatencySeqData* seq_data_2 = &(lat_data->seq_data[2]);

    timespec ts;
    seq_data_0->start_v.push_back({123, ts});
    seq_data_0->start_v.push_back({124, ts});
    seq_data_0->start_state = lib::TimeLogState::STOP;
    seq_data_0->start_match_count = 0;

    seq_data_1->end_v.push_back({123, ts});
    seq_data_1->end_v.push_back({125, ts});
    seq_data_1->end_state = lib::TimeLogState::STOP;
    seq_data_1->end_match_count = 0;

    seq_data_1->start_state = lib::TimeLogState::RUN;

    seq_data_2->end_state = lib::TimeLogState::RUN;

    preprocessor->Run(0);

    EXPECT_EQ((unsigned int)1, seq_data_0->start_match_count);
    EXPECT_EQ((unsigned int)1, seq_data_1->end_match_count);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
