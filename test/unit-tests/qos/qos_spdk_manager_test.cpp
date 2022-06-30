#include "src/qos/qos_spdk_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "src/qos/qos_common.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;

namespace pos
{
TEST(QosSpdkManager, QosSpdkManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
}
TEST(QosSpdkManager, QosSpdkManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = false;
    QosSpdkManager* qosSpdkManager = new QosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    delete qosSpdkManager;
}
TEST(QosSpdkManager, Initialize_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);

    qosSpdkManager.Initialize();
}

TEST(QosSpdkManager, Finalize_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    qosSpdkManager.Finalize();
}

TEST(QosSpdkManager, Check_Update_And_GetReactorData)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
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
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    uint32_t reactor = 1;
    struct spdk_poller* sp[1];
    
    qosSpdkManager.UpdateSpdkPoller(reactor, sp[0]);
    struct spdk_poller* retSpdkPoller = qosSpdkManager.GetSpdkPoller(reactor);
    ASSERT_EQ(retSpdkPoller, sp[0]);   
}

TEST(QosSpdkManager, RegisterQosPoller_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    void *arg1 = static_cast<void *>(&qosSpdkManager);
    int buf[10];
    qosSpdkManager.RegisterQosPoller(arg1, (void*)&buf);
}

TEST(QosSpdkManager, PollerUnregister_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    void *arg1 = static_cast<void *>(&qosSpdkManager);
    int buf[10];
    qosSpdkManager.PollerUnregister(arg1, (void*)&buf);
}

TEST(QosSpdkManager, SpdkVolumeQosPoller_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    bool feQos = false;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    struct poller_structure param;
    qosSpdkManager.SpdkVolumeQosPoller((void*)&param);
}

TEST(QosSpdkManager, Check_Set_And_GetReactorId)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = false;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    uint32_t reactorId = 10;
    qosSpdkManager.SetReactorId(reactorId);
    uint32_t recdReactorId = qosSpdkManager.GetReactorId();
    ASSERT_EQ(reactorId, recdReactorId);
}

TEST(QosSpdkManager, IsFeQosEnabled_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    bool feQos = true;
    QosSpdkManager qosSpdkManager(&mockQoscontext, feQos, &mockEventFrameworkApi);
    bool recdFeQos = qosSpdkManager.IsFeQosEnabled();
    ASSERT_EQ(feQos, recdFeQos);
}
} // namespace pos
