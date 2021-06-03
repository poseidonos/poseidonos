#include "src/array/array.h"

#include <gtest/gtest.h>
#include <sys/queue.h>

#include "lib/spdk/lib/nvme/nvme_internal.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/include/partition_type.h"
#include "test/unit-tests/array/array_interface_mock.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/array/device/array_device_manager_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/array/interface/i_abr_control_mock.h"
#include "test/unit-tests/array/partition/partition_manager_mock.h"
#include "test/unit-tests/array/state/array_state_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/rebuild/array_rebuilder_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"
#include "test/utils/spdk_util.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace pos
{
TEST(Array, Array_testIfConstructedProperly)
{
    // Given: nothing

    // When: testing construct signature (trivial)
    Array array("mock", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    // Then: nothing
}

/**
 * Case to share: can we have a mock child from a real parent? 
 */
TEST(Array, Init_testIfStateNotExistIsHandledProperly)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl); // alloc here, but freed within array.cpp's desctructor
    EXPECT_CALL(*mockState, Exists).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsLoadable).Times(0);

    Array array("mock", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Init();

    // Then: check the return code and make sure mockState->IsLoadable() is never called (i.e., Times(0))
    ASSERT_EQ(EID(ARRAY_STATE_NOT_EXIST), actual);
}

TEST(Array, Init_testIfLoadFailureIsHandledProperly)
{
    // Given
    int LOAD_FAILURE = 1;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    EXPECT_CALL(*mockState, Exists).WillOnce(Return(true));
    EXPECT_CALL(*mockState, IsLoadable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, LoadAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_, _, _)).WillOnce(Return(LOAD_FAILURE)); // make it return non-zero
    EXPECT_CALL(*mockState, SetDelete).Times(1);                                 // failure path
    EXPECT_CALL(*mockState, IsMountable).Times(0);                               // should not be invoked!

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When: array is initialized
    int actual = array.Init();

    // Then: LOAD_FAILURE should be returned and state->IsMountable should never be called
    ASSERT_EQ(LOAD_FAILURE, actual);
}

TEST(Array, Init_testIfPartitionRegistrationFailureHandledProperly)
{
    // Given
    int LOAD_SUCCESS = 0;
    int MOUNT_SUCCESS = 0;
    int PARTITION_REGISTRATION_FAILURE = 1;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockPartitionManager* mockPartMgr = new MockPartitionManager("mock-part", NULL);

    EXPECT_CALL(*mockState, Exists).WillOnce(Return(true));
    EXPECT_CALL(*mockState, IsLoadable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, LoadAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_, _, _)).WillOnce(Return(LOAD_SUCCESS)); // make it return zero
    EXPECT_CALL(*mockState, SetDelete).Times(0);                                 // failure path should not be invoked!
    EXPECT_CALL(*mockState, SetLoad).Times(1);
    EXPECT_CALL(*mockState, IsMountable).WillOnce(Return(MOUNT_SUCCESS));
    DeviceSet<ArrayDevice*> mockDevs;
    EXPECT_CALL(*mockArrDevMgr, Export).WillOnce(ReturnRef(mockDevs));

    EXPECT_CALL(*mockPartMgr, CreateAll).WillOnce(Return(PARTITION_REGISTRATION_FAILURE));
    EXPECT_CALL(*mockState, SetMount).Times(0); // if partition fails to be created, this should not be invoked!

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, mockPartMgr, mockState, NULL, NULL);

    // When: array is initialized
    int actual = array.Init();

    // Then: non-zero should be returned and state->SetMount should never be called
    ASSERT_EQ(PARTITION_REGISTRATION_FAILURE, actual);
}

