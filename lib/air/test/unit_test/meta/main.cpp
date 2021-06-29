
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "global_meta_test.h"
#include "node_meta_test.h"
#include "src/config/ConfigInterface.h"

TEST_F(NodeMetaTest, NodeMeta)
{
    // node processor type
    air::ProcessorType processor_type;
    processor_type = air::ProcessorType::PROCESSORTYPE_NULL;
    EXPECT_EQ(node_meta_getter->ProcessorType(0), processor_type); //default
    EXPECT_EQ(node_meta_getter->ProcessorType(1), processor_type); //default

    processor_type = air::ProcessorType::PERFORMANCE;
    node_meta->SetProcessorType(0, air::ProcessorType::PERFORMANCE);
    EXPECT_EQ(node_meta_getter->ProcessorType(0), processor_type);

    processor_type = air::ProcessorType::LATENCY;
    node_meta->SetProcessorType(1, air::ProcessorType::LATENCY);
    EXPECT_EQ(node_meta_getter->ProcessorType(1), processor_type);

    // run
    EXPECT_EQ(false, node_meta_getter->Run(0)); //default

    node_meta->SetRun(2, false);
    EXPECT_EQ(false, node_meta_getter->Run(2));

    // sampling ratio
    EXPECT_EQ(1000, node_meta_getter->SampleRatio(0)); //default

    node_meta->SetSampleRatio(0, 100);
    EXPECT_EQ(100, node_meta_getter->SampleRatio(0));

    // group id
    uint32_t group_id = cfg::GetSentenceIndex(config::ParagraphType::GROUP, "SUBMIT");
    node_meta->SetGroupId(0, group_id);
    EXPECT_EQ(group_id, node_meta_getter->GroupId(0));

    // meta, check values set above
    air::NodeMetaData* node = (air::NodeMetaData*)node_meta_getter->Meta();
    EXPECT_EQ(node[0].processor_type, air::ProcessorType::PERFORMANCE);
    EXPECT_EQ(node[1].processor_type, air::ProcessorType::LATENCY);
    EXPECT_EQ(node[0].sample_ratio, (uint32_t)100);
    EXPECT_EQ(node[0].group_id, cfg::GetSentenceIndex(config::ParagraphType::GROUP, "SUBMIT"));
}

TEST_F(GlobalMetaTest, GlobalMeta)
{
    // enable
    EXPECT_EQ(true, global_meta_getter->AirPlay()); //default

    global_meta->SetAirPlay(false);
    EXPECT_EQ(false, global_meta_getter->AirPlay());

    // streaming interval
    EXPECT_EQ(1, global_meta_getter->StreamingInterval());   //default
    EXPECT_EQ(false, global_meta_getter->StreamingUpdate()); //default

    global_meta->SetStreamingInterval(3);
    EXPECT_EQ(1, global_meta_getter->StreamingInterval());
    EXPECT_EQ(true, global_meta_getter->StreamingUpdate());
    EXPECT_EQ(3, global_meta_getter->NextStreamingInterval());

    global_meta->UpdateStreamingInterval();
    EXPECT_EQ(3, global_meta_getter->StreamingInterval());
    EXPECT_EQ(false, global_meta_getter->StreamingUpdate());

    // core
    global_meta->SetCpuNum(10);
    EXPECT_EQ(10, global_meta_getter->CpuNum());

    global_meta->SetCpuNum(1);
    EXPECT_EQ(1, global_meta_getter->CpuNum());
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
