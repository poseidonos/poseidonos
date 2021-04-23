#pragma once

#include <iostream>

#include "fake_global_meta_getter.h"
#include "fake_node_manager.h"
#include "fake_node_meta_getter.h"
#include "fake_thread.h"
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
    FakeThread* fake_perf_thread{nullptr};
    FakeThread* fake_lat_thread{nullptr};
    FakeThread* fake_q_thread{nullptr};
    FakeThread* fake_util_thread{nullptr};
    FakeThread* fake_count_thread{nullptr};
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

        fake_perf_thread = new FakeThread(air::ProcessorType::PERFORMANCE, 32);
        fake_lat_thread = new FakeThread(air::ProcessorType::LATENCY, 32);
        fake_q_thread = new FakeThread(air::ProcessorType::QUEUE, 32);
        fake_util_thread = new FakeThread(air::ProcessorType::UTILIZATION, 32);
        fake_count_thread = new FakeThread(air::ProcessorType::COUNT, 32);
        fake_node_meta_getter = new FakeNodeMetaGetter;
        fake_global_meta_getter = new FakeGlobalMetaGetter;
        fake_node_manager = new FakeNodeManager{fake_global_meta_getter,
            fake_node_meta_getter};
    }
    virtual ~ProcessorTest()
    {
        if (nullptr != fake_perf_thread)
        {
            delete fake_perf_thread;
            fake_perf_thread = nullptr;
        }
        if (nullptr != fake_lat_thread)
        {
            delete fake_lat_thread;
            fake_lat_thread = nullptr;
        }
        if (nullptr != fake_q_thread)
        {
            delete fake_q_thread;
            fake_q_thread = nullptr;
        }
        if (nullptr != fake_util_thread)
        {
            delete fake_util_thread;
            fake_util_thread = nullptr;
        }
        if (nullptr != fake_count_thread)
        {
            delete fake_count_thread;
            fake_count_thread = nullptr;
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