TEST(Array, Init_testIfInitIsDoneSuccesfully)
{
    // Given
    int LOAD_SUCCESS = 0;
    int MOUNT_SUCCESS = 0;
    int PARTITION_REGISTRATION_SUCCESS = 0;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockPartitionManager* mockPartMgr = new MockPartitionManager("mock-part", NULL);
    MockArrayInterface* mockArrayInterface = new MockArrayInterface;
    EXPECT_CALL(*mockArrayInterface, GetRecover).Times(1);
    EXPECT_CALL(*mockArrayInterface, GetTranslator).Times(1);

    EXPECT_CALL(*mockState, Exists).WillOnce(Return(true));
    EXPECT_CALL(*mockState, IsLoadable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, LoadAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_, _, _)).WillOnce(Return(LOAD_SUCCESS)); // make it return zero
    EXPECT_CALL(*mockState, SetDelete).Times(0);                                 // failure path should not be invoked!
    EXPECT_CALL(*mockState, SetLoad).Times(1);
    EXPECT_CALL(*mockState, IsMountable).WillOnce(Return(MOUNT_SUCCESS));
    DeviceSet<ArrayDevice*> mockDevs;
    EXPECT_CALL(*mockArrDevMgr, Export).WillOnce(ReturnRef(mockDevs));

    EXPECT_CALL(*mockPartMgr, CreateAll).WillOnce(Return(PARTITION_REGISTRATION_SUCCESS));
    EXPECT_CALL(*mockState, SetMount).Times(1);

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, mockPartMgr, mockState, mockArrayInterface, NULL);

    // When: array is initialized
    int actual = array.Init();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(Array, Dispose_testIfDependenciesAreInvoked)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockPartitionManager* mockPartMgr = new MockPartitionManager("mock-part", NULL);
    Array array("mock", NULL, NULL, NULL, NULL, mockPartMgr, mockState, NULL, NULL);

    EXPECT_CALL(*mockPartMgr, DeleteAll).Times(1);
    EXPECT_CALL(*mockPartMgr, CreateAll).Times(0);
    EXPECT_CALL(*mockState, SetUnmount).Times(1);

    // When
    array.Dispose();

    // Then: verify if the number of API invocations is equal to what's expected
}

/***
 * Load() relies on _LoadImpl() which has been tested as part of unit-testing Array::Init().
 * So, this UT will simply test out happy happy path only
 **/
TEST(Array, Load_testIfDoneSuccessfully)
{
    // Given: a happy path scenario
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockIAbrControl mockAbrControl;
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    EXPECT_CALL(*mockState, IsLoadable).WillOnce(Return(0));          // isLoadable will be true
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);                      // devMgr_ will be able to invoked once
    EXPECT_CALL(mockAbrControl, LoadAbr).WillOnce(Return(0));         // loading array boot record will be successful
    EXPECT_CALL(*mockArrDevMgr, Import(_, _, _)).WillOnce(Return(0)); // import will be successful
    EXPECT_CALL(*mockState, SetLoad).Times(1);                        // SetLoad will be invoked once

    // When: array is loaded
    int actual = array.Load();

    // Then: we should receive zero from the return
    ASSERT_EQ(0, actual);
}

TEST(Array, Load_testIfLoadFailsWhenStateIsNotLoadable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    int LOAD_FAILURE = EID(ARRAY_BROKEN_ERROR);
    EXPECT_CALL(*mockState, IsLoadable).WillOnce(Return(LOAD_FAILURE));
    Array array("mock", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Load();

    // Then
    ASSERT_EQ(LOAD_FAILURE, actual);
}

TEST(Array, Load_testIfLoadFailsWhenArrayBootRecordFailsToBeLoaded)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    int LOAD_SUCCESS = 0;
    int ABR_FAILURE = EID(MBR_ABR_NOT_FOUND);

    EXPECT_CALL(*mockState, IsLoadable).WillOnce(Return(LOAD_SUCCESS));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, LoadAbr).WillOnce(Return(ABR_FAILURE));

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Load();

    // Then
    ASSERT_EQ(ABR_FAILURE, actual);
}

TEST(Array, Create_testIfArrayCreatedWhenInputsAreValid)
{
    // Given: a happy path scenario
    DeviceSet<string> emptyDeviceSet;
    DeviceSet<ArrayDevice*> emptyArrayDeviceSet;
    string mockArrayName = "POSArray";
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl* mockAbrControl = new MockIAbrControl();
    MockPartitionManager* mockPtnMgr = new MockPartitionManager(mockArrayName, mockAbrControl);
    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Export()).WillOnce(ReturnRef(emptyArrayDeviceSet));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillRepeatedly(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(*mockAbrControl, CreateAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockAbrControl, SaveAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockPtnMgr, FormatMetaPartition).Times(1);
    EXPECT_CALL(*mockState, SetCreate).Times(1);

    Array array(mockArrayName, NULL, mockAbrControl, mockArrDevMgr, NULL, mockPtnMgr, mockState, NULL, NULL);

    // When
    int actual = array.Create(emptyDeviceSet, "RAID5" /* is the only option at the moment */);

    // Then
    ASSERT_EQ(0, actual);
    delete mockAbrControl;
}

