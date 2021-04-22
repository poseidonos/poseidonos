
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "out_test.h"
#include "output_manager_test.h"

TEST_F(OutTest, Send)
{
    long pid = 1234;
    int return_code = 0;
    int cmd_type = 0;
    int cmd_order = 0;

    EXPECT_EQ(0, out_command->Send(pid, return_code, cmd_type, cmd_order));
}

TEST_F(OutputManagerTest, HandleMsg)
{
    int num_req = 10;
    for (int i = 0; i < num_req; i++)
    {
        output_manager->EnqueueMsg(0, 0, 0, 0, 0, 0, 0);
    }
    EXPECT_EQ(num_req, output_manager->HandleMsg());

    fake_out->num_called = 0;
    EXPECT_EQ(0, output_manager->HandleMsg());

    fake_out->fail_on = true;
    for (int i = 0; i < num_req; i++)
    {
        output_manager->EnqueueMsg(0, 0, 0, 0, 0, 0, 0);
    }
    fake_out->num_called = 0;
    EXPECT_EQ(num_req, output_manager->HandleMsg());

    fake_out->sequential_fail = true;
    for (int i = 0; i < num_req; i++)
    {
        output_manager->EnqueueMsg(0, 0, 0, 0, 0, 0, 0);
    }
    fake_out->num_called = 0;
    EXPECT_EQ(num_req - 5, output_manager->HandleMsg());
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
