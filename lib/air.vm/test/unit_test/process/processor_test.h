#pragma once

#include <iostream>

#include "fake_global_meta_getter.h"
#include "fake_node_data.h"
#include "fake_node_manager.h"
#include "fake_node_meta_getter.h"
#include "src/lib/Data.h"
#include "src/lib/Type.h"
#include "src/process/processor/CountProcessor.cpp"
#include "src/process/processor/CountProcessor.h"
#include "src/process/processor/LatencyProcessor.cpp"
#include "src/process/processor/LatencyProcessor.h"
#include "src/process/processor/PerformanceProcessor.cpp"
#include "src/process/processor/PerformanceProcessor.h"
#include "src/process/processor/Processor.cpp"
#include "src/process/processor/Processor.h"
#include "src/process/processor/QueueProcessor.cpp"
#include "src/process/processor/QueueProcessor.h"
#include "src/process/processor/UtilizationProcessor.cpp"
#include "src/process/processor/UtilizationProcessor.h"

class ProcessorTest : public ::testing::Test
{
public:
    process::Processor* perf_processor{nullptr};
    process::Processor* lat_processor{nullptr};
    process::Processor* q_processor{nullptr};
    process::Processor* util_processor{nullptr};
    process::Processor* count_processor{nullptr};
    FakeNodeData* fake_perf_data{nullptr};
    FakeNodeData* fake_lat_data{nullptr};
    FakeNodeData* fake_q_data{nullptr};
    FakeNodeData* fake_util_data{nullptr};
    FakeNodeData* fake_count_data{nullptr};
    FakeNodeMetaGetter* fake_node_meta_getter{nullptr};
    FakeGlobalMetaGetter* fake_global_meta_getter{nullptr};
    FakeNodeManager* fake_node_manager{nullptr};

protected:
    ProcessorTest()
    {
        perf_processor = new process::PerformanceProcessor;
        lat_processor = new process::LatencyProcessor;
        q_processor = new process::QueueProcessor;
        util_processor = new process::UtilizationProcessor;
        count_processor = new process::CountProcessor;

        fake_perf_data = new FakeNodeData(air::ProcessorType::PERFORMANCE, 32, 32);
        fake_lat_data = new FakeNodeData(air::ProcessorType::LATENCY, 32, 32);
        fake_q_data = new FakeNodeData(air::ProcessorType::QUEUE, 32, 32);
        fake_util_data = new FakeNodeData(air::ProcessorType::UTILIZATION, 32, 32);
        fake_count_data = new FakeNodeData(air::ProcessorType::COUNT, 32, 32);
        fake_node_meta_getter = new FakeNodeMetaGetter;
        fake_global_meta_getter = new FakeGlobalMetaGetter;
        fake_node_manager = new FakeNodeManager{fake_global_meta_getter,
            fake_node_meta_getter};
    }
    virtual ~ProcessorTest()
    {
        if (nullptr != fake_perf_data)
        {
            delete fake_perf_data;
            fake_perf_data = nullptr;
        }
        if (nullptr != fake_lat_data)
        {
            delete fake_lat_data;
            fake_lat_data = nullptr;
        }
        if (nullptr != fake_q_data)
        {
            delete fake_q_data;
            fake_q_data = nullptr;
        }
        if (nullptr != fake_util_data)
        {
            delete fake_util_data;
            fake_util_data = nullptr;
        }
        if (nullptr != fake_count_data)
        {
            delete fake_count_data;
            fake_count_data = nullptr;
        }
        if (nullptr != perf_processor)
        {
            delete perf_processor;
            perf_processor = nullptr;
        }
        if (nullptr != lat_processor)
        {
            delete lat_processor;
            lat_processor = nullptr;
        }
        if (nullptr != q_processor)
        {
            delete q_processor;
            q_processor = nullptr;
        }
        if (nullptr != util_processor)
        {
            delete util_processor;
            util_processor = nullptr;
        }
        if (nullptr != count_processor)
        {
            delete count_processor;
            count_processor = nullptr;
        }
        if (nullptr != fake_node_meta_getter)
        {
            delete fake_node_meta_getter;
            fake_node_meta_getter = nullptr;
        }
        if (nullptr != fake_global_meta_getter)
        {
            delete fake_global_meta_getter;
            fake_global_meta_getter = nullptr;
        }
    }
    void
    SetUp() override
    {
    }
    void
    TearDown() override
    {
    }
};
