#include "src/array_mgmt/array_manager.h"

#include <gtest/gtest.h>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/gc/garbage_collector_mock.h"
#include "test/unit-tests/state/state_control_mock.h"
#include "test/unit-tests/array_components/array_components_mock.h"
#include "test/unit-tests/array_components/components_info_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/rebuild/array_rebuilder_mock.h"
#include "test/unit-tests/state/state_manager_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_client_mock.h"
#include "test/unit-tests/utils/mock_builder.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/array/build/array_builder_adapter_mock.h"
#include "test/unit-tests/array/build/array_build_info_mock.h"

using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
TEST(ArrayManager, ArrayManager_testUsingShortConstructor)
{
    // Given
    // When
    ArrayManager arrayMgr;

    // Then
}

TEST(ArrayManager, ArrayManager_testDestructorDeletingComponents)
{
    // Given
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(nullptr);
    EXPECT_CALL(mockDevMgr, SetDeviceEventCallback).Times(1);

    // When
    ArrayManager arrayMgr(mockArrayRebuilder, &mockDevMgr, nullptr, nullptr);

    // Then
}

TEST(ArrayManager, ArrayManager_testIfDeviceManagerRegistersArrayManager)
{
    // Given
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);

    EXPECT_CALL(mockDevMgr, SetDeviceEventCallback).Times(1);

    // When
    ArrayManager arrayMgr(nullptr, &mockDevMgr, nullptr, nullptr);

    // Then: verify the expect_call
}

TEST(ArrayManager, ArrayManager_testDestructor)
{
    // Given
    auto mockArrayComp = NewMockArrayComponents("array1");
    map<string, ArrayComponents*> arrayMap =
    {
        {"array1", mockArrayComp},
        {"array2", nullptr}
    };
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    delete arrayMgr;

    // Then
}

TEST(ArrayManager, Create_testIfFailsToCreateArrayWithExistingName)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->Create(existingArray, DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_SAME_ARRAY_NAME_EXISTS), actual);
}

TEST(ArrayManager, Create_testIfFailsToCreateArrayWhenMaxArrayCntIsReached)
{
    // Given
    NiceMock<MockStateManager> mockStateMgr;
    map<string, ArrayComponents*> arrayMap;
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        string arrayName = "array" + i;
        auto mockArrayComp = BuildMockArrayComponents(arrayName);
        arrayMap.emplace(arrayName, mockArrayComp.get());
    }
    ArrayManager* arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->Create("new-array", DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_EXCEED_MAX_NUM_OF_ARRAYS), actual);
}

TEST(ArrayManager, Create_testIfArrayObjectIsFreedAndArrayMapNotUpdatedWhenCreationFails)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = NewMockArrayComponents(arrayName);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockArrayComponentFactory = [&mockArrayComp](string name, IArrayRebuilder* arrayRebuilder)
    {
        return mockArrayComp;
    };
    int CREATION_FAILURE = 1234;
    auto mockArraybuilder = std::make_shared<MockArrayBuilderAdapter>();
    auto mockArrayBuildInfo = new MockArrayBuildInfo();
    mockArrayBuildInfo->buildResult = CREATION_FAILURE;
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, mockArrayComponentFactory, mockArraybuilder.get());
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockArraybuilder, Create).WillOnce(Return(mockArrayBuildInfo));

    // When
    int actual = arrayMgr->Create(arrayName, DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(CREATION_FAILURE, actual);
    ASSERT_EQ(0, arrayMgr->GetInfo().size());
}

TEST(ArrayManager, Create_testIfArrayMapUpdatedWhenCreationSucceeds)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = BuildMockArrayComponents(arrayName);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockTelClient = BuildMockTelemetryClient();
    auto mockArrayComponentFactory = [&mockArrayComp](string name, IArrayRebuilder* arrayRebuilder)
    {
        return mockArrayComp.get();
    };
    auto mockArraybuilder = std::make_shared<MockArrayBuilderAdapter>();
    auto mockArrayBuildInfo = new MockArrayBuildInfo();
    mockArrayBuildInfo->buildResult = 0;

    auto arrayMgr = new ArrayManager(nullptr, nullptr, mockTelClient.get(), mockArrayComponentFactory, mockArraybuilder.get());
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockArraybuilder, Create).WillOnce(Return(mockArrayBuildInfo));            // success
    EXPECT_CALL(*mockArrayComp, Import).WillOnce(Return(0));            // success
    // EXPECT_CALL(*mockTelClient, RegisterPublisher).WillOnce(Return(0)); // success

    // When
    int actual = arrayMgr->Create(arrayName, DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(1, arrayMgr->GetInfo().size());
}

