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
#include "test/unit-tests/mbr/abr_manager_mock.h"
#include "test/unit-tests/rebuild/array_rebuilder_mock.h"
#include "test/unit-tests/state/state_manager_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_client_mock.h"
#include "test/unit-tests/utils/mock_builder.h"

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
    MockAbrManager* mockAbrManager = new MockAbrManager();

    EXPECT_CALL(mockDevMgr, SetDeviceEventCallback).Times(1);

    // When
    ArrayManager arrayMgr(mockArrayRebuilder, mockAbrManager, &mockDevMgr, nullptr, nullptr);

    // Then
}

TEST(ArrayManager, ArrayManager_testIfDeviceManagerRegistersArrayManager)
{
    // Given
    MockAffinityManager mockAffinityMgr = BuildDefaultAffinityManagerMock();
    MockDeviceManager mockDevMgr(&mockAffinityMgr);

    EXPECT_CALL(mockDevMgr, SetDeviceEventCallback).Times(1);

    // When
    ArrayManager arrayMgr(nullptr, nullptr, &mockDevMgr, nullptr, nullptr);

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
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
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
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->Create(existingArray, DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(EID(ARRAY_ALREADY_EXIST), actual);
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
    ArrayManager* arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->Create("new-array", DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(EID(ARRAY_CNT_EXCEEDED), actual);
}

TEST(ArrayManager, Create_testIfArrayObjectIsFreedAndArrayMapNotUpdatedWhenCreationFails)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = NewMockArrayComponents(arrayName);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockArrayComponentFactory = [&mockArrayComp](string name, IArrayRebuilder* arrayRebuilder, IAbrControl* iAbrControl)
    {
        return mockArrayComp;
    };

    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, mockArrayComponentFactory);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    int CREATION_FAILURE = 1234;
    EXPECT_CALL(*mockArrayComp, Create).WillOnce(Return(CREATION_FAILURE));

    // When
    int actual = arrayMgr->Create(arrayName, DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(CREATION_FAILURE, actual);
    ASSERT_EQ(0, arrayMgr->GetArrayComponentMap().size());
}

TEST(ArrayManager, Create_testIfArrayMapUpdatedWhenCreationSucceeds)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = BuildMockArrayComponents(arrayName);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockTelClient = BuildMockTelemetryClient();
    auto mockArrayComponentFactory = [&mockArrayComp](string name, IArrayRebuilder* arrayRebuilder, IAbrControl* iAbrControl)
    {
        return mockArrayComp.get();
    };
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, mockTelClient.get(), mockArrayComponentFactory);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockArrayComp, Create).WillOnce(Return(0));            // success
    // EXPECT_CALL(*mockTelClient, RegisterPublisher).WillOnce(Return(0)); // success

    // When
    int actual = arrayMgr->Create(arrayName, DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(1, arrayMgr->GetArrayComponentMap().size());
}

TEST(ArrayManager, Delete_testIfArrayIsDeletedWhenThereIsNoArrayComponentButItsArrayBootRecordExists)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce([&arrayName](std::vector<ArrayBootRecord>& abrList)
    {
        ArrayBootRecord abr1;
        snprintf(abr1.arrayName, arrayName.length() + 1, "%s", arrayName.c_str());
        abrList.push_back(abr1);
        return EID(SUCCESS);
    });
    EXPECT_CALL(*mockAbrMgr, LoadAbr).WillOnce(Return(EID(SUCCESS)));
    EXPECT_CALL(*mockAbrMgr, DeleteAbr).WillOnce(Return(EID(SUCCESS)));

    // When
    int actual = arrayMgr->Delete(arrayName);

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
}

TEST(ArrayManager, Delete_testIfErrorIsReturnedWhenGivenArrayNameIsWrong)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    const int DOESNT_EXIST = 1234;
    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce(Return(DOESNT_EXIST));

    // When
    int actual = arrayMgr->Delete(arrayName);

    // Then
    ASSERT_EQ(EID(ARRAY_WRONG_NAME), actual);
}

