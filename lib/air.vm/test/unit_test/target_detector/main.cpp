
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>

#include "detect_cor_handler_test.h"

using ::testing::HasSubstr;

TEST_F(DetectCoRHandlerTest, ProcessTest)
{
    mock_node_manager->CreateNodeDataArray(11111); // fake thread array
    mock_node_manager->CreateNodeDataArray(22222); // fake thread array
    mock_node_manager->CreateNodeDataArray(33333); // fake thread array

    EXPECT_NE(nullptr, mock_node_manager->GetNodeDataArray(11111));
    EXPECT_NE(nullptr, mock_node_manager->GetNodeDataArray(22222));
    EXPECT_NE(nullptr, mock_node_manager->GetNodeDataArray(33333));

    detect_cor_handler->HandleRequest();

    EXPECT_NE(nullptr, mock_node_manager->GetNodeDataArray(11111));
    EXPECT_EQ(nullptr, mock_node_manager->GetNodeDataArray(22222));
    EXPECT_EQ(nullptr, mock_node_manager->GetNodeDataArray(33333));
}

int
main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
