
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "collection_manager_test.h"
#include "src/collection/CollectionCorHandler.h"
#include "src/collection/CollectionObserver.cpp"
#include "src/collection/CollectionObserver.h"

TEST_F(WriterTest, PerformanceWriter_LogData)
{
    lib::Data* data = new lib::PerformanceData;
    lib::PerformanceData* perf_data =
        static_cast<lib::PerformanceData*>(data);

    performance_writer->LogData(data, 512);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(512, perf_data->bandwidth);

    performance_writer->LogData(data, 1024);
    EXPECT_EQ(2, perf_data->iops);
    EXPECT_EQ(1536, perf_data->bandwidth);

    performance_writer->LogData(data, 2048);
    EXPECT_EQ(3, perf_data->packet_cnt.size());
    EXPECT_EQ(1, perf_data->packet_cnt[2048]);

    for (uint32_t i = 0; i < 100; i++)
    {
        performance_writer->LogData(data, i);
    }

    EXPECT_EQ(lib::MAX_PACKET_CNT_SIZE, perf_data->packet_cnt.size());
}

TEST_F(WriterTest, PerformanceWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccPerformanceData;
    lib::AccPerformanceData* perf_data = static_cast<lib::AccPerformanceData*>(data_dirty);

    unsigned int value{0};
    EXPECT_EQ(perf_data->need_erase, value);
    value = 1;
    performance_writer->InformInit(data_dirty);
    EXPECT_EQ(perf_data->need_erase, value);

    data_dirty = nullptr;
    performance_writer->InformInit(data_dirty);
    EXPECT_EQ(nullptr, data_dirty);
}

TEST_F(WriterTest, PerformanceWriter_SetSamplingRate)
{
    EXPECT_EQ(0, performance_writer->SetSamplingRate(1));
}

TEST_F(WriterTest, LatencyWriter_LogData)
{
    lib::Data* data_dirty = new lib::LatencyData;
    lib::LatencyData* lat_data =
        static_cast<lib::LatencyData*>(data_dirty);

    lat_data->start_state = lib::TimeLogState::RUN;
    lat_data->start_token = 100;
    lat_data->end_token = 50;

    latency_writer->LogData(data_dirty, 1234);

    EXPECT_EQ((int)99, lat_data->start_token);
    EXPECT_EQ((int)50, lat_data->end_token);

    lat_data->start_state = lib::TimeLogState::RUN;
    lat_data->start_token = 1;

    latency_writer->LogData(data_dirty, 1234);
    EXPECT_EQ(0, lat_data->start_token);
    EXPECT_EQ(50, lat_data->end_token);
    EXPECT_EQ(lib::TimeLogState::FULL, lat_data->start_state);

    lat_data->end_state = lib::TimeLogState::RUN;
    latency_writer->LogData(data_dirty, 1234);
    EXPECT_EQ(0, lat_data->start_token);
    EXPECT_EQ(49, lat_data->end_token);

    lat_data->end_token = 1;
    latency_writer->LogData(data_dirty, 1234);
    EXPECT_EQ(0, lat_data->end_token);
    EXPECT_EQ(lib::TimeLogState::FULL, lat_data->end_state);
}

TEST_F(WriterTest, LatencyWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccLatencyData;
    lib::AccLatencyData* lat_data = static_cast<lib::AccLatencyData*>(data_dirty);

    unsigned int value{0};
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

    queue_writer->LogData(data, 10);
    EXPECT_EQ(10, queue_data->sum_depth);
    EXPECT_EQ(1, queue_data->num_req);
}

TEST_F(WriterTest, QueueWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccQueueData;
    lib::AccQueueData* q_data = static_cast<lib::AccQueueData*>(data_dirty);

    uint64_t value{0};
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
    for (int i = 0; i < 8; i++)
    {
        EXPECT_EQ(false, queue_writer->IsSampling(queue_data));
    }
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));

    queue_writer->SetSamplingRate(4);
    EXPECT_EQ(true, queue_writer->IsSampling(queue_data));
    for (int i = 0; i < 3; i++)
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
    for (int i = 0; i < 10; i++)
    {
        EXPECT_GT((uint32_t)901, queue_writer->GetLoggingPoint(queue_data));
    }

    queue_writer->SetSamplingRate(20);
    queue_writer->IsSampling(queue_data);
    for (int i = 0; i < 10; i++)
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

