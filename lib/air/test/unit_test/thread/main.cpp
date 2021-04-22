
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "thread_manager_test.h"
#include "thread_test.h"

TEST_F(ThreadTest, OneTimeThread)
{
    EXPECT_EQ(nullptr, one_time_thread->RunThreadLoop((void*)one_time_thread));
    EXPECT_EQ(0, pthread_mutex_trylock(&thread::g_thread_mutex));
    EXPECT_EQ((uint32_t)0, one_time_thread->GetRunSkipCount());
    one_time_thread->SetCpuSet(0);
    one_time_thread->StartThread(); // skip count: 1
    usleep(10000);
    EXPECT_EQ((uint32_t)1, one_time_thread->GetRunSkipCount());
    one_time_thread->PullTrigger(); // skip count: 0
    usleep(10000);
    EXPECT_EQ((uint32_t)0, one_time_thread->GetRunSkipCount());
    one_time_thread->JoinThread();
}

TEST_F(ThreadTest, PeriodicThread)
{
    EXPECT_EQ(nullptr, periodic_thread->RunThreadLoop((void*)periodic_thread));
    periodic_thread->StartThread();
    usleep(10000);
    periodic_thread->JoinThread();

    // test thread destructor
    periodic_thread->StartThread();
    usleep(10000);
}

TEST_F(ThreadManagerTest, ChainManagerThread)
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);

    EXPECT_EQ(0, pthread_mutex_trylock(&thread::g_thread_mutex));
    EXPECT_EQ((uint32_t)0, mock_chain_manager->GetRunSkipCount());
    mock_chain_manager->SetCpuSet(0);
    mock_chain_manager->StartThread(); // skip count: 1
    usleep(10000);
    EXPECT_EQ((uint32_t)1, mock_chain_manager->GetRunSkipCount());
    thread_manager->RunThread(1); // skip count: 0
    usleep(10000);
    EXPECT_EQ((uint32_t)0, mock_chain_manager->GetRunSkipCount());
    mock_chain_manager->JoinThread();
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