TEST(ArrayManager, Delete_testIfErrorIsReturnedWhenGivenArrayNameIsWrong)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    // When
    int actual = arrayMgr->Delete(arrayName);

    // Then
    ASSERT_EQ(EID(DELETE_ARRAY_ARRAY_NAME_DOES_NOT_EXIST), actual);
}

TEST(ArrayManager, Delete_testIfArrayComponentMapIsUpdatedWhenDeleteSucceeds)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = NewMockArrayComponents(existingArray); // use New* instead of Build* since arrayMgr->Delete() actually tries to delete
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp);
    auto mockTelClient = BuildMockTelemetryClient();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, mockTelClient.get(), nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    const int DELETE_SUCCESS = EID(SUCCESS);
    EXPECT_CALL(*mockArrayComp, Delete).WillOnce(Return(DELETE_SUCCESS));

    // When
    int actual = arrayMgr->Delete(existingArray);

    // Then
    ASSERT_EQ(DELETE_SUCCESS, actual);
    ASSERT_EQ(0, arrayMgr->GetInfo().size());
}

TEST(ArrayManager, Mount_testIfTargetArrayCallsMount)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto mockTelClient = BuildMockTelemetryClient();

    auto arrayMgr = new ArrayManager(nullptr, nullptr, mockTelClient.get(), nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    const int MOUNT_SUCCESS = 0;
    EXPECT_CALL(*mockArrayComp, Mount).WillOnce(Return(MOUNT_SUCCESS));
    EXPECT_CALL(*mockTelClient, RegisterPublisher).WillOnce(Return(0));  // success
    // When
    int actual = arrayMgr->Mount(existingArray, false);

    // Then
    ASSERT_EQ(MOUNT_SUCCESS, actual);
}

TEST(ArrayManager, Mount_testIfErrorIsReturnedWhenGivenArrayNameIsWrong)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockTelClient = BuildMockTelemetryClient();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, mockTelClient.get(), nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    // When
    int actual = arrayMgr->Mount(arrayName, false);

    // Then
    ASSERT_EQ(EID(MOUNT_ARRAY_ARRAY_NAME_DOES_NOT_EXIST), actual);
}

TEST(ArrayManager, Unmount_testIfTargetArrayCallsUnmount)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto mockTelClient = BuildMockTelemetryClient();

    EXPECT_CALL(*mockArrayComp, Unmount).WillOnce(Return(EID(SUCCESS)));

    auto arrayMgr = new ArrayManager(nullptr, nullptr, mockTelClient.get(), nullptr);

    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->Unmount(existingArray);

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
}

TEST(ArrayManager, AddDevice_testIfTargetArrayCallsAddSpare)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto mockArray = BuildMockArray(existingArray);

    EXPECT_CALL(*mockArrayComp, GetArray).WillOnce(Return(mockArray.get()));
    EXPECT_CALL(*mockArray, AddSpare).WillOnce(Return(EID(SUCCESS)));

    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->AddDevice(existingArray, "some-dev");

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
}

TEST(ArrayManager, RemoveDevice_testIfTargetArrayCallsRemoveSpare)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto mockArray = BuildMockArray(existingArray);

    EXPECT_CALL(*mockArrayComp, GetArray).WillOnce(Return(mockArray.get()));
    EXPECT_CALL(*mockArray, RemoveSpare).WillOnce(Return(EID(SUCCESS)));

    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->RemoveDevice(existingArray, "some-dev");

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
}

TEST(ArrayManager, DeviceDetached_testIfTargetArrayCalls)
{
    // Given
    string existingArray = "array1";
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto mockArray = BuildMockArray(existingArray);
    EXPECT_CALL(*mockArrayComp, GetArray).WillRepeatedly(Return(mockArray.get()));
    EXPECT_CALL(*mockArray, DetachDevice).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*mockArray, FindDevice).WillOnce(Return(mockArrDev));
    auto mockUBlockSharedPtr = BuildMockUBlockDevice("mock-ublock", "sn");
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->DeviceDetached(mockUBlockSharedPtr);

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);

    delete mockArrDev;
}