TEST(ArrayManager, Delete_testIfArrayComponentMapIsUpdatedWhenDeleteSucceeds)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = NewMockArrayComponents(existingArray); // use New* instead of Build* since arrayMgr->Delete() actually tries to delete
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp);
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockTelClient = BuildMockTelemetryClient();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, mockTelClient.get(), nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    const int DELETE_SUCCESS = EID(SUCCESS);
    EXPECT_CALL(*mockArrayComp, Delete).WillOnce(Return(DELETE_SUCCESS));

    // When
    int actual = arrayMgr->Delete(existingArray);

    // Then
    ASSERT_EQ(DELETE_SUCCESS, actual);
    ASSERT_EQ(0, arrayMgr->GetArrayComponentMap().size());
}

TEST(ArrayManager, Mount_testIfTargetArrayCallsMount)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockTelClient = BuildMockTelemetryClient();

    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, mockTelClient.get(), nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    const int MOUNT_SUCCESS = 0;
    EXPECT_CALL(*mockArrayComp, Mount).WillOnce(Return(MOUNT_SUCCESS));
    EXPECT_CALL(*mockTelClient, RegisterPublisher).WillOnce(Return(0));  // success
    // When
    int actual = arrayMgr->Mount(existingArray, false);

    // Then
    ASSERT_EQ(MOUNT_SUCCESS, actual);
}

TEST(ArrayManager, Mount_testIfLoadFailureIsReturnedWhenTargetArrayHasArrayBootRecordButDoesntHaveArrayComponents)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = BuildMockArrayComponents(arrayName);
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockTelClient = BuildMockTelemetryClient();
    auto mockArrayComponentFactory = [&mockArrayComp](string name, IArrayRebuilder* arrayRebuilder, IAbrControl* iAbrControl)
    {
        return mockArrayComp.get();
    };
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, mockTelClient.get(), mockArrayComponentFactory);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce([&arrayName](std::vector<ArrayBootRecord>& abrList)
    {
        ArrayBootRecord abr1;
        snprintf(abr1.arrayName, arrayName.length() + 1, "%s", arrayName.c_str());
        abrList.push_back(abr1);
        return EID(SUCCESS);
    });
    // When
    int actual = arrayMgr->Mount(arrayName, false);

    // Then
    ASSERT_EQ(EID(ARRAY_LOAD_FAIL), actual);
}

TEST(ArrayManager, Mount_testIfErrorIsReturnedWhenGivenArrayNameIsWrong)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockTelClient = BuildMockTelemetryClient();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, mockTelClient.get(), nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce([&arrayName](std::vector<ArrayBootRecord>& abrList)
    {
        ArrayBootRecord abr1;
        string wrongArrayName = "array2";
        snprintf(abr1.arrayName, wrongArrayName.length() + 1, "%s", wrongArrayName.c_str());
        abrList.push_back(abr1);
        return EID(SUCCESS);
    });

    // When
    int actual = arrayMgr->Mount(arrayName, false);

    // Then
    ASSERT_EQ(EID(ARRAY_WRONG_NAME), actual);
}

TEST(ArrayManager, Unmount_testIfTargetArrayCallsUnmount)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockTelClient = BuildMockTelemetryClient();

    EXPECT_CALL(*mockArrayComp, Unmount).WillOnce(Return(EID(SUCCESS)));

    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, mockTelClient.get(), nullptr);

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
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockArray = BuildMockArray(existingArray);

    EXPECT_CALL(*mockArrayComp, GetArray).WillOnce(Return(mockArray.get()));
    EXPECT_CALL(*mockArray, AddSpare).WillOnce(Return(EID(SUCCESS)));

    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
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
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockArray = BuildMockArray(existingArray);

    EXPECT_CALL(*mockArrayComp, GetArray).WillOnce(Return(mockArray.get()));
    EXPECT_CALL(*mockArray, RemoveSpare).WillOnce(Return(EID(SUCCESS)));

    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
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
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto mockAbrMgr = BuildMockAbrManager();
    auto mockArray = BuildMockArray(existingArray);
    EXPECT_CALL(*mockAbrMgr, FindArrayWithDeviceSN).WillOnce(Return(existingArray));
    EXPECT_CALL(*mockArrayComp, GetArray).WillOnce(Return(mockArray.get()));
    EXPECT_CALL(*mockArray, DetachDevice).WillOnce(Return(EID(SUCCESS)));

    auto mockUBlockSharedPtr = BuildMockUBlockDevice("mock-ublock", "sn");
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->DeviceDetached(mockUBlockSharedPtr);

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
}

