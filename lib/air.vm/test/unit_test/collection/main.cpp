
#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "collection_manager_test.h"
#include "src/collection/CollectionObserver.h"
#include "src/collection/CollectionObserver.cpp"
#include "src/collection/CollectionCorHandler.h"
#include "src/config/ConfigParser.cpp"

TEST_F(WriterTest, PerformanceWriter_LogData)
{
    lib::Data* data = new lib::PerformanceData;
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(data);

    performance_writer->LogData(data, AIR_READ, 512);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)512, perf_data->bandwidth_read);

    performance_writer->LogData(data, AIR_WRITE, 1024);
    EXPECT_EQ((unsigned int)1, perf_data->iops_write);
    EXPECT_EQ((unsigned int)1024, perf_data->bandwidth_write);

    performance_writer->LogData(data, 999, 1024);
    EXPECT_EQ((unsigned int)1, perf_data->iops_write);
    EXPECT_EQ((unsigned int)1024, perf_data->bandwidth_write);

    performance_writer->LogData(data, AIR_WRITE, 2048);

    EXPECT_EQ((unsigned int)3, perf_data->packet_cnt.size());
    EXPECT_EQ((unsigned int)1, perf_data->packet_cnt[2048]);

    for (uint32_t i = 0; i < 100; i++)
    {
        performance_writer->LogData(data, AIR_WRITE, i);
    }

    EXPECT_EQ(lib::MAX_PACKET_CNT_SIZE, perf_data->packet_cnt.size());
}

TEST_F(WriterTest, PerformanceWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccPerformanceData;
    lib::AccPerformanceData* perf_data = static_cast<lib::AccPerformanceData*>(data_dirty);

    unsigned int value {0};
    EXPECT_EQ(perf_data->need_erase, value);
    value = 1;
    performance_writer->InformInit(data_dirty);
    EXPECT_EQ(perf_data->need_erase, value);

    data_dirty = nullptr;
    performance_writer->InformInit(data_dirty);
    EXPECT_EQ(nullptr, data_dirty);
}

TEST_F(WriterTest, LatencyWriter_LogData)
{
    lib::Data* data_dirty = new lib::LatencyData;
    lib::LatencyData* lat_data =
        static_cast<lib::LatencyData*>(data_dirty);

    lat_data->seq_data[0].start_state = lib::TimeLogState::RUN;
    lat_data->seq_data[0].start_token = 100;
    lat_data->seq_data[0].end_token = 50;

    latency_writer->LogData(data_dirty, 0, 1234);

    EXPECT_EQ((int)99, lat_data->seq_data[0].start_token);
    EXPECT_EQ((int)50, lat_data->seq_data[0].end_token);

    lat_data->seq_data[0].start_state = lib::TimeLogState::RUN;
    lat_data->seq_data[0].start_token = 0;
    lat_data->seq_data[0].end_token = 0;

    latency_writer->LogData(data_dirty, 0, 1234);

    lat_data->seq_data[0].start_state = lib::TimeLogState::IDLE;
    latency_writer->LogData(data_dirty, 0, 1234);

    latency_writer->LogData(data_dirty, 20, 1234);
    lat_data->seq_data[0].end_state = lib::TimeLogState::RUN;
    latency_writer->LogData(data_dirty, 0, 1234);
}

TEST_F(WriterTest, LatencyWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccLatencyData;
    lib::AccLatencyData* lat_data = static_cast<lib::AccLatencyData*>(data_dirty);

    unsigned int value {0};
    EXPECT_EQ(lat_data->need_erase, value);
    value = 1;
    latency_writer->InformInit(data_dirty);
    EXPECT_EQ(lat_data->need_erase, value);

    data_dirty = nullptr;
    latency_writer->InformInit(data_dirty);
    EXPECT_EQ(nullptr, data_dirty);
}

TEST_F(WriterTest, QueueWriter_LogData)
{
    lib::Data* data = new lib::QueueData;
    lib::QueueData* queue_data =
        static_cast<lib::QueueData*>(data);

    queue_writer->LogData(data, 10, 128);
    EXPECT_EQ((unsigned int)10, queue_data->sum_depth);
    EXPECT_EQ((unsigned int)128, queue_data->q_size);
    EXPECT_EQ((unsigned int)1, queue_data->num_req);
}

