#include "src/qos/qos_volume_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "spdk/pos.h"
#include "src/include/pos_event_id.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "test/unit-tests/qos/qos_array_manager_mock.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"
#include "test/unit-tests/qos/qos_volume_manager_spy.h"
#include "test/unit-tests/qos/rate_limit_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace std;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(QosVolumeManager, QosVolumeManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
}

TEST(QosVolumeManager, QosVolumeManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager* qosVolumeManager = new QosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    delete qosVolumeManager;
}

TEST(QosVolumeManager, VolumeCreated_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    bool expected = true, actual;
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeEventPerf volumePerf;
    volumePerf.maxiops = 10;
    volumePerf.maxbw = 10;
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    actual = qosVolumeManager.VolumeCreated(&volumeBase, &volumePerf, &volumeArrayInfo);
    ASSERT_EQ(expected, actual);
}

TEST(QosVolumeManager, VolumeDeleted_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeEventPerf volumePerf;
    volumePerf.maxiops = 10;
    volumePerf.maxbw = 10;
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    bool expected = true, actual;
    actual = qosVolumeManager.VolumeDeleted(&volumeBase, &volumeArrayInfo);

    ASSERT_EQ(expected, actual);
}

TEST(QosVolumeManager, VolumeMounted_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQoscontext, GetVolumeOperationDone()).WillByDefault(Return(true));
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, QosVolumeManager::_VolumeMountHandler, _, _)).Times(1);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeEventPerf volumePerf;
    volumePerf.maxiops = 10;
    volumePerf.maxbw = 10;
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    bool expected = true, actual;
    actual = qosVolumeManager.VolumeMounted(&volumeBase, &volumePerf, &volumeArrayInfo);
    ASSERT_EQ(expected, actual);
}

TEST(QosVolumeManager, VolumeUnmounted_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQoscontext, GetVolumeOperationDone()).WillByDefault(Return(true));
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, QosVolumeManager::_VolumeUnmountHandler, _, _)).Times(1);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    bool expected = true, actual;
    actual = qosVolumeManager.VolumeUnmounted(&volumeBase, &volumeArrayInfo);
    ASSERT_EQ(expected, actual);
}
TEST(QosVolumeManager, VolumeUnmounted_Default_Parameters_feQos_false)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = false;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    int expected = EID(VOL_EVENT_OK), actual;
    actual = qosVolumeManager.VolumeUnmounted(&volumeBase, &volumeArrayInfo);
    ASSERT_EQ(expected, actual);
}

TEST(QosVolumeManager, VolumeLoaded_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeEventPerf volumePerf;
    volumePerf.maxiops = 10;
    volumePerf.maxbw = 10;
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    bool expected = true, actual;
    actual = qosVolumeManager.VolumeLoaded(&volumeBase, &volumePerf, &volumeArrayInfo);

    ASSERT_EQ(expected, actual);
}

TEST(QosVolumeManager, VolumeUpdated_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t volId = 0;
    qos_vol_policy volumePolicy = mockQosArrayManager.GetVolumePolicy(0);
    volumePolicy.maxIops = 10;
    volumePolicy.maxBw = 10;

    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeEventPerf volumePerf;
    volumePerf.maxiops = 10;
    volumePerf.maxbw = 10;
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    bool expected = true, actual;
    actual = qosVolumeManager.VolumeUpdated(&volumeBase, &volumePerf, &volumeArrayInfo);

    ASSERT_EQ(expected, actual);
}

TEST(QosVolumeManager, VolumeDetached_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    ON_CALL(mockQoscontext, GetVolumeOperationDone()).WillByDefault(Return(true));
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    EXPECT_CALL(mockEventFrameworkApi, SendSpdkEvent(_, QosVolumeManager::_VolumeDetachHandler, _, _)).Times(3);
    std::string arrayName = "POSArray1";
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeEventPerf volumePerf;
    volumePerf.maxiops = 10;
    volumePerf.maxbw = 10;
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";

    vector<int> volList{1, 2, 3};
    bool expected = true, actual;
    qosVolumeManager.VolumeDetached(volList, &volumeArrayInfo);
}
TEST(QosVolumeManager, HandlePosIoSubmission_Test_feQosDisable)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = false;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    AioSubmissionAdapter aioSubmission;
    pos_io io;
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Write;
    qosVolumeManager.HandlePosIoSubmission(&aioSubmission, volIo);
}

TEST(QosVolumeManager, VolumeQosPoller_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = false;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t nqnId = 1;
    uint32_t volId = 1;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    qosVolumeManager.UpdateSubsystemToVolumeMap(nqnId, volId);
    AioSubmissionAdapter aioSubmission;
    double offset = 1.0;
    int actualRetVal = qosVolumeManager.VolumeQosPoller(&aioSubmission, offset);
    ASSERT_EQ(actualRetVal, 0);
}

TEST(QosVolumeManager, Check_Getter_And_Setter_VolumeLimit)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    uint32_t volId = 0;
    int64_t value = 100;
    bool iops = true;
    qosVolumeManager.SetVolumeLimit(volId, value, iops);
    int64_t retVal = qosVolumeManager.GetVolumeLimit(volId, iops);
    ASSERT_EQ(value, retVal);
}

