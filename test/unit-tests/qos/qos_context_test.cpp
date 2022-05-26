#include "src/qos/qos_context.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "test/unit-tests/qos/user_policy_all_volumes_mock.h"

using namespace std;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(QosContext, QosContext_Constructor_One_Stack)
{
    QosContext qosContext();
}
TEST(QosContext, QosContext_Constructor_One_Heap)
{
    QosContext* qosContext = new QosContext();
    delete qosContext;
}

TEST(QosContext, GetQosUserPolicy_Getter)
{
    QosContext qosContext;
    QosUserPolicy qosUserPolicy = qosContext.GetQosUserPolicy();
    ASSERT_NE(&qosUserPolicy, NULL);
}

TEST(QosContext, GetQosParameters_Getter)
{
    QosContext qosContext;
    QosParameters qosParameters = qosContext.GetQosParameters();
    ASSERT_NE(&qosParameters, NULL);
}

TEST(QosContext, GetQosCorrection_Getter)
{
    QosContext qosContext;
    QosCorrection qosCorrection = qosContext.GetQosCorrection();
    ASSERT_NE(&qosCorrection, NULL);
}

TEST(QosContext, Check_Reset_Get_Insert_ActiveVolume)
{
    QosContext qosContext;
    uint32_t volId = 0;
    qosContext.InsertActiveVolume(volId);
    std::map<uint32_t, uint32_t> activeVolumeList = qosContext.GetActiveVolumes();
    size_t noofElements = activeVolumeList.size();
    ASSERT_EQ(noofElements, 1);
    ASSERT_EQ(activeVolumeList[volId], 1);
    qosContext.ResetActiveVolume();

    activeVolumeList = qosContext.GetActiveVolumes();
    noofElements = activeVolumeList.size();
    ASSERT_EQ(noofElements, 0);
}

TEST(QosContext, Check_ResetGetInsert_ActiveReactorVolume)
{
    QosContext qosContext;
    uint32_t reactor = 1;
    uint32_t volId = 0;

    // check insert
    qosContext.InsertActiveReactorVolume(reactor, volId);

    // check get count
    // since only 1 element inserted size should be 1
    uint32_t size = qosContext.GetActiveReactorVolumeCount();
    ASSERT_EQ(size, 1);

    // check reset. After reset size should be zero
    qosContext.ResetActiveReactorVolume();
    size = qosContext.GetActiveReactorVolumeCount();
    ASSERT_EQ(size, 0);
}
TEST(QosContext, Check_InsertGet_activeVolReactorMap)
{
    QosContext qosContext;
    uint32_t reactor = 1;
    uint32_t volId = 2;
    uint32_t count = 3;
    map<uint32_t, uint32_t> reactorCount;
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMap;
    reactorCount[reactor] = count;
    volReactorMap[volId] = reactorCount;
    qosContext.InsertActiveVolumeReactor(volReactorMap);
    std::map<uint32_t, map<uint32_t, uint32_t>> volReactorMapReturned = qosContext.GetActiveVolumeReactors();
    map<uint32_t, uint32_t> reactorCountReturned;
    reactorCountReturned = volReactorMapReturned[volId];
    ASSERT_EQ(reactorCountReturned[reactor], count);
}

TEST(QosContext, IsVolumeMinPolicyInEffect_volMinPolicyDisabled)
{
    QosContext qosContext;
    QosUserPolicy &mockQosUserPolicy = qosContext.GetQosUserPolicy();
    AllVolumeUserPolicy &mockAllVolumeUserPolicy = mockQosUserPolicy.GetAllVolumeUserPolicy();
    mockAllVolumeUserPolicy.SetMinimumPolicyInEffect(false);
    bool expected = false;
    bool actual;
    actual = qosContext.IsVolumeMinPolicyInEffect();
    ASSERT_EQ(expected, actual);
}
TEST(QosContext, IsVolumeMinPolicyInEffect_volMinPolicyEnabled)
{
    QosContext qosContext;
    QosUserPolicy &mockQosUserPolicy = qosContext.GetQosUserPolicy();
    AllVolumeUserPolicy &mockAllVolumeUserPolicy = mockQosUserPolicy.GetAllVolumeUserPolicy();
    mockAllVolumeUserPolicy.SetMinimumPolicyInEffect(true);
    bool expectedVal = true;
    bool actualVal;
    actualVal = qosContext.IsVolumeMinPolicyInEffect();
    ASSERT_EQ(expectedVal, actualVal);
}
TEST(QosContext, Check_GetterandSetterforApplyCorrection)
{
    QosContext qosContext;
    bool applyCorrection = false;
    qosContext.SetApplyCorrection(applyCorrection);
    bool actualApplyCorrection = qosContext.GetApplyCorrection();
    ASSERT_EQ(applyCorrection, actualApplyCorrection);
}