TEST_F(WriterTest, QueueWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccQueueData;
    lib::AccQueueData* q_data = static_cast<lib::AccQueueData*>(data_dirty);

    uint64_t value {0};
    EXPECT_EQ(q_data->need_erase, value);
    queue_writer->InformInit(data_dirty);
    value = 1;
    EXPECT_EQ(q_data->need_erase, value);

    data_dirty = nullptr;
    queue_writer->InformInit(data_dirty);
    EXPECT_EQ(nullptr, data_dirty);
}

TEST_F(WriterTest, QueueWriter_IsSampling)
{
    lib::Data* data = new lib::QueueData;
    lib::QueueData* queue_data =
        static_cast<lib::QueueData*>(data);

    queue_writer->SetSamplingRate(9);
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));
    for (int i = 0 ; i < 8 ; i++)
    {
        EXPECT_EQ(false, queue_writer->IsSampling(queue_data));
    }
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));

    queue_writer->SetSamplingRate(4);
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));
    for (int i = 0 ; i < 3 ; i++)
    {
        EXPECT_EQ(false, queue_writer->IsSampling(queue_data));
    }
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));

    queue_writer->SetSamplingRate(1);
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));


    queue_writer->SetSamplingRate(1000);
    queue_writer->IsSampling(queue_data);
    for (int i = 0 ; i < 10 ; i++)
    {
        EXPECT_GT((uint32_t)901, queue_writer->GetLoggingPoint(queue_data));
    }

    queue_writer->SetSamplingRate(20);
    queue_writer->IsSampling(queue_data);
    for (int i = 0 ; i < 10 ; i++)
    {
        EXPECT_GT((uint32_t)19, queue_writer->GetLoggingPoint(queue_data));
    }
}

TEST_F(WriterTest, QueueWriter_SetSamplingRate)
{
    EXPECT_EQ(0, queue_writer->SetSamplingRate(1));
    EXPECT_EQ(0, queue_writer->SetSamplingRate(1000));
    EXPECT_EQ(0, queue_writer->SetSamplingRate(10000));
    EXPECT_EQ(-2, queue_writer->SetSamplingRate(0));
    EXPECT_EQ(-2, queue_writer->SetSamplingRate(10001));
}

TEST_F(WriterTest, PerformanceWriter_SetSamplingRate)
{
    EXPECT_EQ(0, performance_writer->SetSamplingRate(1));
}

TEST_F(CollectorTest, PerformanceCollector_SetSamplingRate)
{
    EXPECT_EQ(0, performance_collector->SetSamplingRate(1));
}

TEST_F(CollectorTest, PerformanceCollector_LogData)
{
    lib::Data* data = new lib::PerformanceData;
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(data);

    performance_collector->LogData(data, AIR_READ, 512);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)512, perf_data->bandwidth_read);
}

TEST_F(CollectorTest, PerformanceCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccPerformanceData;
    lib::AccPerformanceData* perf_data = static_cast<lib::AccPerformanceData*>(data_dirty);
    unsigned int value {0};

    EXPECT_EQ(value, perf_data->need_erase);
    performance_collector->InformInit(data_dirty);
    value = 1;
    EXPECT_EQ(value, perf_data->need_erase);

    data_dirty = nullptr;
    performance_collector->InformInit(data_dirty);
}

TEST_F(CollectorTest, LatencyCollector_SetSamplingRate)
{
    EXPECT_EQ(0, latency_collector->SetSamplingRate(1));
}

TEST_F(CollectorTest, LeatencyCollector_LogData)
{
    lib::Data* data = new lib::LatencyData;
    lib::LatencyData* latency_data =
        static_cast<lib::LatencyData*>(data);

    latency_data->seq_data[0].start_state = lib::TimeLogState::RUN;
    latency_data->seq_data[0].start_token = 43;
    latency_collector->LogData(data, 0, 1234);

    int value {42};
    EXPECT_EQ(value, latency_data->seq_data[0].start_token);
}

TEST_F(CollectorTest, LatencyCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccLatencyData;
    lib::AccLatencyData* lat_data = static_cast<lib::AccLatencyData*>(data_dirty);

    uint32_t value {0};
    EXPECT_EQ(value, lat_data->need_erase);
    latency_collector->InformInit(data_dirty);
    value = 1;
    EXPECT_EQ(value, lat_data->need_erase);
}

TEST_F(CollectorTest, QueueCollector_SetSamplingRate)
{
    EXPECT_EQ(0, queue_collector->SetSamplingRate(1));
}

