
#include <stdio.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "air_test.h"
#include "src/config/ConfigInterface.h"

air::InstanceManager* AIR<true, true>::instance_manager = nullptr;
node::NodeManager* AIR<true, true>::node_manager = nullptr;
collection::CollectionManager* AIR<true, true>::collection_manager = nullptr;
thread_local node::ThreadArray* AIR<true, true>::thread_array = nullptr;

FakeCollectionManager* MockAIR::fake_collection_manager = nullptr;
FakeInstanceManager* MockAIR::fake_instance_manager = nullptr;
FakeNodeManager* MockAIR::fake_node_manager = nullptr;

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
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "PERF_BENCHMARK")>(0, AIR_READ, 4096);
    uint64_t value {4096};
    EXPECT_EQ(value, mock_air->fake_collection_manager->value2);

    mock_air->SetNullCollectionManager();
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "PERF_BENCHMARK")>(0, AIR_READ, 4096);
}

TEST_F(TestAPI, UT_AIR_LogLat)
{
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "LAT_SUBMIT")>(0, 0, 123);

    AIR<true, true>::thread_array = nullptr;
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "LAT_SUBMIT")>(0, 0, 123);

    mock_air->SetNullCollectionManager();
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "LAT_SUBMIT")>(0, 0, 123);
}

TEST_F(TestAPI, UT_AIR_LogQ)
{
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "Q_SUBMISSION")>(0, 128, 256);
    uint32_t value {128};
    EXPECT_EQ(value, mock_air->fake_collection_manager->value1);

    AIR<true, true>::thread_array = nullptr;
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "Q_SUBMISSION")>(0, 128, 256);

    mock_air->SetNullCollectionManager();
    mock_air->LogData<cfg::GetIndex(config::ConfigType::NODE, "Q_SUBMISSION")>(0, 128, 256);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
