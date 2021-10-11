#include "src/admin/admin_command_complete_handler.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "spdk/pos.h"
#include "test/unit-tests/event_scheduler/callback_mock.h"

using ::testing::_;
using testing::NiceMock;
namespace pos
{
TEST(AdminCommandCompleteHandler, AdminCommandCompleteHandler_Constructor_One_Stack)
{
    uint32_t originCore = 0;
    pos_io* ibofIo;
    CallbackSmartPtr callBack(new NiceMock<MockCallback>(true, 0));
    AdminCommandCompleteHandler adminCommandCompleteHandler(ibofIo, originCore, callBack);
}

TEST(AdminCommandCompleteHandler, AdminCommandCompleteHandler_Constructor_One_Heap)
{
    uint32_t originCore = 0;
    pos_io* ibofIo;
    CallbackSmartPtr callBack(new NiceMock<MockCallback>(true, 0));
    AdminCommandCompleteHandler* adminCommandCompleteHandler = new AdminCommandCompleteHandler(ibofIo, originCore, callBack);
    delete adminCommandCompleteHandler;
}

TEST(AdminCommandCompleteHandler, AdminCommandCompleteHandle_DoSpecificJob_always_true)
{
    // function always returns true
    uint32_t originCore = 0;
    pos_io ibofIo;
    CallbackSmartPtr callBack(new NiceMock<MockCallback>(true, 0));
    AdminCommandCompleteHandler adminCommandCompleteHandler(&ibofIo, originCore, callBack);
    bool expected = true, actual;
    actual = adminCommandCompleteHandler.Execute();
    ASSERT_EQ(expected, actual);
}

} // namespace pos
