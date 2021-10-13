#include "src/qos/qos_volume_manager.h"
#include "spdk/pos.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/qos_array_manager_mock.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/qos/rate_limit_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::AtLeast;
using ::testing::Return;


namespace pos
{
TEST(QosVolumeManager, QosVolumeManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
}

TEST(QosVolumeManager, QosVolumeManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager* qosVolumeManager = new QosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
    delete qosVolumeManager;
}

TEST(QosVolumeManager, VolumeCreated_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
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
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
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
#if 0
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
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
    actual = qosVolumeManager.VolumeMounted(&volumeBase, &volumePerf, &volumeArrayInfo);qosVolumeManager.VolumeMounted
    ASSERT_EQ(expected, actual);
#endif
}

TEST(QosVolumeManager, VolumeUnmounted_Default_Parameters)
{
#if 0
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
    VolumeEventBase volumeBase;
    volumeBase.volId = 0;
    volumeBase.volName = "vol1";
    VolumeArrayInfo volumeArrayInfo;
    volumeArrayInfo.arrayId = 0;
    volumeArrayInfo.arrayName = "POSArray1";
    bool expected = true, actual;
    actual = qosVolumeManager.VolumeUnmounted(&volumeBase, &volumeArrayInfo);
    ASSERT_EQ(expected, actual);
#endif
}

TEST(QosVolumeManager, VolumeLoaded_Default_Parameters)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
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
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
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
#if 0
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
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
#endif
}
TEST(QosVolumeManager, HandlePosIoSubmission_Test)
{
#if 0
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
    AioSubmissionAdapter aioSubmission;
    pos_io io;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    ON_CALL(mockEventFrameworkApi, GetCurrentReactor()).WillByDefault(Return(0));
    qosVolumeManager.HandlePosIoSubmission(&aioSubmission, &io);
#endif
}

TEST(QosVolumeManager, DequeueParams_Test)
{
}

TEST(QosVolumeManager, VolumeQosPoller_Test)
{
}

TEST(QosVolumeManager, Check_Getter_And_Setter_VolumeLimit)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
    uint32_t reactor = 0;
    uint32_t volId = 0;
    int64_t value = 100;
    bool iops = true;
    qosVolumeManager.SetVolumeLimit(reactor, volId, value, iops);
    int64_t retVal = qosVolumeManager.GetVolumeLimit(reactor, volId, iops);
    ASSERT_EQ(value, retVal);
}

TEST(QosVolumeManager, Check_Getter_Setter_Subsystem_VolumeMap)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);

    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
    uint32_t nqnId = 1;
    uint32_t volId = 1;

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
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);

    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);

    NiceMock<MockBwIopsRateLimit> mockBwIopsRateLimit;
    uint32_t reactor = 1;
    int volId = 1;
    double offset = 100;
    //EXPECT_CALL(mockBwIopsRateLimit, ResetRateLimit);
    qosVolumeManager.ResetRateLimit(reactor, volId, offset);
}

TEST(QosVolumeManager, Check_Getter_And_Setter_ArrayName)
{
    NiceMock<MockQosContext> mockQoscontext;
    uint32_t arrayIndex = 0;
    NiceMock<MockQosArrayManager> mockQosArrayManager(arrayIndex, &mockQoscontext);
    bool feQosEnabled = true;
    QosVolumeManager qosVolumeManager(&mockQoscontext, feQosEnabled, arrayIndex, &mockQosArrayManager);
    std::string arrayName = "POSArray1";
    qosVolumeManager.SetArrayName(arrayName);
    std::string returnedArrayName = qosVolumeManager.GetArrayName();
    int expectedRetVal = 0;
    int actualRetVal = arrayName.compare(returnedArrayName);
    ASSERT_EQ(expectedRetVal, actualRetVal);
}

} // namespace pos