TEST_F(CollectorTest, QueueCollector_LogData)
{
    lib::Data* data = new lib::QueueData;
    lib::QueueData* queue_data =
        static_cast<lib::QueueData*>(data);

    queue_collector->LogData(data, 10, 128);
    EXPECT_EQ((unsigned int)10, queue_data->sum_depth);
    EXPECT_EQ((unsigned int)128, queue_data->q_size);
    EXPECT_EQ((unsigned int)1, queue_data->num_req);
}

TEST_F(CollectorTest, QueueCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccQueueData;
    lib::AccQueueData* q_data = static_cast<lib::AccQueueData*>(data_dirty);

    uint32_t value {0};
    EXPECT_EQ(value, q_data->need_erase);
    queue_collector->InformInit(data_dirty);
    value = 1;
    EXPECT_EQ(value, q_data->need_erase);
}

TEST_F(CollectionManagerTest, Subject_Notify)
{
    EXPECT_EQ(-1, collection_subject->Notify(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(CollectionManagerTest, CreateThread)
{
    EXPECT_EQ(1, collection_manager->CreateThread(0));
}

TEST_F(CollectionManagerTest, GetThread)
{
    EXPECT_EQ(1, collection_manager->CreateThread(0));
    EXPECT_NE(nullptr, collection_manager->GetThread(0));
    EXPECT_EQ(nullptr, collection_manager->GetThread(1));
}

TEST_F(CollectionManagerTest, IsLog)
{
    collection_manager->Init();
    EXPECT_TRUE(collection_manager->IsLog(0)); // performance
}

TEST_F(CollectionManagerTest, LogData)
{
    collection_manager->LogData(0, 101, nullptr, 0, 0);
    collection_manager->Init();

    collection_manager->CreateThread(0);
    node::ThreadArray* thread_array = collection_manager->GetThread(0);
    node::Thread* thr = thread_array->node[0];
    
    collection_manager->LogData(0, 0, thread_array, AIR_READ, 128);
    
    lib::Data* data = thr->GetUserDataByAidValue(0);
    lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)128, perf_data->bandwidth_read);

    collection_manager->LogData(0, 1, thread_array, AIR_READ, 128);
    collection_manager->LogData(0, 2, thread_array, AIR_READ, 256);
    collection_manager->LogData(0, 3, thread_array, AIR_READ, 512);

    data = thr->GetUserDataByAidValue(1);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)128, perf_data->bandwidth_read);
    data = thr->GetUserDataByAidValue(2);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)256, perf_data->bandwidth_read);
    data = thr->GetUserDataByAidValue(3);
    EXPECT_EQ(nullptr, data);

    collection_manager->LogData(3, 0, thread_array, AIR_READ, 4096);
}