TEST(Array, Create_testIfErrorIsReturnedWhenArrayStateIsNotCreatable)
{
    // Given
    DeviceSet<string> emptyDeviceSet;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    int STATE_ERROR = EID(ARRAY_STATE_EXIST);
    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(STATE_ERROR));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);

    Array array("goodmockname", NULL, NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Create(emptyDeviceSet, "doesn't matter");

    // Then
    ASSERT_EQ(STATE_ERROR, actual);
}

TEST(Array, Create_testIfErrorIsReturnedWhenDeviceImportFails)
{
    // Given
    DeviceSet<string> emptyDeviceSet;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    int IMPORT_ERROR = EID(ARRAY_DEVICE_NOT_FOUND);

    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_)).WillOnce(Return(IMPORT_ERROR));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);

    Array array("goodmockname", NULL, NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Create(emptyDeviceSet, "doesn't matter");

    // Then
    ASSERT_EQ(IMPORT_ERROR, actual);
}

TEST(Array, Create_testIfErrorIsReturnedWhenArrayNameIsInvalid)
{
    // Given
    DeviceSet<string> emptyDeviceSet;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    string BAD_ARRAY_NAME = "This is really bad array name because it contains multiple whitespaces and is too long";

    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);

    Array array(BAD_ARRAY_NAME, NULL, NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Create(emptyDeviceSet, "doesn't matter");

    // Then
    ASSERT_EQ(EID(ARRAY_NAME_TOO_LONG), actual);
}

TEST(Array, Create_testIfErrorIsReturnedWhenRaidTypeIsInvalid)
{
    // Given
    DeviceSet<string> emptyDeviceSet;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    string BAD_RAID_TYPE = "RAID6"; // 'cause we don't support at the moment

    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);

    Array array("goodmockname", NULL, NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Create(emptyDeviceSet, BAD_RAID_TYPE);

    // Then
    ASSERT_EQ(EID(ARRAY_WRONG_FT_METHOD), actual);
}

TEST(Array, Create_testIfErrorIsReturnedWhenAbrFailsToBeCreated)
{
    // Given
    DeviceSet<string> emptyDeviceSet;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    int ABR_CREATE_FAILURE = EID(MBR_ABR_ALREADY_EXIST);
    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, CreateAbr).WillOnce(Return(ABR_CREATE_FAILURE));

    Array array("goodmockname", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);
    string GOOD_RAID_TYPE = "RAID5";

    // When
    int actual = array.Create(emptyDeviceSet, GOOD_RAID_TYPE);

    // Then
    ASSERT_EQ(ABR_CREATE_FAILURE, actual);
}

