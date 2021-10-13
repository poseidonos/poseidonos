#include "src/qos/qos_spdk_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "lib/spdk/include/spdk_internal/thread.h"
#include "src/qos/qos_common.h"
#include "src/spdk_wrapper/poller_management.h"
#include "test/unit-tests/qos/qos_context_mock.h"
using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosSpdkManager, QosSpdkManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
}
TEST(QosSpdkManager, QosSpdkManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = false;
    QosSpdkManager* qosSpdkManager = new QosSpdkManager(&mockQoscontext, feQos);
    delete qosSpdkManager;
}
TEST(QosSpdkManager, Initialize_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);

    qosSpdkManager.Initialize();
}

TEST(QosSpdkManager, Finalize_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    qosSpdkManager.Finalize();
}

TEST(QosSpdkManager, Check_Update_And_GetReactorData)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    uint32_t reactor = 1;
    struct poller_structure data;
    data.id = 10;
    qosSpdkManager.UpdateReactorData(reactor, data);
    struct poller_structure returnedStruct = qosSpdkManager.GetReactorData(reactor);
    ASSERT_EQ(returnedStruct.id, 10);
}

TEST(QosSpdkManager, Check_Update_And_Get_SpdkPoller)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    uint32_t reactor = 1;
    struct spdk_poller spdkPoller;
    spdkPoller.next_run_tick = 10;
    qosSpdkManager.UpdateSpdkPoller(reactor, &spdkPoller);
    struct spdk_poller* retSpdkPoller = qosSpdkManager.GetSpdkPoller(reactor);
    ASSERT_EQ(retSpdkPoller->next_run_tick, 10);
}

TEST(QosSpdkManager, RegisterQosPoller_Test)
{
    /*NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    void *arg1 = static_cast<void *>(&qosSpdkManager);
    int buf[10];
    qosSpdkManager.RegisterQosPoller(arg1,(void*)&buf);*/
}

TEST(QosSpdkManager, PollerUnregister_Test)
{
    /*NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    void *arg1 = static_cast<void *>(&qosSpdkManager);
    int buf[10];
    qosSpdkManager.PollerUnregister(arg1,(void*)&buf);*/
}

TEST(QosSpdkManager, SpdkVolumeQosPoller_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    struct poller_structure param;
    qosSpdkManager.SpdkVolumeQosPoller((void*)&param);
}

TEST(QosSpdkManager, Check_Set_And_GetReactorId)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    uint32_t reactorId = 10;
    qosSpdkManager.SetReactorId(reactorId);
    uint32_t recdReactorId = qosSpdkManager.GetReactorId();
    ASSERT_EQ(reactorId, recdReactorId);
}

TEST(QosSpdkManager, IsFeQosEnabled_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos);
    bool recdFeQos = qosSpdkManager.IsFeQosEnabled();
    ASSERT_EQ(feQos, recdFeQos);
}
} // namespace pos
