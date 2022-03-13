#include "src/qos/qos_manager.h"

#include <gtest/gtest.h>
#include "test/unit-tests/master_context/config_manager_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_pos_nvmf_caller_mock.h"
#include "test/unit-tests/spdk_wrapper/caller/spdk_env_caller_mock.h"
#include "test/unit-tests/qos/submission_adapter_mock.h"
#include "test/unit-tests/qos/submission_notifier_mock.h"
#include "src/io/frontend_io/aio_submission_adapter.h"
#include "test/unit-tests/spdk_wrapper/event_framework_api_mock.h"
#include <unistd.h>

using ::testing::_;
using ::testing::NiceMock;
namespace pos
{
ACTION_P(SetArg2ToBoolAndReturn0, boolValue)
{
    *static_cast<bool*>(arg2) = boolValue;
    return 0;
}

TEST(QosManager, QosManager_Constructor_One_Stack)
{
    QosManager qosManager();
}

TEST(QosManager, QosManager_Constructor_One_Heap)
{
    QosManager* qosManager = new QosManager();
    delete qosManager;
}

TEST(QosManager, Initialize_Test)
{
    QosManager qosManager;
    qosManager.Initialize();
    // try to initialize again, it will return from the first if.
    qosManager.Initialize();
}
TEST(QosManager, Initialize_Test2)
{
    QosManager* qosManager = new QosManager();
    qosManager->Initialize();
    usleep(100);
    delete qosManager;
}

TEST(QosManager, IsFeQosEnabled_heap)
{
    QosManager* qosManager = new QosManager();
    qosManager->IsFeQosEnabled();
    delete qosManager;
}

TEST(QosManager, IsFeQosEnabled_stack)
{
    QosManager qosManager;
    qosManager.IsFeQosEnabled();
}

TEST(QosManager, DequeueEventParams_Run)
{
    QosManager qosManager;
    uint32_t workerId = 1;
    BackendEvent eventId = BackendEvent_GC;
    qosManager.DequeueEventParams(workerId, eventId);
}

TEST(QosManager, Test_Getter_Setter_EventWeightWRR)
{
    QosManager* qosManager = new QosManager();
    BackendEvent eventId = BackendEvent_GC;
    int64_t weight = 100;
    qosManager->SetEventWeightWRR(eventId, weight);
    int64_t retWeight = qosManager->GetEventWeightWRR(eventId);
    ASSERT_EQ(weight, retWeight);
    delete qosManager;
}

TEST(QosManager, Test_Getter_Default_EventWeightWRR)
{
    BackendEvent eventId = BackendEvent_GC;
    QosManager qosManager;
    int64_t retWeight = qosManager.GetDefaultEventWeightWRR(eventId);
    ASSERT_EQ(retWeight, M_DEFAULT_WEIGHT);
}

TEST(QosManager, Test_Getter_Setter_StripeCnt)
{
    QosManager qosManager;
    uint32_t arrayId = 0;
    std::string arrayName = "POSArray";
    qosManager.UpdateArrayMap(arrayName);
    uint32_t InitStripeCount = qosManager.GetUsedStripeCnt(arrayId);
    qosManager.IncreaseUsedStripeCnt(arrayId);
    qosManager.IncreaseUsedStripeCnt(arrayId);
    uint32_t stripeCount = qosManager.GetUsedStripeCnt(arrayId);
    ASSERT_EQ(stripeCount, 2 + InitStripeCount);
    qosManager.DecreaseUsedStripeCnt(arrayName);
    stripeCount = qosManager.GetUsedStripeCnt(arrayId);
    ASSERT_EQ(stripeCount, 1 + InitStripeCount);
}

TEST(QosManager, Test_Getter_Setter_SubsystemToVolumeMap)
{
    QosManager qosManager;
    std::string arrayName = "POSArray";
    uint32_t nqnId = 1;
    uint32_t volId = 1;
    qosManager.UpdateArrayMap(arrayName);
    qosManager.UpdateSubsystemToVolumeMap(nqnId, volId, arrayName);
    uint32_t arrayId = 0;
    std::vector<int> volVector = qosManager.GetVolumeFromActiveSubsystem(nqnId, arrayId);
    std::vector<int>::iterator it;
    it = std::find(volVector.begin(), volVector.end(), volId);
    ASSERT_NE(it, volVector.end());
    qosManager.DeleteVolumeFromSubsystemMap(nqnId, volId, arrayName);
    std::vector<int> volVector2 = qosManager.GetVolumeFromActiveSubsystem(nqnId, arrayId);
    std::vector<int>::iterator it2;
    it2 = std::find(volVector2.begin(), volVector2.end(), volId);
    ASSERT_EQ(it2, volVector2.end());
}

TEST(QosManager, Test_Getter_Setter_LogEvent)
{
     QosManager qosManager;
     BackendEvent eventId = BackendEvent_Flush;
     uint32_t InitCount = qosManager.GetEventLog(eventId);
     qosManager.LogEvent(eventId);
     uint32_t count = qosManager.GetEventLog(eventId);
     ASSERT_EQ(count, InitCount+1);
}

TEST(QosManager, Test_IsMinimumPolicyInEffectInSystem)
{
    QosManager qosManager;
    bool actual = qosManager.IsMinimumPolicyInEffectInSystem();
    ASSERT_EQ(actual, false);
}

TEST(QosManager, Test_ResetCorrection)
{
    QosManager qosManager;
    qosManager.ResetCorrection();
}
TEST(QosManager, Test_Setter_Getter_volume_Policy)
{
    QosManager qosManager;
    std::string arrayName = "Posarray";
    qosManager.UpdateArrayMap(arrayName);
    uint32_t arrayId = 0;
    uint32_t volId = 0;
    qos_vol_policy volPolicy;
    volPolicy.minBwGuarantee = true;
    int actual = qosManager.UpdateVolumePolicy(volId, volPolicy, arrayId);
    ASSERT_EQ(actual, 0);
    qos_vol_policy retPolicy = qosManager.GetVolumePolicy(volId, arrayName);
    ASSERT_EQ(volPolicy.minBwGuarantee, true);
}

TEST(QosManager, Test_Getter_Setter_Pending_BackendEvent_fe_qos_false)
{
    QosManager qosManager;
    BackendEvent event = BackendEvent_Flush;
    qosManager.IncreasePendingBackendEvents(event);
    int actual = qosManager.GetPendingBackendEvents(event);
    qosManager.DecreasePendingBackendEvents(event);
    actual = qosManager.GetPendingBackendEvents(event);
}

TEST(QosManager, Test_Getter_Setter_VolumeLimit)
{
    QosManager qosManager;
    uint32_t volId = 0;
    int64_t weight = 10;
    bool iops = true;
    uint32_t arrayId = 1;
    qosManager.SetVolumeLimit(volId, weight, iops, arrayId);
    int64_t retWeight = qosManager.GetVolumeLimit(volId, iops, arrayId);
    ASSERT_EQ(retWeight, weight);
}
TEST(QosManager, Test_Getter_Setter_GcFreeSegment)
{
    QosManager qosManager;
    uint32_t freeSegments = 10;
    uint32_t arrayId = 1;
    qosManager.SetGcFreeSegment(freeSegments, arrayId);
    int64_t retFreeSegments = qosManager.GetGcFreeSegment(arrayId);
    ASSERT_EQ(retFreeSegments, freeSegments);
}

TEST(QosManager, Test_Getter_Setter_ArrayId_ArrayName_Map)
{
    QosManager qosManager;
    std::string arrayName = "POSArray";
    qosManager.UpdateArrayMap(arrayName);
    qosManager.UpdateArrayMap(arrayName);
    uint32_t arrayId = 0;
    uint32_t retArrayId = qosManager.GetArrayIdFromMap(arrayName);
    ASSERT_EQ(retArrayId, arrayId);
    std::string retArrayName = qosManager.GetArrayNameFromMap(arrayId);
    ASSERT_EQ(retArrayName, arrayName);
    uint32_t noOfArrays = qosManager.GetNumberOfArrays();
    ASSERT_EQ(noOfArrays, 1);
}

TEST(QosManager, Test_Getter_ContentionCycles)
{
    QosManager qosManager;
    uint32_t contentionCycles = qosManager.GetNoContentionCycles();
    ASSERT_EQ(contentionCycles, NO_CONTENTION_CYCLES);
}

TEST(QosManager, Test_Getter_SubsystemVolumeMap)
{
    QosManager qosManager;
    std::unordered_map<int32_t, std::vector<int>> subsysVolMap;
    uint32_t arrayId = 0;
    qosManager.GetSubsystemVolumeMap(subsysVolMap, arrayId);
}

TEST(QosManager, Test_Getter_GetVolumePolicyMap)
{
    QosManager qosManager;
    std::map<uint32_t, qos_vol_policy> volumePolicyMapCopy;
    uint32_t arrayId = 0;
    qosManager.GetVolumePolicyMap(arrayId, volumePolicyMapCopy);
}

TEST(QosManager, Test_Getter_UpdateRebuildPolicy)
{
    QosManager qosManager;
    qos_rebuild_policy rebuildPolicy;
    rebuildPolicy.rebuildImpact = PRIORITY_HIGH;
    std::string arrayName = "POSArray";
    qosManager.UpdateArrayMap(arrayName);
    uint32_t arrayId = 0;
    qosManager.UpdateRebuildPolicy(rebuildPolicy);
    qos_rebuild_policy retRebuildPolicy = qosManager.GetRebuildPolicy(arrayName);
    ASSERT_EQ(retRebuildPolicy.rebuildImpact, rebuildPolicy.rebuildImpact);
}
NiceMock<MockConfigManager>*
CreateQosMockConfigManager(bool isQosEnabled)
{
    NiceMock<MockConfigManager>* configManager = new NiceMock<MockConfigManager>;

    ON_CALL(*configManager, GetValue("fe_qos", "enable", _, _)).WillByDefault(SetArg2ToBoolAndReturn0(isQosEnabled));
    return configManager;
}

TEST(QosManager, Test_HandlePosIoSubmission)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateQosMockConfigManager(false);
    NiceMock<MockSpdkPosNvmfCaller>* mockSpdkPosNvmfCaller =
        new NiceMock<MockSpdkPosNvmfCaller>;
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller =
        new NiceMock<MockSpdkEnvCaller>();
    QosManager qosManager(mockSpdkEnvCaller, mockSpdkPosNvmfCaller, mockConfigManager);
    std::string arrayName = "POSArray";
    qosManager.UpdateArrayMap(arrayName);
    uint32_t arrayId = 0;
    pos_io io;
    io.volume_id = 1;
    io.length = 10;
    io.ioType = IO_TYPE::READ;
    io.array_id = 0;
    io.arrayName = new char[9] {"POSArray"};
    AioSubmissionAdapter aioSubmission;
    VolumeIoSmartPtr volIo(new VolumeIo(nullptr, 8, 0));
    volIo->dir = UbioDir::Write;
    qosManager.HandlePosIoSubmission(&aioSubmission, volIo);
    delete mockConfigManager;
}