TEST(Array, Create_testIfErrorIsReturnedWhenAbrFailsToBeSaved)
{
    // Given
    DeviceSet<string> emptyDeviceSet;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    int ABR_SAVE_FAILURE = EID(MBR_ABR_NOT_FOUND);
    EXPECT_CALL(*mockState, IsCreatable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Import(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillRepeatedly(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, CreateAbr).WillOnce(Return(0));
    EXPECT_CALL(mockAbrControl, SaveAbr).WillOnce(Return(ABR_SAVE_FAILURE));

    Array array("goodmockname", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Create(emptyDeviceSet, "RAID5");

    // Then
    ASSERT_EQ(ABR_SAVE_FAILURE, actual);
}

TEST(Array, Delete_testIfArrayDeletedSuccessfullyWhenInputsAreValid)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, DeleteAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockState, SetDelete).Times(1);

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Delete();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(Array, Delete_testIfArrayNotDeletedWhenStateIsNotDeletable)
{
    // Given: Non-deletable state
    int NON_ZERO = 1;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(NON_ZERO));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(0); // this should never be called

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Delete();

    // Then: ArrayDeviceManager should never call Clear() method
    ASSERT_EQ(NON_ZERO, actual);
}

TEST(Array, Delete_testIfArrayNotDeletedWhenArrayBootRecordFailsToBeUpdated)
{
    // Given: a deletable state, but ABR I/O fails
    int ABR_FAILURE = 1;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(mockAbrControl, DeleteAbr).WillOnce(Return(ABR_FAILURE));
    EXPECT_CALL(*mockState, SetDelete).Times(0); // this should never be called

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.Delete();

    // Then: state should never be set to Delete
    ASSERT_EQ(ABR_FAILURE, actual);
}

TEST(Array, AddSpare_testIfSpareIsAddedWhenInputsAreValid)
{
    // Given: a happy path scenario
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockEventScheduler mockEventScheduler;

    EXPECT_CALL(*mockState, CanAddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, AddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(mockAbrControl, SaveAbr).WillOnce(Return(0));
    EXPECT_CALL(mockEventScheduler, EnqueueEvent).Times(1);

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, &mockEventScheduler);

    // When: we try to add a spare device
    int actual = array.AddSpare("mock-spare");

    // Then: we should receive 0 as a return
    ASSERT_EQ(0, actual);
}

TEST(Array, RemoveSpare_testIfSpareIsRemovedWhenInputsAreValid)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    string mockSpareName = "mock-spare";

    EXPECT_CALL(*mockState, CanRemoveSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(mockSpareName)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(mockAbrControl, SaveAbr).WillOnce(Return(0));

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    int actual = array.RemoveSpare(mockSpareName);

    // Then
    ASSERT_EQ(0, actual);
}

TEST(Array, DetachDevice_testIfSpareDeviceIsSuccessfullyDetachedFromUnmountedArray)
{
    // Given: a spare device and unmounted array
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockDeviceManager mockSysDevMgr;

    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    tuple<ArrayDevice*, ArrayDeviceType> mockGetDevRes = make_tuple(
        mockArrDev,
        ArrayDeviceType::SPARE);

    EXPECT_CALL(*mockArrDevMgr, GetDev(fakeUblockSharedPtr)).WillOnce(Return(mockGetDevRes));
    EXPECT_CALL(*mockArrDev, GetUblock).WillOnce(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(spareDevName)).WillOnce(Return(0));
    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(false));

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, &mockSysDevMgr, NULL, mockState, NULL, NULL);

    // When: detachDevice() is invoked
    int actual = array.DetachDevice(fakeUblockSharedPtr);

    // Then: the operation should be successful
    ASSERT_EQ(0, actual);
    DestroyFakeNvmeNamespace(fakeNs); // just for cleanup
}

TEST(Array, DetachDevice_testIfDataDeviceIsSuccessfullyDetachedFromUnmountedArray)
{
    // Given: a data device and unmounted array
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockDeviceManager mockSysDevMgr;

    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    tuple<ArrayDevice*, ArrayDeviceType> mockGetDevRes = make_tuple(
        mockArrDev,
        ArrayDeviceType::DATA);

    EXPECT_CALL(*mockArrDevMgr, GetDev(fakeUblockSharedPtr)).WillOnce(Return(mockGetDevRes));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockState, DataRemoved).Times(1);
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::FAULT)).Times(1);
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockArrDev, SetUblock).Times(1);
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(false));

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, &mockSysDevMgr, NULL, mockState, NULL, NULL);

    // When: DetachDevice() is invoked
    int actual = array.DetachDevice(fakeUblockSharedPtr);

    // Then: the operation should be successful
    ASSERT_EQ(0, actual);
    DestroyFakeNvmeNamespace(fakeNs);
    delete mockArrDev;
}

TEST(Array, DetachDevice_testIfDataDeviceIsSuccessfullyDetachedFromMountedArray)
{
    // Given: a data device and mounted array
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockDeviceManager mockSysDevMgr;

    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    tuple<ArrayDevice*, ArrayDeviceType> mockGetDevRes = make_tuple(
        mockArrDev,
        ArrayDeviceType::DATA);

    EXPECT_CALL(*mockArrDevMgr, GetDev(fakeUblockSharedPtr)).WillOnce(Return(mockGetDevRes));
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockState, DataRemoved).Times(1);
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::FAULT)).Times(1);
    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockArrDev, SetUblock).Times(1);
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(true));
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(mockAbrControl, SaveAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(false));

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, &mockSysDevMgr, NULL, mockState, NULL, NULL);
    // When: DetachDevice() is invoked
    int actual = array.DetachDevice(fakeUblockSharedPtr);

    // Then: the operation should be successful and the dependent operations should be invoked accordingly
    ASSERT_EQ(0, actual);
    DestroyFakeNvmeNamespace(fakeNs);
    delete mockArrDev;
}