TEST(ArrayManager, DeviceDetached_testIfZeroIsReturnedWhenNoArrayFoundForGivenSerialNumber)
{
    // Given
    string existingArray = "array1";
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);
    auto mockUBlockSharedPtr = BuildMockUBlockDevice("mock-ublock", "sn");
    auto arrayMgr = new ArrayManager(nullptr, &mockDevMgr, nullptr, nullptr);

    // When
    int actual = arrayMgr->DeviceDetached(mockUBlockSharedPtr);

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayManager, PrepareRebuild_testIfTargetArrayCallsPrepareRebuild)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    int PREPARE_RESULT = 1234;
    EXPECT_CALL(*mockArrayComp, PrepareRebuild).WillOnce(Return(PREPARE_RESULT));

    // When
    bool boolOutParam = false;
    int actual = arrayMgr->PrepareRebuild(existingArray, boolOutParam /*don't care*/);

    // Then
    ASSERT_EQ(PREPARE_RESULT, actual);
}

TEST(ArrayManager, PrepareRebuild_testIfFailToPrepareRebuildWhenWrongNameIsGiven)
{
    // Given
    string nonExistentArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(nonExistentArray);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    // When
    bool boolOutParam = false;
    int actual = arrayMgr->PrepareRebuild("array2" /* != array1 */, boolOutParam /* don't care */);

    // Then
    ASSERT_EQ(EID(REBUILD_JOB_PREPARE_FAIL), actual);
}

TEST(ArrayManager, RebuildDone_testIfTargetArrayCallsRebuildDone)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    EXPECT_CALL(*mockArrayComp, RebuildDone).Times(1);

    // When
    arrayMgr->RebuildDone(existingArray);

    // Then: verify the expectation
}

TEST(ArrayManager, RebuildDone_testIfRebuildDoneIsntInvokedWhenWrongNameIsGiven)
{
    // Given
    string nonExistentArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(nonExistentArray);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockArrayComp, RebuildDone).Times(0);

    // When
    arrayMgr->RebuildDone(nonExistentArray);

    // Then: verify the expectation
}

TEST(ArrayManager, GetInfo_testIfTargetArrayCallsGetArray)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = BuildMockArrayComponents(arrayName);
    auto mockArray = BuildMockArray(arrayName);
    auto mockStateControl = new MockStateControl("array");
    auto mockGc = new MockGarbageCollector(mockArray.get(), mockStateControl);
    MockComponentsInfo* mockCompInfo = new MockComponentsInfo(mockArray.get(), mockGc);
    auto arrayMap = BuildArrayComponentsMap(arrayName, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);
    EXPECT_CALL(*mockArrayComp, GetInfo).WillOnce(Return(mockCompInfo));

    // When
    auto actual = arrayMgr->GetInfo(arrayName);

    // Then
    ASSERT_TRUE(actual != nullptr);
}

TEST(ArrayManager, GetInfo_testIfTargetArrayCallsGetArrayWithIndex)
{
    // Given
    string arrayName = "array1";
    unsigned int arrayIndex = 0;
    auto mockArrayComp = BuildMockArrayComponents(arrayName);
    auto mockArray = BuildMockArray(arrayName);
    auto mockStateControl = new MockStateControl("array");
    auto mockGc = new MockGarbageCollector(mockArray.get(), mockStateControl);
    MockComponentsInfo* mockCompInfo = new MockComponentsInfo(mockArray.get(), mockGc);
    auto arrayMap = BuildArrayComponentsMap(arrayName, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);
    EXPECT_CALL(*mockArray, GetIndex).WillOnce(Return(0));
    EXPECT_CALL(*mockArrayComp, GetInfo).WillOnce(Return(mockCompInfo));
    EXPECT_CALL(*mockArrayComp, GetArray).WillRepeatedly(Return(mockArray.get()));

    // When
    auto actual = arrayMgr->GetInfo(arrayIndex);

    // Then
    ASSERT_TRUE(actual != nullptr);
}


TEST(ArrayManager, GetInfo_testIfGetAllArraysExisting)
{
    // Given

    map<string, ArrayComponents*> arrayMap;
    for (int i = 0; i < ArrayMgmtPolicy::MAX_ARRAY_CNT; i++)
    {
        string arrayName = "array" + i;
        auto mockArrayComp = BuildMockArrayComponents(arrayName);
        arrayMap.emplace(arrayName, mockArrayComp.get());
    }
    ArrayManager* arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    auto actual = arrayMgr->GetInfo();

    // Then
    ASSERT_TRUE(actual.size() == ArrayMgmtPolicy::MAX_ARRAY_CNT);
}