TEST(ArrayManager, DeviceDetached_testIfZeroIsReturnedWhenNoArrayFoundForGivenSerialNumber)
{
    // Given
    string existingArray = "array1";
    auto mockAbrMgr = BuildMockAbrManager();
    EXPECT_CALL(*mockAbrMgr, FindArrayWithDeviceSN).WillOnce(Return(""));
    auto mockUBlockSharedPtr = BuildMockUBlockDevice("mock-ublock", "sn");
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);

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
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
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
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    // When
    bool boolOutParam = false;
    int actual = arrayMgr->PrepareRebuild("array2" /* != array1 */, boolOutParam /* don't care */);

    // Then
    ASSERT_EQ(EID(ARRAY_WRONG_NAME), actual);
}

TEST(ArrayManager, RebuildDone_testIfTargetArrayCallsRebuildDone)
{
    // Given
    string existingArray = "array1";
    auto mockArrayComp = BuildMockArrayComponents(existingArray);
    auto arrayMap = BuildArrayComponentsMap(existingArray, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
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
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    EXPECT_CALL(*mockArrayComp, RebuildDone).Times(0);

    // When
    arrayMgr->RebuildDone(nonExistentArray);

    // Then: verify the expectation
}

TEST(ArrayManager, Load_testIfTryToLoadEveryArrayInArrayBootRecordAndReturnFailuresViaOutParam)
{
    // Given
    auto mockExistentArrayComp = BuildMockArrayComponents("array1");
    auto mockNewArrayComp = BuildMockArrayComponents("array2");
    auto mockFailedToLoadArrayComp = NewMockArrayComponents("array3");
    auto arrayMap = BuildArrayComponentsMap();
    arrayMap.emplace("array1", mockExistentArrayComp.get());
    auto mockAbrMgr = BuildMockAbrManager();
    int loadMode = 0;
    auto mockArrayComponentFactory = [&mockNewArrayComp, &mockFailedToLoadArrayComp, &loadMode](string name, IArrayRebuilder* arrayRebuilder, IAbrControl* iAbrControl)
    {
        // using "loadMode" just to implement a sequence of return values.
        if (loadMode == 0)
        {
            loadMode = 1;
            EXPECT_CALL(*mockNewArrayComp, Load).WillOnce(Return(0));
            return mockNewArrayComp.get();
        }
        else
        {
            int LOAD_FAILURE = 1234;
            EXPECT_CALL(*mockFailedToLoadArrayComp, Load).WillOnce(Return(LOAD_FAILURE));
            return mockFailedToLoadArrayComp;
        }
    };
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, mockArrayComponentFactory);
    arrayMgr->SetArrayComponentMap(arrayMap);

    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce([](std::vector<ArrayBootRecord>& abrList)
    {
        for (auto arrayName : {"array1", "array2", "array3"})
        {
            ArrayBootRecord abr;
            snprintf(abr.arrayName, strlen(arrayName) + 1, "%s", arrayName);
            abrList.push_back(abr);
        }
        return EID(SUCCESS);
    });

    // When
    std::list<string> failedArrayList;
    int actual = arrayMgr->Load(failedArrayList);

    // Then
    ASSERT_EQ(2, failedArrayList.size());
    auto itor = failedArrayList.begin();
    ASSERT_EQ("array1", *itor++);
    ASSERT_EQ("array3", *itor++);
}

TEST(ArrayManager, Load_testIfAbrLoadFailureIsHandled)
{
    // Given
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);

    int LOAD_FAILURE = 2345;
    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce(Return(LOAD_FAILURE));

    // When
    std::list<string> failedArrayList;
    int actual = arrayMgr->Load(failedArrayList);

    // Then
    ASSERT_EQ(LOAD_FAILURE, actual);
}

