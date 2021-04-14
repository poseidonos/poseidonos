
#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "chain_manager_test.h"
#include "mock_cor_handler.h"

TEST_F(ChainManagerTest, Init)
{
    uint32_t value;
    chain::AnalysisTask* anlysis_task = chain_manager->GetAnalysisTask();
    value = 1000;
    EXPECT_EQ(anlysis_task->GetPeriod(), value); // default

    chain_manager->Init(); // mock streaming intverval -> 3
    value = 3000;
    EXPECT_EQ(anlysis_task->GetPeriod(), value); 
}

TEST_F(ChainManagerTest, RunThread)
{
    /*
    <Period Default>
    switch gear     10ms
    preprocess     100ms
    cli            100ms
    analysis       1000ms
    */
    chain::SwitchGearTask* switch_gear_task = chain_manager->GetSwitchGearTask();
    chain::PreprocessTask* preprocess_task = chain_manager->GetPreprocessTask();
    chain::CLITask* cli_task = chain_manager->GetCLITask();
    chain::AnalysisTask* anlysis_task = chain_manager->GetAnalysisTask();

    // make fake chains
    MockCoRHandler* mock_cor_handler = new MockCoRHandler();
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::INPUT));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::POLICY));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::COLLECTION));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::OUTPUT));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::PROCESS));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::STREAM));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::SWITCHGEAR));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::PREPROCESS));
    chain_manager->AttachChain(mock_cor_handler,
                               to_dtype(pi::ChainHandler::DETECT));

    // after 10ms (total: 10ms), trigger switch gear  
    chain_manager->RunThread(0); 
    EXPECT_EQ(switch_gear_task->GetDeadline(), (uint32_t)10);
    EXPECT_EQ(preprocess_task->GetDeadline(), (uint32_t)90);
    EXPECT_EQ(cli_task->GetDeadline(), (uint32_t)90);
    EXPECT_EQ(anlysis_task->GetDeadline(), (uint32_t)990);

    // after 90ms(10ms(tick)+80ms(delay)) (total: 100ms), trigger switch gear, preprocess, cli
    chain_manager->RunThread(8);
    EXPECT_EQ(switch_gear_task->GetDeadline(), (uint32_t)10);
    EXPECT_EQ(preprocess_task->GetDeadline(), (uint32_t)100);
    EXPECT_EQ(cli_task->GetDeadline(), (uint32_t)100);
    EXPECT_EQ(anlysis_task->GetDeadline(), (uint32_t)900);

    // after 900ms (10ms(tick)+890ms(delay)) (total: 1000ms), trigger all
    chain_manager->RunThread(89);
    EXPECT_EQ(switch_gear_task->GetDeadline(), (uint32_t)10);
    EXPECT_EQ(preprocess_task->GetDeadline(), (uint32_t)100);
    EXPECT_EQ(cli_task->GetDeadline(), (uint32_t)100);
    EXPECT_EQ(anlysis_task->GetDeadline(), (uint32_t)3000); // after trigger, the period is updated
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