TEST(QosManager, Test_HandleEventUbioSubmission)
{
    QosManager qosManager;
    std::string arrayName = "POSArray";
    qosManager.UpdateArrayMap(arrayName);
    uint32_t arrayId = 0;
    NiceMock<MockSubmissionNotifier> mockSubmissionNotifier;
    int buf[10];
    UbioSmartPtr ubio = std::make_shared<Ubio>((void*)buf, 10, 0);
    uint32_t id = 1;
    NiceMock<MockSubmissionAdapter> mockSubmissionAdapter;
    qosManager.HandleEventUbioSubmission(&mockSubmissionAdapter, &mockSubmissionNotifier, id, ubio);
}

TEST(QosManager, Test_Delete_ArrayName_Map)
{
    QosManager qosManager;
    std::string arrayName = "POSArray";
    qosManager.UpdateArrayMap(arrayName);
    qosManager.DeleteEntryArrayMap(arrayName);
    uint32_t noOfArrays = qosManager.GetNumberOfArrays();
    ASSERT_EQ(noOfArrays, 0);
}

TEST(QosManager, Initialize_FinalizeSpdkManager_Test)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateQosMockConfigManager(false);
    NiceMock<MockSpdkPosNvmfCaller>* mockSpdkPosNvmfCaller =
        new NiceMock<MockSpdkPosNvmfCaller>;
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller =
        new NiceMock<MockSpdkEnvCaller>();
     NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    QosManager qosManager(mockSpdkEnvCaller, mockSpdkPosNvmfCaller, mockConfigManager, &mockEventFrameworkApi);
    qosManager.InitializeSpdkManager();
    qosManager.FinalizeSpdkManager();
    delete mockConfigManager;
}