TEST_F(WriterTest, CountWriter_LogData)
{
    lib::Data* data = new lib::CountData;
    lib::CountData* count_data = static_cast<lib::CountData*>(data);

    count_writer->LogData(data, 10);

    int16_t val1 = -10;
    count_writer->LogData(data, val1);
    int32_t val2 = -100;
    count_writer->LogData(data, val2);
    int64_t val3 = -1000;
    count_writer->LogData(data, val3);
    count_writer->LogData(data, -1);
    count_writer->LogData(data, -10000);

    EXPECT_EQ(10, count_data->count_positive);
    EXPECT_EQ(1, count_data->num_req_positive);
    EXPECT_EQ(11111, count_data->count_negative);
    EXPECT_EQ(5, count_data->num_req_negative);
}

TEST_F(WriterTest, CountWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccCountData;
    lib::AccCountData* count_data = static_cast<lib::AccCountData*>(data_dirty);

    EXPECT_EQ(0, count_data->need_erase);
    count_writer->InformInit(data_dirty);
    EXPECT_EQ(1, count_data->need_erase);

    data_dirty = nullptr;
    count_writer->InformInit(data_dirty);
    EXPECT_EQ(nullptr, data_dirty);
}

TEST_F(WriterTest, CountWriter_SetSamplingRate)
{
    EXPECT_EQ(0, count_writer->SetSamplingRate(1));
}

TEST_F(WriterTest, UtilizationWriter_LogData)
{
    lib::Data* data = new lib::UtilizationData;
    lib::UtilizationData* util_data = static_cast<lib::UtilizationData*>(data);

    util_writer->LogData(data, 10);
    EXPECT_EQ(10, util_data->usage);
}

TEST_F(WriterTest, UtilizationWriter_InformInit)
{
    lib::AccData* data_dirty = new lib::AccUtilizationData;
    lib::AccUtilizationData* util_data = static_cast<lib::AccUtilizationData*>(data_dirty);

    EXPECT_EQ(0, util_data->need_erase);
    util_writer->InformInit(data_dirty);
    EXPECT_EQ(1, util_data->need_erase);

    data_dirty = nullptr;
    util_writer->InformInit(data_dirty);
    EXPECT_EQ(nullptr, data_dirty);
}

TEST_F(WriterTest, UtilizationWriter_SetSamplingRate)
{
    EXPECT_EQ(0, util_writer->SetSamplingRate(1));
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

    performance_collector->LogData(data, 512);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(512, perf_data->bandwidth);
}

TEST_F(CollectorTest, PerformanceCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccPerformanceData;
    lib::AccPerformanceData* perf_data = static_cast<lib::AccPerformanceData*>(data_dirty);
    unsigned int value{0};

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

    latency_data->start_state = lib::TimeLogState::RUN;
    latency_data->start_token = 43;
    latency_collector->LogData(data, 1234);

    EXPECT_EQ(42, latency_data->start_token);
}

TEST_F(CollectorTest, LatencyCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccLatencyData;
    lib::AccLatencyData* lat_data = static_cast<lib::AccLatencyData*>(data_dirty);

    uint32_t value{0};
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

    queue_collector->LogData(data, 10);
    EXPECT_EQ(10, queue_data->sum_depth);
    EXPECT_EQ(1, queue_data->num_req);
}

TEST_F(CollectorTest, QueueCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccQueueData;
    lib::AccQueueData* q_data = static_cast<lib::AccQueueData*>(data_dirty);

    uint32_t value{0};
    EXPECT_EQ(value, q_data->need_erase);
    queue_collector->InformInit(data_dirty);
    value = 1;
    EXPECT_EQ(value, q_data->need_erase);
}

