
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include <map>
#include <string>

#include "node_data_test.h"
#include "node_manager_test.h"
#include "pthread.h"

TEST_F(NodeDataTest, GetData)
{
    uint32_t idx{0};

    node_data = new node::NodeData{air::ProcessorType::PERFORMANCE, 32, 32};
    EXPECT_NE(nullptr, node_data->GetUserDataByHashIndex(idx, idx));
    EXPECT_NE(nullptr, node_data->GetAirData(idx, idx));
    EXPECT_NE(nullptr, node_data->GetAccData(idx, idx));

    idx = 1;
    EXPECT_NE(nullptr, node_data->GetUserDataByHashIndex(idx, idx));
    EXPECT_NE(nullptr, node_data->GetAirData(idx, idx));
    EXPECT_NE(nullptr, node_data->GetAccData(idx, idx));

    idx = 33;
    EXPECT_EQ(nullptr, node_data->GetUserDataByHashIndex(idx, idx));
    EXPECT_EQ(nullptr, node_data->GetAirData(idx, idx));
    EXPECT_EQ(nullptr, node_data->GetAccData(idx, idx));

    EXPECT_NE(nullptr, node_data->GetUserDataByNodeIndex(111, 0));
    EXPECT_NE(nullptr, node_data->GetUserDataByNodeIndex(222, 0));
    EXPECT_NE(nullptr, node_data->GetUserDataByNodeIndex(333, 0));
}

TEST_F(NodeDataTest, SwapBuffer)
{
    uint32_t idx{0};
    node_data = new node::NodeData{air::ProcessorType::PERFORMANCE, 32, 32};
    lib::Data* user_data = node_data->GetUserDataByHashIndex(idx, idx);
    lib::Data* air_data = node_data->GetAirData(idx, idx);

    EXPECT_EQ(1, user_data == node_data->GetUserDataByHashIndex(idx, idx));
    EXPECT_EQ(1, air_data == node_data->GetAirData(idx, idx));

    node_data->SwapBuffer(0);
    EXPECT_EQ(1, air_data == node_data->GetUserDataByHashIndex(idx, idx));
    EXPECT_EQ(1, user_data == node_data->GetAirData(idx, idx));

    node_data->SwapBuffer(33);
}

TEST_F(NodeManagerTest, SetNodeDataArrayName)
{
    uint32_t tid{0x00000001};

    node_manager->CreateNodeDataArray(tid);
    node::NodeDataArray* node_data_array = node_manager->GetNodeDataArray(tid);
    node_manager->SetNodeDataArrayName(tid);
    const char* str = node_data_array->tname.c_str();
    EXPECT_EQ(strcmp(str, "test_suite"), 0);

    tid = 0x00000002;
    node_manager->CreateNodeDataArray(tid);
    node_data_array = node_manager->GetNodeDataArray(tid);
    pthread_setname_np(pthread_self(), "thread0");
    node_manager->SetNodeDataArrayName(tid);
    str = node_data_array->tname.c_str();
    EXPECT_EQ(strcmp(str, "thread0"), 0);
}

TEST_F(NodeManagerTest, CreateNodeDataArray)
{
    uint32_t tid{0x00000001};

    node_manager->CreateNodeDataArray(tid);
    node::NodeDataArray* node_data_array = node_manager->GetNodeDataArray(tid);
    node::NodeDataArray* node_data_array_ = node_manager->nda_map[tid];
    EXPECT_EQ(node_data_array_, node_data_array);

    tid = 0x00000002;
    node_manager->CreateNodeDataArray(tid);
    node_data_array = node_manager->GetNodeDataArray(tid);
    EXPECT_NE(node_data_array_, node_data_array);

    tid = 0x00000001;
    node_manager->CreateNodeDataArray(tid);
    node_data_array = node_manager->GetNodeDataArray(tid);
    EXPECT_EQ(node_data_array_, node_data_array);
}

TEST_F(NodeManagerTest, DeleteNodeDataArray)
{
    uint32_t tid{0x00000001};

    node_manager->CreateNodeDataArray(tid);
    node::NodeDataArray* node_data_array = node_manager->GetNodeDataArray(tid);
    EXPECT_NE(nullptr, node_data_array->node[0]);

    auto iter = node_manager->nda_map.begin();
    while (iter != node_manager->nda_map.end())
    {
        node_manager->DeleteNodeDataArray(iter->second);
        node_manager->nda_map.erase(iter++);
    }
    EXPECT_EQ(nullptr, node_manager->GetNodeDataArray(tid));
}

TEST_F(NodeManagerTest, CanDelete)
{
    uint32_t tid{0x00000001};

    node_manager->CreateNodeDataArray(tid);
    node::NodeDataArray* node_data_array = node_manager->GetNodeDataArray(tid);
    lib::Data* data = node_data_array->node[0]->GetAirData(0, 0);

    data->access = 1;
    EXPECT_EQ(false, node_manager->CanDeleteNodeDataArray(node_data_array));
    data->access = 0;
    EXPECT_EQ(true, node_manager->CanDeleteNodeDataArray(node_data_array));

    data = node_data_array->node[0]->GetUserDataByHashIndex(1, 0);
    data->access = 1;
    EXPECT_EQ(false, node_manager->CanDeleteNodeDataArray(node_data_array));
    data->access = 0;
    EXPECT_EQ(true, node_manager->CanDeleteNodeDataArray(node_data_array));

    lib::Data* lat_data = node_data_array->node[1]->GetUserDataByHashIndex(0, 0);
    lat_data->access = 1;
    EXPECT_EQ(false, node_manager->CanDeleteNodeDataArray(node_data_array));
    lat_data->access = 0;
    EXPECT_EQ(true, node_manager->CanDeleteNodeDataArray(node_data_array));
}

TEST_F(NodeManagerTest, GetAccLatSeqData)
{
    node_manager->CreateNodeDataArray(0);
    lib::AccLatencyData* seq_data = node_manager->GetAccLatData(0, 0, 0);
    EXPECT_EQ(nullptr, seq_data);
    seq_data = node_manager->GetAccLatData(1, 0, 0);
    EXPECT_EQ(0, seq_data->sample_count);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