TEST(QosManager, PeriodicalJob)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateQosMockConfigManager(false);
    NiceMock<MockSpdkPosNvmfCaller>* mockSpdkPosNvmfCaller =
        new NiceMock<MockSpdkPosNvmfCaller>;
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller =
        new NiceMock<MockSpdkEnvCaller>();
     NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    QosManager qosManager(mockSpdkEnvCaller, mockSpdkPosNvmfCaller, mockConfigManager, &mockEventFrameworkApi);
    uint64_t next_tick;
    qosManager.PeriodicalJob(&next_tick);
    delete mockConfigManager;
}

TEST(QosManager, SetMinimumVolume)
{
    NiceMock<MockConfigManager>* mockConfigManager = CreateQosMockConfigManager(false);
    NiceMock<MockSpdkPosNvmfCaller>* mockSpdkPosNvmfCaller =
        new NiceMock<MockSpdkPosNvmfCaller>;
    NiceMock<MockSpdkEnvCaller>* mockSpdkEnvCaller =
        new NiceMock<MockSpdkEnvCaller>();
     NiceMock<MockEventFrameworkApi> mockEventFrameworkApi;
    QosManager qosManager(mockSpdkEnvCaller, mockSpdkPosNvmfCaller, mockConfigManager, &mockEventFrameworkApi);
    uint64_t next_tick;
    qosManager.SetMinimumVolume(0, 1, 10, false);
    qosManager.SetMinimumVolume(0, 1, 10, true);
    delete mockConfigManager;
}

} // namespace pos
