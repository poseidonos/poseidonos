
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "instance_test.h"

TEST_F(InstanceTest, Initialize)
{
    EXPECT_EQ(0, instance_manager->Initialize(0));
}

TEST_F(InstanceTest, Activate)
{
    EXPECT_EQ(0, instance_manager->Initialize(0));
    EXPECT_EQ(0, instance_manager->Activate());
}

TEST_F(InstanceTest, Deactivate)
{
    EXPECT_EQ(0, instance_manager->Initialize(0));
    EXPECT_EQ(0, instance_manager->Deactivate());
}

TEST_F(InstanceTest, Finalize)
{
    EXPECT_EQ(0, instance_manager->Initialize(0));
    EXPECT_EQ(0, instance_manager->Finalize());
}

TEST_F(InstanceTest, GetNodeManager)
{
    EXPECT_EQ(0, instance_manager->Initialize(0));
    EXPECT_NE(nullptr, instance_manager->GetNodeManager());
}

TEST_F(InstanceTest, GetCollectionManager)
{
    EXPECT_EQ(0, instance_manager->Initialize(0));
    EXPECT_NE(nullptr, instance_manager->GetCollectionManager());
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