TEST(QosContext, Check_IncrementAndIsCorrectionCyleFunctions)
{
    QosContext qosContext;
    int i = 0;
    for (i = 0; i < 102; i++)
    {
        qosContext.IncrementCorrectionCycle();
    }
    bool expected = true;
    bool actual;
    actual = qosContext.IsCorrectionCycleOver();
    ASSERT_EQ(expected, actual);
}

TEST(QosContext, Check_IncrementAndIsCorrectionCyleOverFalse)
{
    QosContext qosContext;
    int i = 0;
    for (i = 0; i < 10; i++)
    {
        qosContext.IncrementCorrectionCycle();
    }
    bool expected = false;
    bool actual;
    actual = qosContext.IsCorrectionCycleOver();
    ASSERT_EQ(expected, actual);
}

TEST(QosContext, Check_GetterSetter_TotalConnection)
{
    QosContext qosContext;
    uint32_t volId = 0;
    uint32_t setValue = 10;
    qosContext.SetTotalConnection(volId, setValue);
    uint32_t returnedValue;
    returnedValue = qosContext.GetTotalConnection(volId);
    ASSERT_EQ(setValue, returnedValue);
}

TEST(QosContext, Check_UpdateandGetReactorCoreListFunction)
{
    QosContext qosContext;
    uint32_t reactorCore = 6;
    qosContext.UpdateReactorCoreList(reactorCore);
    std::vector<uint32_t> reactorList;
    reactorList = qosContext.GetReactorCoreList();
    size_t noOfElements = reactorList.size();
    // since the update function has been called only once, size of vector should be 1.
    ASSERT_EQ(noOfElements, 1);

    ASSERT_EQ(reactorList[noOfElements - 1], reactorCore);
}
TEST(QosContext, Test_SetResetReactorProcessed)
{
    QosContext qosContext;
    uint32_t reactorId = 6;
    bool value = true;

    qosContext.UpdateReactorCoreList(reactorId);
    qosContext.SetReactorProcessed(reactorId, true);
    bool expected = true;
    bool actual = qosContext.AllReactorsProcessed();
    ASSERT_EQ(expected, actual);
    qosContext.ResetAllReactorsProcessed();
    actual =  qosContext.AllReactorsProcessed();
    ASSERT_EQ(actual, false);
}

TEST(QosContext, Test_GetSetVolumeOperationDone)
{
    QosContext qosContext;
    qosContext.SetVolumeOperationDone(false);
    bool expected = false;
    bool actual = qosContext.GetVolumeOperationDone();
    ASSERT_EQ(actual, false);
}
TEST(QosContext, InactiveReactorInsertandGet)
{
    QosContext qosContext;
    std::map<uint32_t, vector<uint32_t>> inactiveReactors;
    std::vector<uint32_t> volumeInactiveReactors;
    volumeInactiveReactors.push_back(1);
    volumeInactiveReactors.push_back(2);
    inactiveReactors[0].push_back(1);
    inactiveReactors[0].push_back(2);
    qosContext.InsertInactiveReactors(inactiveReactors);
    std::vector<uint32_t> recdInactiveReactor;
    recdInactiveReactor = qosContext.GetInactiveReactorsList(0);
    ASSERT_EQ(recdInactiveReactor, volumeInactiveReactors);
}
} // namespace pos