TEST(Array, DetachDevice_IntegrationTestIfDataDeviceIsNotDetachedFromMountedArrayDueToArrayDeviceStateIsNotInFault)
{
    // Given: a data device in FAULT state and mounted array
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;
    MockDeviceManager mockSysDevMgr;

    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    tuple<ArrayDevice*, ArrayDeviceType> mockGetDevRes = make_tuple(
        mockArrDev,
        ArrayDeviceType::DATA);

    EXPECT_CALL(*mockArrDevMgr, GetDev(fakeUblockSharedPtr)).WillOnce(Return(mockGetDevRes));
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockState, DataRemoved).Times(0); // should never be called

    Array array("mock", NULL, &mockAbrControl, mockArrDevMgr, &mockSysDevMgr, NULL, mockState, NULL, NULL);

    // When: DetachDevice is invoked
    int actual = array.DetachDevice(fakeUblockSharedPtr);

    // Then: the operation should succeed(?): TODO(hsung): please check whether the return value should be zero.
    ASSERT_EQ(0, actual);
    delete mockArrDev;
}

TEST(Array, MountDone_testIfResumeRebuildEventIsSent)
{
    // Given: an array
    MockEventScheduler mockEventScheduler;
    MockArrayDeviceManager* mockArrayDeviceManager = new MockArrayDeviceManager(NULL);
    string mockDevName = "mockDevName";
    MockUBlockDevice* mockUblockDevice = new MockUBlockDevice(mockDevName, 1024, NULL);
    UblockSharedPtr mockUblockSharedPtr = shared_ptr<UBlockDevice>(mockUblockDevice);
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(NULL);

    EXPECT_CALL(mockEventScheduler, EnqueueEvent).Times(1);
    EXPECT_CALL(*mockArrayDeviceManager, GetRebuilding).WillOnce(Return(mockArrayDevice));
    EXPECT_CALL(*mockArrayDevice, GetUblock).WillRepeatedly(Return(mockUblockSharedPtr));
    EXPECT_CALL(*mockUblockDevice, GetSN).WillOnce(Return(mockDevName));

    Array array("mock", NULL, NULL, mockArrayDeviceManager, NULL, NULL, NULL, NULL, &mockEventScheduler);

    // When: Mount is done
    array.MountDone();

    // Then: EnqueueEvent should be called once
    delete mockArrayDevice;
}

