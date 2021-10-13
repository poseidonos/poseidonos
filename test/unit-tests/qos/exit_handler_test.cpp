#include "src/qos/exit_handler.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ExitQosHandler, ExitQosHandler_Contructor_One_Stack)
{
    ExitQosHandler exitQosHandler();
}

TEST(ExitQosHandler, ExitQosHandler_Contructor_One_Heap)
{
    ExitQosHandler* exitQosHandler = new ExitQosHandler();
    delete exitQosHandler;
}

TEST(ExitQosHandler, Check_Getter_Setter_ExitQos)
{
    ExitQosHandler exitQosHandler;
    exitQosHandler.SetExitQos();
    bool expected = true, actual;
    actual = exitQosHandler.IsExitQosSet();
    ASSERT_EQ(expected, actual);
}

} // namespace pos