TEST_F(CollectorTest, CountCollector_SetSamplingRate)
{
    EXPECT_EQ(0, count_collector->SetSamplingRate(1));
}

TEST_F(CollectorTest, CountCollector_LogData)
{
    lib::Data* data = new lib::CountData;
    lib::CountData* count_data = static_cast<lib::CountData*>(data);

    count_collector->LogData(data, 10);
    count_collector->LogData(data, -1);
    count_collector->LogData(data, -10000);
    count_collector->LogData(data, 0);

    EXPECT_EQ(10, count_data->count_positive);
    EXPECT_EQ(2, count_data->num_req_positive);
    EXPECT_EQ(10001, count_data->count_negative);
    EXPECT_EQ(2, count_data->num_req_negative);
}

TEST_F(CollectorTest, CountCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccCountData;
    lib::AccCountData* count_data = static_cast<lib::AccCountData*>(data_dirty);

    EXPECT_EQ(0, count_data->need_erase);
    count_collector->InformInit(data_dirty);
    EXPECT_EQ(1, count_data->need_erase);

    data_dirty = nullptr;
    count_collector->InformInit(data_dirty);
}

TEST_F(CollectorTest, UtilizationCollector_SetSamplingRate)
{
    EXPECT_EQ(0, util_collector->SetSamplingRate(1));
}

TEST_F(CollectorTest, UtilizationCollector_LogData)
{
    lib::Data* data = new lib::UtilizationData;
    lib::UtilizationData* util_data = static_cast<lib::UtilizationData*>(data);

    util_collector->LogData(data, 131230);
    EXPECT_EQ(131230, util_data->usage);
}

TEST_F(CollectorTest, UtilizationCollector_InformInit)
{
    lib::AccData* data_dirty = new lib::AccUtilizationData;
    lib::AccUtilizationData* util_data = static_cast<lib::AccUtilizationData*>(data_dirty);

    EXPECT_EQ(0, util_data->need_erase);
    util_collector->InformInit(data_dirty);
    EXPECT_EQ(1, util_data->need_erase);

    data_dirty = nullptr;
    util_collector->InformInit(data_dirty);
}

TEST_F(CollectionManagerTest, Subject_Notify)
{
    EXPECT_EQ(-1, collection_subject->Notify(1, 0, 0, 0, 0, 0, 0, 0));
}

TEST_F(CollectionManagerTest, CreateThread)
{
    mock_node_manager->CreateNodeDataArray(0);
    EXPECT_NE(nullptr, mock_node_manager->GetNodeDataArray(0));
    EXPECT_EQ(nullptr, mock_node_manager->GetNodeDataArray(1));
}

TEST_F(CollectionManagerTest, GetThread)
{
    mock_node_manager->CreateNodeDataArray(0);
    EXPECT_NE(nullptr, collection_manager->GetNodeDataArray(0));
    EXPECT_EQ(nullptr, collection_manager->GetNodeDataArray(1));
}

TEST_F(CollectionManagerTest, IsLog)
{
    collection_manager->Init();
    EXPECT_TRUE(collection_manager->IsLog(0)); // performance
}

TEST_F(CollectionManagerTest, LogData)
{
    collection_manager->Init();

    mock_node_manager->CreateNodeDataArray(0);
    node::NodeDataArray* node_data_array = collection_manager->GetNodeDataArray(0);
    node::NodeData* node_data = node_data_array->node[0];

    collection_manager->LogData(0, 0, node_data_array, 0, 128);
    lib::Data* data = node_data->GetUserDataByHashIndex(0, 0);
    lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(128, perf_data->bandwidth);

    collection_manager->LogData(0, 0, node_data_array, 1, 128);
    collection_manager->LogData(0, 0, node_data_array, 2, 256);
    collection_manager->LogData(0, 0, node_data_array, 3, 512);

    data = node_data->GetUserDataByHashIndex(1, 0);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(128, perf_data->bandwidth);
    data = node_data->GetUserDataByHashIndex(2, 0);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(256, perf_data->bandwidth);
    data = node_data->GetUserDataByHashIndex(3, 0);
    EXPECT_EQ(nullptr, data);
}