TEST(ArrayManager, GetAbrList_testIfAbrManagerIsQueried)
{
    // Given
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);

    int ABR_SUCCESS = 1212;
    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce(Return(ABR_SUCCESS));

    // When
    vector<ArrayBootRecord> abrList;
    int actual = arrayMgr->GetAbrList(abrList);

    // Then
    ASSERT_EQ(ABR_SUCCESS, actual);
}

TEST(ArrayManager, GetInfo_testIfTargetArrayCallsGetArray)
{
    // Given
    string arrayName = "array1";
    auto mockArrayComp = BuildMockArrayComponents(arrayName);
    auto mockArray = BuildMockArray(arrayName);
    auto mockStateControl = new MockStateControl();
    auto mockGc = new MockGarbageCollector(mockArray.get(), mockStateControl);
    MockComponentsInfo* mockCompInfo = new MockComponentsInfo(mockArray.get(), mockGc);
    auto arrayMap = BuildArrayComponentsMap(arrayName, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
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
    auto mockStateControl = new MockStateControl();
    auto mockGc = new MockGarbageCollector(mockArray.get(), mockStateControl);
    MockComponentsInfo* mockCompInfo = new MockComponentsInfo(mockArray.get(), mockGc);
    auto arrayMap = BuildArrayComponentsMap(arrayName, mockArrayComp.get());
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);
    EXPECT_CALL(*mockArrayComp, GetInfo).WillOnce(Return(mockCompInfo));
    EXPECT_CALL(*mockArrayComp, GetArray).WillRepeatedly(Return(mockArray.get()));

    // When
    auto actual = arrayMgr->GetInfo(arrayIndex);

    // Then
    ASSERT_TRUE(actual != nullptr);
}

TEST(ArrayManager, GetInfo_testIfReturnsNullWhenGivenArrayDoesntExist)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);

    // When
    auto actual = arrayMgr->GetInfo(arrayName);

    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(ArrayManager, ResetMbr_testIfTheOperationIsRejectedWhenThereIsAtLeastOneArrayNotDeletableSomehow)
{
    // Given
    int DELETABLE = 0, NOT_DELETABLE = 1;
    auto mockArrayExist = BuildMockArray("array1");
    EXPECT_CALL(*mockArrayExist, CheckDeletable).WillOnce(Return(DELETABLE));
    auto mockArrayNotDeletable = BuildMockArray("array2");
    EXPECT_CALL(*mockArrayNotDeletable, CheckDeletable).WillOnce(Return(NOT_DELETABLE));

    auto mockArrayCompExist = BuildMockArrayComponents("array1");
    auto mockArrayCompNotDeletable = BuildMockArrayComponents("array2");
    auto arrayMap = BuildArrayComponentsMap();
    arrayMap.emplace("array1", mockArrayCompExist.get());
    arrayMap.emplace("array2", mockArrayCompNotDeletable.get());
    EXPECT_CALL(*mockArrayCompExist, GetArray).WillOnce(Return(mockArrayExist.get()));
    EXPECT_CALL(*mockArrayCompNotDeletable, GetArray).WillOnce(Return(mockArrayNotDeletable.get()));

    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    // When
    int actual = arrayMgr->ResetMbr();

    // Then
    ASSERT_EQ(NOT_DELETABLE, actual);
}

