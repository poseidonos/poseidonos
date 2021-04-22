#pragma once

#include <iostream>

#include "fake_global_meta_getter.h"
#include "fake_node_manager.h"
#include "fake_node_meta_getter.h"
#include "fake_thread.h"
#include "src/lib/Data.h"
#include "src/lib/Type.h"
#include "src/process/Processor.cpp"
#include "src/process/Processor.h"

class ProcessorTest : public ::testing::Test
{
public:
    process::Processor* perf_processor{nullptr};
    process::Processor* lat_processor{nullptr};
    process::Processor* q_processor{nullptr};
    FakeThread* fake_perf_thread{nullptr};
    FakeThread* fake_lat_thread{nullptr};
    FakeThread* fake_q_thread{nullptr};
    FakeNodeMetaGetter* fake_node_meta_getter{nullptr};
    FakeGlobalMetaGetter* fake_global_meta_getter{nullptr};
    FakeNodeManager* fake_node_manager{nullptr};

protected:
    ProcessorTest()
    {
        perf_processor = new process::PerformanceProcessor;
        lat_processor = new process::LatencyProcessor;
        q_processor = new process::QueueProcessor;

        fake_perf_thread = new FakeThread(air::ProcessorType::PERFORMANCE, 32);
        fake_lat_thread = new FakeThread(air::ProcessorType::LATENCY, 32);
        fake_q_thread = new FakeThread(air::ProcessorType::QUEUE, 32);
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