TEST_F(CollectionManagerTest, UpdateCollection)
{
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_AIR), 1, 0));
    EXPECT_EQ(0, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_AIR), 0, 0));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, to_dtype(pi::Type2::ENABLE_AIR), 99, 0));
    EXPECT_EQ(-1, collection_manager->UpdateCollection(0, 99, 0, 0));

    collection_manager->Init();
    mock_node_manager->Init();

    // init node 0 (perf)
    mock_node_manager->CreateNodeDataArray(1);
    node::NodeDataArray* node_data_array = collection_manager->GetNodeDataArray(1);
    node::NodeData* node_data = node_data_array->node[0];
    collection_manager->LogData(0, 0, node_data_array, 0, 128);
    lib::Data* data = node_data->GetUserDataByHashIndex(0, 0);
    lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(128, perf_data->bandwidth);

    lib::AccData* acc_data = node_data->GetAccData(0, 0);
    lib::AccPerformanceData* acc_perf_data = static_cast<lib::AccPerformanceData*>(acc_data);
    EXPECT_EQ(0, acc_perf_data->need_erase);
    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE), 0, 0);
    EXPECT_EQ(1, acc_perf_data->need_erase);

    // init node 1 (latency)
    collection_manager->UpdateCollection(0, to_dtype(pi::Type2::INITIALIZE_NODE), 1, 0);

    // init node 0-2
    node_data = node_data_array->node[0];
    collection_manager->LogData(0, 0, node_data_array, 0, 128);
    data = node_data->GetUserDataByHashIndex(0, 0);
    perf_data = static_cast<lib::PerformanceData*>(data);
    EXPECT_EQ(2, perf_data->iops);
    EXPECT_EQ(256, perf_data->bandwidth); // not initialized yet

    node_data = node_data_array->node[2];
    collection_manager->LogData(2, 0, node_data_array, 0, 128);
    data = node_data->GetUserDataByHashIndex(0, 0);
    lib::QueueData* queue_data = static_cast<lib::QueueData*>(data);
    EXPECT_EQ(128, queue_data->sum_depth);
    EXPECT_EQ(1, queue_data->num_req);
}

TEST_F(CollectionManagerTest, Observer)
{
    collection::Observer* observer = new collection::Observer{collection_manager};
    collection::CollectionCoRHandler* collection_cor_handler = new collection::CollectionCoRHandler{observer};
    MockOutputObserver* mock_output_observer = new MockOutputObserver{};
    collection_subject->Attach(mock_output_observer, 0);

    // send init msg
    observer->Update(0, to_dtype(pi::Type2::INITIALIZE_NODE), 0, 0, 0, 0, 0);

    mock_node_manager->CreateNodeDataArray(1);
    collection_manager->Init();
    node::NodeDataArray* node_data_array = collection_manager->GetNodeDataArray(1);
    node::NodeData* node_data = node_data_array->node[0];
    collection_manager->LogData(0, 0, node_data_array, 0, 128);
    lib::Data* data = node_data->GetUserDataByHashIndex(0, 0);
    lib::PerformanceData* perf_data = static_cast<lib::PerformanceData*>(data);
    lib::AccData* acc_data = node_data->GetAccData(0, 0);
    lib::AccPerformanceData* acc_perf_data =
        static_cast<lib::AccPerformanceData*>(acc_data);
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(128, perf_data->bandwidth);
    EXPECT_EQ(0, acc_perf_data->need_erase);

    // handle init msg
    collection_cor_handler->HandleRequest();

    // handle error
    EXPECT_EQ(1, perf_data->iops);
    EXPECT_EQ(128, perf_data->bandwidth); // not initialized yet
    EXPECT_EQ(1, acc_perf_data->need_erase);

    observer->Update(0, to_dtype(pi::Type2::SET_SAMPLING_RATE_ALL), 0, 0, 0, 0, 0);
    collection_cor_handler->HandleRequest();

    delete observer;
    delete collection_cor_handler;
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