TEST(ArrayManager, ResetMbr_testIfTheOperationIsRejectedWhenThereIsAtLeastOneArrayWhoseArrayComponentsDontExist)
{
    // Given
    string arrayName = "array1";
    auto emptyArrayMap = BuildArrayComponentsMap();
    emptyArrayMap.emplace(arrayName, nullptr);
    auto arrayMgr = new ArrayManager(nullptr, nullptr, nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(emptyArrayMap);

    // When
    int actual = arrayMgr->ResetMbr();

    // Then
    ASSERT_EQ(EID(ARRAY_STATE_NOT_EXIST), actual);
}

TEST(ArrayManager, ResetMbr_testIfEveryArrayCallsDeleteSuccessfullyAndAbrManagerCallsResetMbr)
{
    // Given
    string arrayName = "array1";
    auto mockArray = BuildMockArray(arrayName);
    auto mockArrayComp = NewMockArrayComponents(arrayName);
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMap = BuildArrayComponentsMap(arrayName, mockArrayComp);
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    EXPECT_CALL(*mockArray, CheckDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrayComp, GetArray).WillOnce(Return(mockArray.get()));
    int DELETE_SUCCESS = 0;
    EXPECT_CALL(*mockArrayComp, Delete).WillOnce(Return(DELETE_SUCCESS));
    int RESET_SUCCESS = 1212;
    EXPECT_CALL(*mockAbrMgr, ResetMbr).WillOnce(Return(RESET_SUCCESS));

    // When
    int actual = arrayMgr->ResetMbr();

    // Then
    ASSERT_EQ(RESET_SUCCESS, actual);
    ASSERT_EQ(0, arrayMgr->GetArrayComponentMap().size());
}

TEST(ArrayManager, ResetMbr_testIfSomeArrayDeletionsFailAndAbrManagerDoesntResetMbrConsequently)
{
    // Given
    auto mockArray1Deletable = BuildMockArray("array1");
    auto mockArray2Deletable = BuildMockArray("array2");
    auto mockArrayComp1Deletable = NewMockArrayComponents("array1");
    auto mockArrayComp2NotDeletable = BuildMockArrayComponents("array2");
    auto arrayMap = BuildArrayComponentsMap();
    arrayMap.emplace("array1", mockArrayComp1Deletable);
    arrayMap.emplace("array2", mockArrayComp2NotDeletable.get());
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);
    arrayMgr->SetArrayComponentMap(arrayMap);

    EXPECT_CALL(*mockArray1Deletable, CheckDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArray2Deletable, CheckDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrayComp1Deletable, GetArray).WillOnce(Return(mockArray1Deletable.get()));
    EXPECT_CALL(*mockArrayComp2NotDeletable, GetArray).WillOnce(Return(mockArray2Deletable.get()));
    EXPECT_CALL(*mockArrayComp1Deletable, Delete).WillOnce(Return(0));
    int DELETE_FAILURE = 1234;
    EXPECT_CALL(*mockArrayComp2NotDeletable, Delete).WillOnce(Return(DELETE_FAILURE));
    EXPECT_CALL(*mockAbrMgr, ResetMbr).Times(0); // this must not be called because there's one array unable to delete

    // When
    int actual = arrayMgr->ResetMbr();

    // Then: verify the return value and make sure ResetMbr() is never called
    ASSERT_EQ(DELETE_FAILURE, actual);
    ASSERT_EQ(1, arrayMgr->GetArrayComponentMap().size());
}

TEST(ArrayManager, AbrExists_testIfGivenArrayIsFoundOrNotFoundInArrayBootRecord)
{
    // Given
    string arrayNameFound = "array1";
    string arrayNameNotFound = "array2";
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);

    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillRepeatedly([&arrayNameFound](std::vector<ArrayBootRecord>& abrList)
    {
        ArrayBootRecord abr;
        snprintf(abr.arrayName, arrayNameFound.length() + 1, "%s", arrayNameFound.c_str());
        abrList.push_back(abr);
        return EID(SUCCESS);
    });

    // When 1
    bool actual = arrayMgr->AbrExists(arrayNameFound);

    // Then 1
    ASSERT_TRUE(actual);

    // When 2
    actual = arrayMgr->AbrExists(arrayNameNotFound);

    // Then 2
    ASSERT_FALSE(actual);
}

TEST(ArrayManager, AbrExists_testIfAbrLoadFailureIsHandledProperly)
{
    // Given
    auto mockAbrMgr = BuildMockAbrManager();
    auto arrayMgr = new ArrayManager(nullptr, mockAbrMgr.get(), nullptr, nullptr, nullptr);

    int LOAD_FAILURE = 2345;
    EXPECT_CALL(*mockAbrMgr, GetAbrList).WillOnce(Return(LOAD_FAILURE));

    // When
    bool actual = arrayMgr->AbrExists("array1");

    // Then
    ASSERT_FALSE(actual);
}

} // namespace pos
