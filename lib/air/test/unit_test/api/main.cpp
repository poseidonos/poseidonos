
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "air_test.h"
#include "src/config/ConfigInterface.h"

air::InstanceManager* AIR<true, true>::instance_manager = nullptr;
node::NodeManager* AIR<true, true>::node_manager = nullptr;
collection::CollectionManager* AIR<true, true>::collection_manager = nullptr;
thread_local node::NodeDataArray* AIR<true, true>::node_data_array = nullptr;

FakeCollectionManager* MockAIR::fake_collection_manager = nullptr;
FakeInstanceManager* MockAIR::fake_instance_manager = nullptr;
FakeNodeManager* MockAIR::fake_node_manager = nullptr;

enum IOtype
{
    AIR_READ,
    AIR_WRITE,
};

TEST_F(TestAPI, UT_AIR_Initialize)
{
    mock_air->Initialize();
}

TEST_F(TestAPI, UT_AIR_Activate)
{
    mock_air->Activate();
}

TEST_F(TestAPI, UT_AIR_Deactivate)
{
    mock_air->Deactivate();
}

TEST_F(TestAPI, UT_AIR_Finalize)
{
    mock_air->Finalize();
}

TEST_F(TestAPI, UT_AIR_LogPerf)
{
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "PERF_BENCHMARK"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "PERF_BENCHMARK"), "AIR_READ")>(0, 4096);

    mock_air->SetNullCollectionManager();
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "PERF_BENCHMARK"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "PERF_BENCHMARK"), "AIR_READ")>(0, 4096);
}

TEST_F(TestAPI, UT_AIR_LogLat)
{
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "LAT_SUBMIT"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "LAT_SUBMIT"), "AIR_0")>(0, 123);

    AIR<true, true>::node_data_array = nullptr;
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "LAT_SUBMIT"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "LAT_SUBMIT"), "AIR_0")>(0, 123);

    mock_air->SetNullCollectionManager();
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "LAT_SUBMIT"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "LAT_SUBMIT"), "AIR_0")>(0, 123);
}

TEST_F(TestAPI, UT_AIR_LogQ)
{
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "Q_SUBMISSION"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "Q_SUBMISSION"), "AIR_BASE")>(0, 128);
    uint32_t value{128};
    EXPECT_EQ(value, mock_air->fake_collection_manager->value);

    AIR<true, true>::node_data_array = nullptr;
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "Q_SUBMISSION"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "Q_SUBMISSION"), "AIR_BASE")>(0, 128);

    mock_air->SetNullCollectionManager();
    mock_air->LogData<cfg::GetSentenceIndex(config::ParagraphType::NODE, "Q_SUBMISSION"),
        cfg::GetIntValue(config::ParagraphType::FILTER, "Item",
            cfg::GetStrValue(config::ParagraphType::NODE, "Filter", "Q_SUBMISSION"), "AIR_BASE")>(0, 128);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
