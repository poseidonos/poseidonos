#include "src/qos/qos_array_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "src/include/backend_event.h"
#include "src/qos/qos_common.h"
#include "test/unit-tests/qos/qos_context_mock.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include "test/unit-tests/qos/qos_manager_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
namespace pos
{
TEST(QosArrayManager, QosArrayManager_Constructor_One_Stack)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
}

TEST(QosArrayManager, QosArrayManager_Constructor_One_Heap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager* qosArrayManager = new QosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    delete qosArrayManager;
}

TEST(QosArrayManager, Check_Update_And_Get_VolumePolicy)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t volId = 0;
    qos_vol_policy setPolicy;
    setPolicy.maxIops = 1000;
    setPolicy.maxBw = 1000;
    qosArrayManager.UpdateVolumePolicy(volId, setPolicy);
    qos_vol_policy retPolicy = qosArrayManager.GetVolumePolicy(volId);
    ASSERT_EQ(retPolicy.maxIops, setPolicy.maxIops);
    ASSERT_EQ(retPolicy.maxBw, setPolicy.maxBw);
    bool expected = true, actual;
    actual = qosArrayManager.IsVolumePolicyUpdated();
    ASSERT_EQ(expected, actual);

    std::map<uint32_t, qos_vol_policy> volumePolicyMapCopy;
    qosArrayManager.GetVolumePolicyMap(volumePolicyMapCopy);
    qos_vol_policy volPolicy = volumePolicyMapCopy[volId];
    expected = false;
    actual = qosArrayManager.IsVolumePolicyUpdated();
    ASSERT_EQ(expected, actual);
}

TEST(QosArrayManager, Check_Update_And_GetUsedStripeCnt)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t oldUsedStripeCnt = qosArrayManager.GetUsedStripeCnt();
    qosArrayManager.IncreaseUsedStripeCnt();
    uint32_t newUsedStripeCnt = qosArrayManager.GetUsedStripeCnt();
    ASSERT_EQ(newUsedStripeCnt, oldUsedStripeCnt + 1);
    qosArrayManager.DecreaseUsedStripeCnt();
    newUsedStripeCnt = qosArrayManager.GetUsedStripeCnt();
    ASSERT_EQ(newUsedStripeCnt, oldUsedStripeCnt);
}

TEST(QosArrayManager, Check_Get_And_Update_RebuildPolicy)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    qos_rebuild_policy setPolicy;
    setPolicy.rebuildImpact = PRIORITY_HIGH;
    setPolicy.policyChange = true;
    qosArrayManager.UpdateRebuildPolicy(setPolicy);
    qos_rebuild_policy retPolicy;
    retPolicy = qosArrayManager.GetRebuildPolicy();
    ASSERT_EQ(retPolicy.rebuildImpact, setPolicy.rebuildImpact);
    ASSERT_EQ(retPolicy.policyChange, setPolicy.policyChange);
}

TEST(QosArrayManager, Check_Set_And_Get_VolumeLimit)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t volId = 1;
    int64_t weight = 100;
    bool iops = true;
    qosArrayManager.SetVolumeLimit(volId, weight, iops);
    int64_t retWeight = qosArrayManager.GetVolumeLimit(volId, iops);
    ASSERT_EQ(weight, retWeight);
}

TEST(QosArrayManager, Check_Get_And_Set_GC_Free_Segments)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t freeSegments = 100;
    qosArrayManager.SetGcFreeSegment(freeSegments);
    uint32_t retFreeSegments = qosArrayManager.GetGcFreeSegment();
    ASSERT_EQ(retFreeSegments, freeSegments);
}

TEST(QosArrayManager, Check_Set_And_Get_UpdateSubsystemToVolumeMap)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint32_t nqnId = 1;
    uint32_t volId = 1;

    qosArrayManager.UpdateSubsystemToVolumeMap(nqnId, volId);
    std::vector<int> volList = qosArrayManager.GetVolumeFromActiveSubsystem(nqnId);
    std::vector<int>::iterator position = std::find(volList.begin(), volList.end(), volId);
    ASSERT_NE(position, volList.end());
    std::unordered_map<int32_t, std::vector<int>> subsysVolMap;
    qosArrayManager.GetSubsystemVolumeMap(subsysVolMap);
    position = std::find(subsysVolMap[nqnId].begin(), subsysVolMap[nqnId].end(), volId);
    ASSERT_NE(position, subsysVolMap[nqnId].end());
    qosArrayManager.DeleteVolumeFromSubsystemMap(nqnId, volId);
    volList = qosArrayManager.GetVolumeFromActiveSubsystem(nqnId);
    position = std::find(volList.begin(), volList.end(), volId);
    ASSERT_EQ(position, volList.end());
}

TEST(QosArrayManager, SetArrayName_TestRun)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    std::string name = "POSArray2";
    qosArrayManager.SetArrayName(name);
}

TEST(QosArrayManager, IsMinimumPolicyInEffect)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = true;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    bool minPolicy = qosArrayManager.IsMinimumPolicyInEffect();
    ASSERT_EQ(minPolicy, false);
}

TEST(QosArrayManager, HandlePosIoSubmissionTest)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = false;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    AioSubmissionAdapter aioSubmission;
    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Write;
    qosArrayManager.HandlePosIoSubmission(&aioSubmission, volIo);
}

TEST(QosArrayManager, GetDynamicVolumeThrottling)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = false;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    uint64_t iops = qosArrayManager.GetDynamicVolumeThrottling(1, true);
    uint64_t bw = qosArrayManager.GetDynamicVolumeThrottling(1, false);
    ASSERT_EQ(bw, iops);
    ASSERT_EQ(bw, 0);
}

TEST(QosArrayManager, ResetVolumeThrottling)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = false;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    qosArrayManager.ResetVolumeThrottling();
}

TEST(QosArrayManager, SetMinimumVolume)
{
    NiceMock<MockQosContext> mockQoscontext;
    NiceMock<MockQosManager> mockQosManager;
    NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    uint32_t arrayIndex = 1;
    bool feQosEnabled = false;
    QosArrayManager qosArrayManager(arrayIndex, &mockQoscontext, feQosEnabled, &mockEventFrameworkApi, &mockQosManager);
    qosArrayManager.SetMinimumVolume(1, 100, true);
    qosArrayManager.SetMinimumVolume(1, 100, false);
}

} // namespace pos