TEST(Array, CheckUnmountable_testIfStateIsQueriedOn)
{
    // Given: an array and a state mock
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    Array array("mock", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    int expected = 121212; // return some random event id
    EXPECT_CALL(*mockState, IsUnmountable).WillOnce(Return(expected));

    // When: CheckUnmountable is invoked
    int actual = array.CheckUnmountable();

    // Then: we should query state object and get the result
    ASSERT_EQ(expected, actual);
}

TEST(Array, CheckDeletable_testIfStateIsQueriedOn)
{
    // Given: an array and a state mock
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    Array array("mock", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    int expected = 121212; // return some random event id
    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(expected));

    // When: CheckUnmountable is invoked
    int actual = array.CheckDeletable();

    // Then: we should query state object and get the result
    ASSERT_EQ(expected, actual);
}

/***
 * These are trivial tests to achieve high code coverage.
 */
TEST(Array, Set_testIfSettersAreInvoked)
{
    // Given: an array
    Array array("mock", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    string expectedMRT = "mock-meta-raid-type";
    string expectedDRT = "mock-data-raid-type";

    // When: setters are called + a few trivial API invocations
    array.SetMetaRaidType(expectedMRT);
    array.SetDataRaidType(expectedDRT);
    array.GetName();
    array.GetCreateDatetime();
    array.GetUpdateDatetime();

    // Then:
    ASSERT_EQ(expectedMRT, array.GetMetaRaidType());
    ASSERT_EQ(expectedDRT, array.GetDataRaidType());
}

TEST(Array, GetSizeInfo_testIfPartitionManagerIsQueriedOn)
{
    // Given: an array and a partition manager
    MockPartitionManager* mockPartMgr = new MockPartitionManager("mock-array", NULL);
    PartitionType partType = PartitionType::USER_DATA;
    PartitionLogicalSize logicalSize;
    EXPECT_CALL(*mockPartMgr, GetSizeInfo(partType)).WillOnce(Return(&logicalSize));

    Array array("mock", NULL, NULL, NULL, NULL, mockPartMgr, NULL, NULL, NULL);

    // When: GetSizeInfo() is invoked
    array.GetSizeInfo(partType);

    // Then: we should ask partition manager to get us the result (defined by EXPECT_CALL)
}

TEST(Array, GetDevNames_testIfArrayDeviceManagerIsQueriedOn)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    EXPECT_CALL(*mockArrDevMgr, ExportToName).Times(1);
    Array array("mock", NULL, NULL, mockArrDevMgr, NULL, NULL, NULL, NULL, NULL);

    // When
    array.GetDevNames();

    // Then
}

TEST(Array, GetState_testIfStateIsQueriedOn)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    Array array("mock", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    ArrayStateType expected(ArrayStateEnum::NORMAL);
    EXPECT_CALL(*mockState, GetState).WillOnce(Return(expected));

    // When
    ArrayStateType actual = array.GetState();

    // Then
    ASSERT_EQ(expected, actual);
}

TEST(Array, GetRebuildingProgress_testIfArrayNameIsPassedInProperly)
{
    // Given
    string arrayName = "mock-array";
    MockArrayRebuilder mockArrayRebuilder(NULL);
    int expectedProgress = 121212;
    EXPECT_CALL(mockArrayRebuilder, GetRebuildProgress(arrayName)).WillOnce(Return(expectedProgress));

    Array array(arrayName, &mockArrayRebuilder, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

    // When
    int actual = array.GetRebuildingProgress();

    // Then
    ASSERT_EQ(expectedProgress, actual);
}

TEST(Array, IsRecoverable_testIfBrokenArrayIsNotRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);

    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(true));
    EXPECT_CALL(*mockState, IsRecoverable).Times(0); // should never be called

    Array array("mock-array", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    // When
    bool actual = array.IsRecoverable(NULL, NULL);

    // Then
    ASSERT_FALSE(actual);
}

TEST(Array, IsRecoverable_testIfUnmountedArrayIsNotRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDevice* mockArrDev = new MockArrayDevice(NULL);
    MockUBlockDevice* uBlockPtr = new MockUBlockDevice("mock-dev", 1024, NULL);

    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(false));

    Array array("mock-array", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    // When
    bool actual = array.IsRecoverable(mockArrDev, uBlockPtr);

    // Then
    ASSERT_FALSE(actual);
}

TEST(Array, IsRecoverable_testIfArrayFailingToTranslateIsNotRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDevice* mockArrDev = new MockArrayDevice(NULL);
    MockUBlockDevice* uBlockPtr = nullptr; // Simulate translate/convert failure

    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(true));

    Array array("mock-array", NULL, NULL, NULL, NULL, NULL, mockState, NULL, NULL);

    // When
    bool actual = array.IsRecoverable(mockArrDev, uBlockPtr);

    // Then: TODO(hsung): please check whether "true" is intended. IsRecoverable() is returning true when uBlock is null.
    ASSERT_TRUE(actual);
}

TEST(Array, IsRecoverable_testIfMountedHealthyArrayDetachesDataDeviceAndIsRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    // ok to use real obj
    string dataDevName = "mock-unvme";
    MockUBlockDevice* mockUblockDevPtr = new MockUBlockDevice(dataDevName, 1024, nullptr);
    UblockSharedPtr mockUblockSharedPtr = shared_ptr<UBlockDevice>(mockUblockDevPtr);

    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    MockDeviceManager mockSysDevMgr;
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockIAbrControl mockAbrControl;

    EXPECT_CALL(*mockState, IsBroken).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(mockUblockSharedPtr));
    EXPECT_CALL(*mockUblockDevPtr, GetName).WillOnce(Return(dataDevName.c_str()));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockState, DataRemoved(false)).Times(1);
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::FAULT)).Times(1);
    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(mockAbrControl, SaveAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDev, SetUblock).Times(1);
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsRecoverable).WillOnce(Return(true));

    Array array("mock-array", NULL, &mockAbrControl, mockArrDevMgr, &mockSysDevMgr, NULL, mockState, NULL, NULL);

    // When
    bool actual = array.IsRecoverable(mockArrDev, mockUblockDevPtr);

    // Then
    ASSERT_TRUE(actual);
    delete mockArrDev;
}