TEST(ArrayManager, GetInfo_testIfReturnsNullWhenGivenArrayDoesntExist)
{
    // Given
    string arrayName = "array1";
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);

    // When
    auto actual = arrayMgr->GetInfo(arrayName);

    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(ArrayManager, ResetPbr_testIfResetSuccessWhenThereIsNoArray)
{
    // Given
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);
    int RESET_SUCCESS = 0;

    // When
    int actual = arrayMgr->ResetPbr();

    // Then
    ASSERT_EQ(RESET_SUCCESS, actual);
}

TEST(ArrayManager, ResetPbr_testIfEveryArrayCallsDeleteSuccessfully)
{
    // Given
    string arrayName = "array1";
    auto mockArray = BuildMockArray(arrayName);
    auto mockArrayComp = NewMockArrayComponents(arrayName);
    auto arrayMap = BuildArrayComponentsMap(arrayName, mockArrayComp);
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    auto mockStateControl = new MockStateControl(arrayName);
    auto mockGc = new MockGarbageCollector(mockArray.get(), mockStateControl);
    MockComponentsInfo* mockCompInfo = new MockComponentsInfo(mockArray.get(), mockGc);
    arrayMgr->SetArrayComponentMap(arrayMap);

    int DELETE_SUCCESS = 0;
    EXPECT_CALL(*mockArray, GetName).WillOnce(Return(arrayName));
    EXPECT_CALL(*mockArrayComp, GetInfo).WillOnce(Return(mockCompInfo));
    EXPECT_CALL(*mockArrayComp, Delete).WillOnce(Return(DELETE_SUCCESS));
    int RESET_SUCCESS = 0;

    // When
    int actual = arrayMgr->ResetPbr();

    // Then
    ASSERT_EQ(RESET_SUCCESS, actual);
    ASSERT_EQ(0, arrayMgr->GetInfo().size());
}

TEST(ArrayManager, ResetPbr_testIfSomeArrayDeletionsFail)
{
    // Given
    auto mockArray1Deletable = BuildMockArray("array1");
    auto mockArray2Deletable = BuildMockArray("array2");
    auto mockArrayComp1Deletable = NewMockArrayComponents("array1");
    auto mockArrayComp2NotDeletable = BuildMockArrayComponents("array2");
    auto arrayMap = BuildArrayComponentsMap();
    arrayMap.emplace("array1", mockArrayComp1Deletable);
    arrayMap.emplace("array2", mockArrayComp2NotDeletable.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr);
    auto mockStateControl1 = new MockStateControl("array1");
    auto mockStateControl2 = new MockStateControl("array2");
    auto mockGc1 = new MockGarbageCollector(mockArray1Deletable.get(), mockStateControl1);
    auto mockGc2 = new MockGarbageCollector(mockArray2Deletable.get(), mockStateControl2);
    MockComponentsInfo* mockCompInfo1 = new MockComponentsInfo(mockArray1Deletable.get(), mockGc1);
    MockComponentsInfo* mockCompInfo2 = new MockComponentsInfo(mockArray2Deletable.get(), mockGc2);
    arrayMgr->SetArrayComponentMap(arrayMap);

    EXPECT_CALL(*mockArray1Deletable, GetName).WillOnce(Return("array1"));
    EXPECT_CALL(*mockArray2Deletable, GetName).WillOnce(Return("array2"));
    EXPECT_CALL(*mockArrayComp1Deletable, GetInfo).WillRepeatedly(Return(mockCompInfo1));
    EXPECT_CALL(*mockArrayComp2NotDeletable, GetInfo).WillRepeatedly(Return(mockCompInfo2));
    EXPECT_CALL(*mockArrayComp1Deletable, Delete).WillOnce(Return(0));
    int DELETE_FAILURE = 1234;
    EXPECT_CALL(*mockArrayComp2NotDeletable, Delete).WillOnce(Return(DELETE_FAILURE));

    // When
    int actual = arrayMgr->ResetPbr();

    // Then: verify the return value and make sure ResetPbr() is never called
    ASSERT_EQ(DELETE_FAILURE, actual);
    ASSERT_EQ(1, arrayMgr->GetInfo().size());
}

} // namespace pos