TEST(QosVolumeManager, Check_Getter_Setter_Subsystem_VolumeMap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    uint32_t nqnId = 1;
    uint32_t volId = 1;

    qosVolumeManager.UpdateSubsystemToVolumeMap(nqnId, volId);
    qosVolumeManager.UpdateSubsystemToVolumeMap(nqnId, volId);
    std::vector<int> volList = qosVolumeManager.GetVolumeFromActiveSubsystem(nqnId);
    std::vector<int>::iterator position = std::find(volList.begin(), volList.end(), volId);
    ASSERT_NE(position, volList.end());

    qosVolumeManager.DeleteVolumeFromSubsystemMap(nqnId, volId);
    volList = qosVolumeManager.GetVolumeFromActiveSubsystem(nqnId);
    position = std::find(volList.begin(), volList.end(), volId);
    ASSERT_EQ(position, volList.end());
}

TEST(QosVolumeManager, ResetRateLimit_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    NiceMock<MockBwIopsRateLimit> mockBwIopsRateLimit;
    uint32_t reactor = 1;
    int volId = 1;
    double offset = 100;
    // EXPECT_CALL(mockBwIopsRateLimit, ResetRateLimit);
    qosVolumeManager.ResetRateLimit(reactor, volId, offset);
}

TEST(QosVolumeManager, Check_Getter_And_Setter_ArrayName)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    std::string arrayName = "POSArray1";
    qosVolumeManager.SetArrayName(arrayName);
    std::string returnedArrayName = qosVolumeManager.GetArrayName();
    int expectedRetVal = 0;
    int actualRetVal = arrayName.compare(returnedArrayName);
    ASSERT_EQ(expectedRetVal, actualRetVal);
}
TEST(QosVolumeManager, _VolumeMountHandlerTest)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManagerSpy qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    std::string arrayName = "POSArray";
    struct pos_volume_info* vInfo = new (struct pos_volume_info);
    strncpy(vInfo->array_name, arrayName.c_str(), arrayName.size());
    int len = arrayName.size();
    vInfo->array_name[len] = '\0';
    vInfo->id = 1;
    vInfo->iops_limit = 10;
    vInfo->bw_limit = 10;

    qosVolumeManager.VolumeMountHandler(vInfo, &qosVolumeManager);
}

TEST(QosVolumeManager, _VolumeUnmountHandlerTest)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManagerSpy qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    std::string arrayName = "POSArray";
    struct pos_volume_info* vInfo = new (struct pos_volume_info);
    strncpy(vInfo->array_name, arrayName.c_str(), arrayName.size());
    int len = arrayName.size();
    vInfo->array_name[len] = '\0';
    vInfo->id = 1;
    vInfo->iops_limit = 0;
    vInfo->bw_limit = 0;

    qosVolumeManager.VolumeUnmountHandler(vInfo, &qosVolumeManager);
}

TEST(QosVolumeManager, _VolumeDetachHandlerTest)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    NiceMock<MockQosManager> mockQosManager;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManagerSpy qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    std::string arrayName = "POSArray";
    struct pos_volume_info* vInfo = new (struct pos_volume_info);
    strncpy(vInfo->array_name, arrayName.c_str(), arrayName.size());
    int len = arrayName.size();
    vInfo->array_name[len] = '\0';
    vInfo->id = 1;
    vInfo->iops_limit = 0;
    vInfo->bw_limit = 0;

    qosVolumeManager.VolumeDetachHandler(vInfo, &qosVolumeManager);
}

TEST(QosVolumeManager, HandlePosIoSubmission_Test_feQosEnable)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);

    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(1));

    uint32_t volId = 1;

    qosVolumeManager.SetVolumeLimit(volId, 10, false);
    AioSubmissionAdapter aioSubmission;

    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Deallocate;
    try
    {
        qosVolumeManager.HandlePosIoSubmission(&aioSubmission, volIo);
    }
    catch (...)
    {
    }
}

TEST(QosVolumeManager, EnqueueVolumeParamsUt_Test)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(1));

    uint32_t volId = 1;
    qosVolumeManager.SetVolumeLimit(volId, 10, false);
    AioSubmissionAdapter aioSubmission;
    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Deallocate;
    try
    {
        qosVolumeManager.HandlePosIoSubmission(&aioSubmission, volIo);
    }
    catch (...)
    {
    }

    ON_CALL(mockQosManager, IsMinimumPolicyInEffectInSystem()).WillByDefault(Return(true));
}

TEST(QosVolumeManager, EnqueueDequeueVolumeIo)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, 8, 0));
    qosVolumeManager.EnqueueVolumeIo(1, volumeIo);
    VolumeIoSmartPtr retVolIo = qosVolumeManager.DequeueVolumeIo(1);
    ASSERT_EQ(retVolIo.get(), volumeIo.get());
}

TEST(QosVolumeManager, SubmitAio)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 0;
    bool feQosEnabled = true;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager, &mockEventFrameworkApi, &mockQosManager);
    VolumeIoSmartPtr volumeIo(new VolumeIo(nullptr, 8, 0));
    AioSubmissionAdapter aioSubmission;
    try
    {
        qosVolumeManager.SubmitVolumeIoToAio(&aioSubmission, 1, volumeIo);
    }
    catch(...)
    {
    }
}

} // namespace pos