TEST_F(CollectionManagerTest, UpdateCollection)
{
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_AIR), 1, 0));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_AIR), 0, 0));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_AIR), 99, 0));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, 99, 0, 0));
    
    collection_manager->Init();

    // init node 0 (perf)
    collection_manager->CreateThread(1);
    node::ThreadArray* thread_array = collection_manager->GetThread(1);
    node::Thread* thr = thread_array->node[0];
    collection_manager->LogData(0, 0, thread_array, AIR_READ, 128);
    lib::Data *data = thr->GetUserDataByAidValue(0);
    lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)128, perf_data->bandwidth_read);

    lib::AccData *acc_data = thr->GetAccData(0);
    lib::AccPerformanceData* acc_perf_data 
                    = static_cast<lib::AccPerformanceData*>(acc_data);
    EXPECT_EQ((unsigned int)0, acc_perf_data->need_erase);
    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE), 0, 0);
    EXPECT_EQ((unsigned int)1, acc_perf_data->need_erase);
    
    // init node 1 (latency)
    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE), 1, 0);
    
    // init node 0-2
    thr = thread_array->node[0];
    collection_manager->LogData(0, 0, thread_array, AIR_READ, 128);
    data = thr->GetUserDataByAidValue(0);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ((unsigned int)2, perf_data->iops_read);
    EXPECT_EQ((unsigned int)256, perf_data->bandwidth_read); // not initialized yet
    
    thr = thread_array->node[2];
    collection_manager->LogData(2, 0, thread_array, 10, 128);
    data = thr->GetUserDataByAidValue(0);
    lib::QueueData* queue_data = static_cast<lib::QueueData*>(data);
    EXPECT_EQ((unsigned int)10, queue_data->sum_depth);
    EXPECT_EQ((unsigned int)128, queue_data->q_size);
    EXPECT_EQ((unsigned int)1, queue_data->num_req);
    /*
    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE_WITH_RANGE), 0x00000002, 0);
    acc_data = thr->GetAccData(0);
    lib::AccQueueData* acc_queue_data 
                    = static_cast<lib::AccQueueData*>(acc_data);
    EXPECT_EQ((unsigned int)1, acc_perf_data->need_erase);
    EXPECT_EQ((unsigned int)1, acc_queue_data->need_erase);
    
    // init group 0
    thr = thread_array->node[5];
    collection_manager->LogData(5, 0, thread_array, 10, 128);
    data = thr->GetUserDataByAidValue(0);
    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE_WITH_GROUP), 0x00000000, 0);
    acc_data = thr->GetAccData(0);
    EXPECT_EQ((unsigned int)1, acc_queue_data->need_erase);
    
    // init node all
    thr = thread_array->node[0];
    collection_manager->LogData(0, 0, thread_array, AIR_READ, 128);
    data = thr->GetUserDataByAidValue(0);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ((unsigned int)3, perf_data->iops_read);
    EXPECT_EQ((unsigned int)384, perf_data->bandwidth_read); // not initialized yet

    thr = thread_array->node[2];
    collection_manager->LogData(2, 0, thread_array, 10, 128);
    data = thr->GetUserDataByAidValue(0);
    queue_data = static_cast<lib::QueueData*>(data);
    EXPECT_EQ((unsigned int)10, queue_data->sum_depth);
    EXPECT_EQ((unsigned int)128, queue_data->q_size);
    EXPECT_EQ((unsigned int)1, queue_data->num_req); // not initialized yet

    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE_ALL), 0, 0);

    // update enable node
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE), to_dtype(pi::OnOff::OFF), 0));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE), to_dtype(pi::OnOff::ON), 0));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE), 99, 0));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE), to_dtype(pi::OnOff::OFF), 0x00000001));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE), to_dtype(pi::OnOff::ON), 0x00000001));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_WITH_RANGE), 99, 0x00000001));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP), to_dtype(pi::OnOff::ON), 0x00000001));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_WITH_GROUP), 99, 0x00000001));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_ALL), to_dtype(pi::OnOff::OFF), 0));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_NODE_ALL), to_dtype(pi::OnOff::ON), 0));

    // set sample rate
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE), 1, 0)); // perf
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE), 0, 1)); // latency
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE), 1, 1)); // latency
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE), 1, 0x00000001)); // 0-1
    EXPECT_EQ(-2, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_RANGE), 0, 0x00000002)); // 0-1
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP), 1, 0x00000000));
    EXPECT_EQ(-2, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_WITH_GROUP), 0, 0x00000000));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL), 1, 0)); // all
    EXPECT_EQ(-2, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL), 0, 0)); // all
    */
}

TEST_F(CollectionManagerTest, Observer)
{
    collection::Observer* observer = new collection::Observer { collection_manager };
    collection::CollectionCoRHandler* collection_cor_handler = new collection::CollectionCoRHandler { observer };
    MockOutputObserver* mock_output_observer = new MockOutputObserver {};
    collection_subject->Attach(mock_output_observer, 0);
    
    // send init msg
    observer->Update(0, to_dtype(pi::Type2::INITIALIZE_NODE), 0, 0, 0, 0, 0);

    collection_manager->Init();
    collection_manager->CreateThread(1);
    node::ThreadArray* thread_array = collection_manager->GetThread(1);
    node::Thread* thr = thread_array->node[0];
    collection_manager->LogData(0, 0, thread_array, AIR_READ, 128);
    lib::Data *data = thr->GetUserDataByAidValue(0);
    lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
    lib::AccData * acc_data = thr->GetAccData(0);
    lib::AccPerformanceData* acc_perf_data = 
                            static_cast<lib::AccPerformanceData*>(acc_data);
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)128, perf_data->bandwidth_read);
    EXPECT_EQ((unsigned int)0, acc_perf_data->need_erase);

    // handle init msg
    collection_cor_handler->HandleRequest();

    // handle error
    EXPECT_EQ((unsigned int)1, perf_data->iops_read);
    EXPECT_EQ((unsigned int)128, perf_data->bandwidth_read); // not initialized yet
    EXPECT_EQ((unsigned int)1, acc_perf_data->need_erase);

    observer->Update(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL), 0, 0, 0, 0, 0);
    collection_cor_handler->HandleRequest();

    delete observer;
    delete collection_cor_handler;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