TEST(Array, FindDevice_testIfArrayDevMgrIsQueriedAgainst)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    string mockSerialNumber = "mock-sn";

    EXPECT_CALL(*mockArrDevMgr, GetDev(mockSerialNumber)).WillOnce(Return(make_tuple(nullptr, ArrayDeviceType::DATA)));

    Array array("mock-array", NULL, NULL, mockArrDevMgr, NULL, NULL, NULL, NULL, NULL);

    // When
    IArrayDevice* actual = array.FindDevice(mockSerialNumber);

    // Then
    ASSERT_EQ(nullptr, actual);
}

TEST(Array, TriggerRebuild_testIfNullTargetShouldNotBeRetried)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);

    EXPECT_CALL(*mockArrDevMgr, GetFaulty).WillOnce(Return(nullptr));

    Array array("mock-array", NULL, NULL, mockArrDevMgr, NULL, NULL, NULL, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(nullptr);

    // Then
    ASSERT_FALSE(actual);
}

TEST(Array, TriggerRebuild_testIfNonFaultyArrayDeviceCanSuccessfullyTriggerRebuild)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::NORMAL)); // NORMAL or REBUILD?

    Array array("mock-array", NULL, NULL, mockArrDevMgr, NULL, NULL, NULL, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(mockArrDev);

    // Then
    ASSERT_TRUE(actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfFaultyArrayDeviceDoesNotRetryWhenTheStateIsntSetToRebuild)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(false));
    EXPECT_CALL(*mockArrDev, GetUblock).WillOnce(Return(nullptr));

    Array array("mock-array", NULL, NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(mockArrDev);

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfFaultyArrayDeviceDoesNotRetryRebuildDueToReplaceFailure)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    int REPLACE_FAILURE = 1;

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockArrDev, GetUblock).WillOnce(Return(nullptr));
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(true));
    EXPECT_CALL(*mockArrDevMgr, ReplaceWithSpare).WillOnce(Return(REPLACE_FAILURE));
    EXPECT_CALL(*mockState, SetRebuildDone(false)).Times(1);
    EXPECT_CALL(*mockState, SetDegraded).Times(1);

    Array array("mock-array", NULL, NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(mockArrDev);

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfFaultyArrayDeviceDoesNotNeedToRetryAfterTriggeringRebuild)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL);
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockIAbrControl mockAbrControl;
    MockArrayInterface* mockArrayInterface = new MockArrayInterface;
    MockArrayRebuilder mockArrayRebuilder(NULL);
    int REPLACE_SUCCESS = 0;
    std::list<RebuildTarget*> emptyTargets;

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockArrDev, GetUblock).WillOnce(Return(nullptr));
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(true));
    EXPECT_CALL(*mockArrDevMgr, ReplaceWithSpare).WillOnce(Return(REPLACE_SUCCESS));
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::REBUILD)).Times(1);
    EXPECT_CALL(*mockArrDevMgr, ExportToMeta).WillOnce(Return(DeviceSet<DeviceMeta>()));
    EXPECT_CALL(mockAbrControl, SaveAbr).WillOnce(Return(0));
    EXPECT_CALL(*mockArrayInterface, GetRebuildTargets).WillOnce(Return(emptyTargets));
    EXPECT_CALL(mockArrayRebuilder, Rebuild).Times(AtLeast(0)); // simply, ignore

    Array array("mock-array", &mockArrayRebuilder, &mockAbrControl, mockArrDevMgr, NULL, NULL, mockState, mockArrayInterface, NULL);

    // When
    bool actual = array.TriggerRebuild(mockArrDev);

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;

    usleep(10000); // intentionally put some jitter to avoid signal propagated from internally-spawned thread
}

} // namespace pos
