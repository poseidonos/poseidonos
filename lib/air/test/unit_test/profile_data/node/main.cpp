
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include <map>
#include <string>

#include "node_manager_test.h"
#include "node_thread_test.h"
#include "pthread.h"
#include "src/config/ConfigParser.cpp"

TEST_F(NodeThreadTest, GetData)
{
    uint32_t aid_idx{0};

    thread = new node::Thread{air::ProcessorType::PERFORMANCE, 32};
    EXPECT_NE(nullptr, thread->GetUserDataByAidIndex(aid_idx));
    EXPECT_NE(nullptr, thread->GetAirData(aid_idx));
    EXPECT_NE(nullptr, thread->GetAccData(aid_idx));

    aid_idx = 1;
    EXPECT_NE(nullptr, thread->GetUserDataByAidIndex(aid_idx));
    EXPECT_NE(nullptr, thread->GetAirData(aid_idx));
    EXPECT_NE(nullptr, thread->GetAccData(aid_idx));

    aid_idx = 33;
    EXPECT_EQ(nullptr, thread->GetUserDataByAidIndex(aid_idx));
    EXPECT_EQ(nullptr, thread->GetAirData(aid_idx));
    EXPECT_EQ(nullptr, thread->GetAccData(aid_idx));

    thread = new node::Thread{air::ProcessorType::PROCESSORTYPE_NULL, 32};
    aid_idx = 0;
    EXPECT_EQ(nullptr, thread->GetUserDataByAidIndex(aid_idx));
    delete thread;

    thread = new node::Thread{air::ProcessorType::PERFORMANCE, 3};
    EXPECT_NE(nullptr, thread->GetUserDataByAidValue(111));
    EXPECT_NE(nullptr, thread->GetUserDataByAidValue(222));
    EXPECT_NE(nullptr, thread->GetUserDataByAidValue(333));
    EXPECT_EQ(nullptr, thread->GetUserDataByAidValue(444));

    EXPECT_EQ((uint64_t)(-1), thread->GetUserAidValue(4));
}

TEST_F(NodeThreadTest, SwapBuffer)
{
    uint32_t aid_idx{0};
    thread = new node::Thread{air::ProcessorType::PERFORMANCE, 32};
    lib::Data* user_data = thread->GetUserDataByAidIndex(aid_idx);
    lib::Data* air_data = thread->GetAirData(aid_idx);

    EXPECT_EQ(1, user_data == thread->GetUserDataByAidIndex(aid_idx));
    EXPECT_EQ(1, air_data == thread->GetAirData(aid_idx));

    thread->SwapBuffer(0);
    EXPECT_EQ(1, air_data == thread->GetUserDataByAidIndex(aid_idx));
    EXPECT_EQ(1, user_data == thread->GetAirData(aid_idx));

    aid_idx = 33;
    thread->SwapBuffer(35);
}

TEST_F(NodeManagerTest, SetThreadName)
{
    uint32_t tid{0x00000001};

    node_manager->CreateThread(tid);
    node::ThreadArray* thread_array = node_manager->GetThread(tid);
    node_manager->SetThreadName(tid);
    const char* str = thread_array->tname.c_str();
    EXPECT_EQ(strcmp(str, "test_suite"), 0);

    tid = 0x00000002;
    node_manager->CreateThread(tid);
    thread_array = node_manager->GetThread(tid);
    pthread_setname_np(pthread_self(), "thread0");
    node_manager->SetThreadName(tid);
    str = thread_array->tname.c_str();
    EXPECT_EQ(strcmp(str, "thread0"), 0);
}

TEST_F(NodeManagerTest, CreateThread)
{
    uint32_t tid{0x00000001};

    node_manager->CreateThread(tid);
    node::ThreadArray* thread_array = node_manager->GetThread(tid);
    node::ThreadArray* thread_array_ = &(node_manager->thread_map[tid]);
    EXPECT_EQ(thread_array_, thread_array);

    tid = 0x00000002;
    node_manager->CreateThread(tid);
    thread_array = node_manager->GetThread(tid);
    EXPECT_NE(thread_array_, thread_array);

    tid = 0x00000001;
    node_manager->CreateThread(tid);
    thread_array = node_manager->GetThread(tid);
    EXPECT_EQ(thread_array_, thread_array);
}

TEST_F(NodeManagerTest, DeleteThread)
{
    uint32_t tid{0x00000001};

    node_manager->CreateThread(tid);
    node::ThreadArray* thread_array = node_manager->GetThread(tid);
    EXPECT_NE(nullptr, thread_array->node[0]);

    node_manager->DeleteThread(thread_array);
    EXPECT_EQ(nullptr, thread_array->node[0]);
}

TEST_F(NodeManagerTest, CanDelete)
{
    uint32_t tid{0x00000001};

    node_manager->CreateThread(tid);
    node::ThreadArray* thread_array = node_manager->GetThread(tid);
    lib::Data* data = thread_array->node[0]->GetAirData(0);

    data->access = 1;
    EXPECT_EQ(false, node_manager->CanDelete(thread_array));
    data->access = 0;
    EXPECT_EQ(true, node_manager->CanDelete(thread_array));

    data = thread_array->node[0]->GetUserDataByAidIndex(1);
    data->access = 1;
    EXPECT_EQ(false, node_manager->CanDelete(thread_array));
    data->access = 0;
    EXPECT_EQ(true, node_manager->CanDelete(thread_array));

    lib::Data* lat_data = thread_array->node[1]->GetUserDataByAidIndex(0);
    lat_data->access = 1;
    EXPECT_EQ(false, node_manager->CanDelete(thread_array));
    lat_data->access = 0;
    EXPECT_EQ(true, node_manager->CanDelete(thread_array));
}

TEST_F(NodeManagerTest, GetAccLatSeqData)
{
    lib::AccLatencySeqData* seq_data = node_manager->GetAccLatSeqData(0, 0, 0);
    EXPECT_EQ((unsigned int)0, seq_data->sample_count);
}

TEST_F(NodeManagerTest, GetAccLatData)
{
    lib::AccLatencySeqData* seq_data = &(node_manager->GetAccLatData(0, 0)->seq_data[0]);
    EXPECT_EQ((unsigned int)0, seq_data->sample_count);
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
