
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "in_test.h"

TEST_F(InTest, HandleKernelMsg)
{
    char command[6][30];
    strcpy(command[0], "air-run");
    strcpy(command[1], "air-stream-interval");
    strcpy(command[2], "node-run");
    strcpy(command[3], "node-init");
    strcpy(command[4], "node-sample-ratio");
    strcpy(command[5], "error");

    for (int range = 1; range <= 4; range++)
    {
        for (int cmd = 0; cmd < 5; cmd++)
        {
            fake_cli_send->FakeSend(range, command[cmd]);
            EXPECT_EQ(1, in_command->HandleKernelMsg());
        }
    }

    fake_cli_send->FakeSend(1, command[6]);
    EXPECT_EQ(0, in_command->HandleKernelMsg());
}

TEST_F(InTest, Subject_Notify)
{
    EXPECT_EQ(0, input_subject->Notify(0, 0, 0, 0, 0, 0, 0, 0));
    EXPECT_EQ(-1, input_subject->Notify(99, 0, 0, 0, 0, 0, 0, 0));
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
