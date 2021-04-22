
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "global_meta_test.h"
#include "node_meta_test.h"
#include "src/config/ConfigInterface.h"
#include "src/config/ConfigParser.cpp"

TEST_F(NodeMetaTest, NodeMeta)
{
    uint32_t value;

    // node size
    value = sizeof(struct air::Node);
    EXPECT_EQ(node_meta_getter->NodeSize(), value);

    // node processor type
    air::ProcessorType processor_type;
    processor_type = air::ProcessorType::PROCESSORTYPE_NULL;
    EXPECT_EQ(node_meta_getter->NodeProcessorType(0), processor_type); //default
    EXPECT_EQ(node_meta_getter->NodeProcessorType(1), processor_type); //default

    processor_type = air::ProcessorType::PERFORMANCE;
    node_meta->SetNodeProcessorType(0, air::ProcessorType::PERFORMANCE);
    EXPECT_EQ(node_meta_getter->NodeProcessorType(0), processor_type);

    processor_type = air::ProcessorType::LATENCY;
    node_meta->SetNodeProcessorType(1, air::ProcessorType::LATENCY);
    EXPECT_EQ(node_meta_getter->NodeProcessorType(1), processor_type);

    // run
    EXPECT_EQ(node_meta_getter->Run(), true);     //default
    EXPECT_EQ(node_meta_getter->Update(), false); //default

    node_meta->SetRun(false);
    EXPECT_EQ(node_meta_getter->Run(), false);
    EXPECT_EQ(node_meta_getter->Update(), true);

    node_meta->SetUpdate(false);
    EXPECT_EQ(node_meta_getter->Update(), false);

    // node enable
    EXPECT_EQ(node_meta_getter->NodeEnable(0), false); //default
    EXPECT_EQ(node_meta_getter->Update(), false);      //default

    node_meta->SetNodeEnable(0, true);
    EXPECT_EQ(node_meta_getter->NodeEnable(0), true);
    EXPECT_EQ(node_meta_getter->Update(), true);

    node_meta->SetUpdate(false);
    EXPECT_EQ(node_meta_getter->Update(), false);

    // sampling ratio
    value = 1000;
    EXPECT_EQ(node_meta_getter->NodeSampleRatio(0), value); //default

    value = 100;
    node_meta->SetNodeSampleRatio(0, value);
    EXPECT_EQ(node_meta_getter->NodeSampleRatio(0), value);

    uint32_t node_id = cfg::GetIndex(config::ConfigType::NODE, "PERF_BENCHMARK");
    node_meta->SetNodeChildId(0, 0, node_id);
    EXPECT_EQ(node_meta_getter->NodeChildId(0, 0), node_id);

    // group id
    uint32_t group_id = cfg::GetIndex(config::ConfigType::GROUP, "SUBMIT");
    node_meta->SetNodeGroupId(0, group_id);
    EXPECT_EQ((uint32_t)node_meta_getter->NodeGroupId(0), group_id);

    // meta, check values set above
    air::Node* node = (air::Node*)node_meta_getter->Meta();
    EXPECT_EQ(node[0].processor_type, air::ProcessorType::PERFORMANCE);
    EXPECT_EQ(node[1].processor_type, air::ProcessorType::LATENCY);
    EXPECT_EQ(node[0].sample_ratio, (uint32_t)100);
    EXPECT_EQ(node[0].child_id[0], (uint32_t)cfg::GetIndex(config::ConfigType::NODE, "PERF_BENCHMARK"));
    EXPECT_EQ(node[0].group_id, cfg::GetIndex(config::ConfigType::GROUP, "SUBMIT"));
}

TEST_F(GlobalMetaTest, GlobalMeta)
{
    // enable
    EXPECT_EQ(global_meta_getter->Enable(), true); //default

    global_meta->SetEnable(false);
    EXPECT_EQ(global_meta_getter->Enable(), false);

    // streaming interval
    uint32_t value;
    value = 1;
    EXPECT_EQ(global_meta_getter->StreamingInterval(), value); //default
    EXPECT_EQ(global_meta_getter->StreamingUpdate(), false);   //default

    value = 3;
    global_meta->SetStreamingInterval(value);
    EXPECT_EQ(global_meta_getter->StreamingInterval(), (uint32_t)1);
    EXPECT_EQ(global_meta_getter->StreamingUpdate(), true);
    EXPECT_EQ(global_meta_getter->NextStreamingInterval(), (uint32_t)3);

    global_meta->UpdateStreamingInterval();
    EXPECT_EQ(global_meta_getter->StreamingInterval(), value);
    EXPECT_EQ(global_meta_getter->StreamingUpdate(), false);

    // core
    global_meta->SetCpuNum(0);
    value = 0;
    EXPECT_EQ(value, global_meta_getter->CpuNum());

    global_meta->SetCpuNum(1);
    value = 1;
    EXPECT_EQ(value, global_meta_getter->CpuNum());

    // aid size
    global_meta->SetAidSize(0);
    value = 0;
    EXPECT_EQ(value, global_meta_getter->AidSize());

    global_meta->SetAidSize(1);
    value = 1;
    EXPECT_EQ(value, global_meta_getter->AidSize());
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
