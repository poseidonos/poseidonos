#include "src/array/array.h"

#include <gtest/gtest.h>
#include <sys/queue.h>

#include "lib/spdk/lib/nvme/nvme_internal.h"
#include "src/device/unvme/unvme_ssd.h"
#include "src/include/partition_type.h"
#include "test/unit-tests/array/partition/partition_services_mock.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/array/device/array_device_manager_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/array/partition/partition_manager_mock.h"
#include "test/unit-tests/array/rebuild/i_array_rebuilder_mock.h"
#include "test/unit-tests/array/service/array_service_layer_mock.h"
#include "test/unit-tests/array/state/array_state_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/event_scheduler/event_scheduler_mock.h"
#include "test/unit-tests/rebuild/array_rebuilder_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"
#include "test/unit-tests/utils/mock_builder.h"
#include "test/unit-tests/utils/spdk_util.h"
#include "test/unit-tests/pbr/pbr_adapter_mock.h"

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

TEST(Array, Init_testIfArrayServiceRegistrationFailureHandledProperly)
{
    // Given
    int LOAD_SUCCESS = 0;
    int MOUNT_SUCCESS = 0;
    int PARTITION_REGISTRATION_SUCCESS = 0;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockPartitionManager* mockPartMgr = new MockPartitionManager();
    MockPartitionServices* mockSvc = new MockPartitionServices;
    MockArrayServiceLayer* mockArrayService = new MockArrayServiceLayer;
    EXPECT_CALL(*mockSvc, GetRecover).Times(1);
    EXPECT_CALL(*mockSvc, GetTranslator).Times(1);

    EXPECT_CALL(*mockState, IsMountable).WillOnce(Return(MOUNT_SUCCESS));
    vector<ArrayDevice*> mockDevs;
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillRepeatedly(ReturnRef(mockDevs));
    EXPECT_CALL(*mockArrayService, Register).WillOnce(Return(EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_RECOVER)));
    EXPECT_CALL(*mockState, SetMount).Times(0);
    EXPECT_CALL(*mockArrayService, Unregister).Times(1);

    Array array("mock", NULL, mockArrDevMgr, NULL, mockPartMgr, mockState, mockSvc, NULL, mockArrayService);

    // When: array is initialized
    int actual = array.Init();

    // Then
    ASSERT_EQ(EID(MOUNT_ARRAY_UNABLE_TO_REGISTER_RECOVER), actual);

    // Wrapup
    delete mockArrayService;
}

TEST(Array, Init_testIfInitIsDoneSuccessfully)
{
    // Given
    int LOAD_SUCCESS = 0;
    int MOUNT_SUCCESS = 0;
    int PARTITION_REGISTRATION_SUCCESS = 0;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    MockPartitionManager* mockPartMgr = new MockPartitionManager();
    MockPartitionServices* mockSvc = new MockPartitionServices;
    MockArrayServiceLayer* mockArrayService = new MockArrayServiceLayer;
    EXPECT_CALL(*mockSvc, GetRecover).Times(1);
    EXPECT_CALL(*mockSvc, GetTranslator).Times(1);

    EXPECT_CALL(*mockState, IsMountable).WillOnce(Return(MOUNT_SUCCESS));
    vector<ArrayDevice*> mockDevs;
    EXPECT_CALL(*mockState, SetMount).Times(1);
    EXPECT_CALL(*mockArrayService, Register).WillOnce(Return(0));
    EXPECT_CALL(*mockPartMgr, GetRaidType).Times(1);
    EXPECT_CALL(*mockArrayService, Unregister).Times(0);

    Array array("mock", NULL, mockArrDevMgr, NULL, mockPartMgr, mockState, mockSvc, NULL, mockArrayService);

    // When: array is initialized
    int actual = array.Init();

    // Then
    ASSERT_EQ(0, actual);

    // Wrapup
    delete mockArrayService;
}

TEST(Array, Dispose_testIfDependenciesAreInvoked)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayServiceLayer* mockArrayService = new MockArrayServiceLayer;
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    MockPartitionManager* mockPartMgr = new MockPartitionManager();
    Array array("mock", NULL, NULL, NULL, mockPartMgr, mockState, NULL, NULL, mockArrayService);

    EXPECT_CALL(*mockPartMgr, DeletePartitions).Times(0);
    EXPECT_CALL(*mockPartMgr, Import).Times(0);
    EXPECT_CALL(*mockState, SetUnmount).Times(1);
    EXPECT_CALL(*mockArrayService, Unregister).Times(1);

    // When
    array.Dispose();

    // Then: verify if the number of API invocations is equal to what's expected

    // Wrapup
    delete mockArrayService;
}

// TEST(Array, Import_testIfDoneSuccessfully)
// {
//     // Given: a happy path scenario
//     NiceMock<MockIStateControl> mockIStateControl;
//     MockArrayState* mockState = new MockArrayState(&mockIStateControl);
//     DeviceSet<ArrayDevice*> emptyArrayDeviceSet;
//     MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
//     MockPartitionManager* mockPtnMgr = new MockPartitionManager();
//     Array array("mock", NULL, mockArrDevMgr, NULL, mockPtnMgr, mockState, NULL, NULL, NULL);

//     EXPECT_CALL(*mockArrDevMgr, Import).WillOnce(Return(0));                      // import will be successful
//     EXPECT_CALL(*mockState, GetSysState).Times(2);
//     EXPECT_CALL(*mockPtnMgr, GetPhysicalSize).Times(6);
//     EXPECT_CALL(*mockPtnMgr, GetSizeInfo).Times(6);
//     // When: array is loaded
//     int actual = array.Import();

//     // Then: we should receive zero from the return
//     ASSERT_EQ(0, actual);
// }


TEST(Array, Delete_testIfArrayDeletedSuccessfullyWhenInputsAreValid)
{
    // Given
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockPartitionServices* mockSvc = new MockPartitionServices;
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();

    EXPECT_CALL(*mockRebuilder, IsRebuilding).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(1);
    EXPECT_CALL(*mockState, SetDelete).Times(1);
    EXPECT_CALL(*mockState, WaitShutdownDone).WillOnce(Return(0));
    EXPECT_CALL(*mockPtnMgr, DeletePartitions).Times(1);
    EXPECT_CALL(*mockPbrAdapter, Reset("mock")).WillOnce(Return(0));

    Array array("mock", mockRebuilder, mockArrDevMgr, NULL, mockPtnMgr, mockState, mockSvc, NULL, NULL, mockPbrAdapter);

    // When
    int actual = array.Delete();

    // Then
    ASSERT_EQ(0, actual);

    // CleanUp
    delete mockRebuilder;
}

TEST(Array, Delete_testIfArrayNotDeletedWhenStateIsNotDeletable)
{
    // Given: Non-deletable state
    int NON_ZERO = 1;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    MockPartitionServices* mockSvc = new MockPartitionServices();

    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(NON_ZERO));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(0); // this should never be called

    Array array("mock", NULL, mockArrDevMgr, NULL, mockPtnMgr, mockState,
        mockSvc, NULL, NULL);

    // When
    int actual = array.Delete();

    // Then: ArrayDeviceManager should never call Clear() method
    ASSERT_EQ(NON_ZERO, actual);
}

TEST(Array, AddSpare_testIfSpareIsAddedWhenInputsAreValid)
{
    // Given: a happy path scenario
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockDeviceManager mockDevMgr;
    MockEventScheduler mockEventScheduler;
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();

    string spareDevName = "spareDev";
    MockUBlockDevice* rawPtr = new MockUBlockDevice(spareDevName, 1024, nullptr);
    UblockSharedPtr mockSpareDev = shared_ptr<MockUBlockDevice>(rawPtr);
    vector<ArrayDevice*> mockDevs;

    EXPECT_CALL(*mockState, CanAddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, AddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillRepeatedly(ReturnRef(mockDevs));
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(0));

    Array array("mock", NULL, mockArrDevMgr, &mockDevMgr, mockPtnMgr, mockState, NULL, &mockEventScheduler, NULL, mockPbrAdapter);

    // When: we try to add a spare device
    int actual = array.AddSpare("mock-spare");

    // Then: we should receive 0 as a return
    ASSERT_EQ(0, actual);
}

TEST(Array, Delete_testIfArrayDeletedWhenArrayIsBrokenAndShutdownNotEnded)
{
    // Given: a deletable and broken state
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    EXPECT_CALL(*mockState, IsDeletable).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, Clear).Times(0);
    EXPECT_CALL(*mockState, SetDelete).Times(0);
    EXPECT_CALL(*mockRebuilder, IsRebuilding).WillOnce(Return(false));
    EXPECT_CALL(*mockState, WaitShutdownDone).WillOnce(Return(EID(DELETE_ARRAY_TIMED_OUT)));
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));

    Array array("mock", mockRebuilder, mockArrDevMgr, NULL, mockPtnMgr, mockState, NULL, NULL, NULL);
    // When: array is broken but shutdown process is not finished
    int actual = array.Delete();

    // Then
    ASSERT_EQ(EID(DELETE_ARRAY_TIMED_OUT), actual);

    // CleanUp
    delete mockRebuilder;
}

TEST(Array, AddSpare_testIfSpareIsAddedWhenDeviceIsAlreadyInOtherArray)
{
    // Given: a happy path scenario
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockDeviceManager mockDevMgr;
    MockEventScheduler mockEventScheduler;
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockState, CanAddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, AddSpare).WillOnce(Return(EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED)));

    Array array("mock", NULL, mockArrDevMgr, &mockDevMgr, mockPtnMgr, mockState, NULL, &mockEventScheduler, NULL);
    int expected = EID(UNABLE_TO_ADD_SSD_ALREADY_OCCUPIED);
    // When: we try to add a spare device
    int actual = array.AddSpare("mock-spare");

    // Then
    ASSERT_EQ(expected, actual);
}

TEST(Array, AddSpare_testIfSpareIsNotAddedWhenStateIsNotProper)
{
    // Given : state which cannot add spare
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockEventScheduler mockEventScheduler;
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    int STATE_CANNOT_ADD_SPARE = -1;
    EXPECT_CALL(*mockState, CanAddSpare).WillOnce(Return(STATE_CANNOT_ADD_SPARE));
    EXPECT_CALL(*mockArrDevMgr, AddSpare).Times(0);
    EXPECT_CALL(mockEventScheduler, EnqueueEvent).Times(0);
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));

    Array array("mock", NULL, mockArrDevMgr, NULL, mockPtnMgr, mockState, NULL, &mockEventScheduler, NULL);

    // When: we try to add a spare device
    int actual = array.AddSpare("mock-spare");

    // Then
    ASSERT_EQ(STATE_CANNOT_ADD_SPARE, actual);
}

TEST(Array, AddSpare_testIfSpareIsNotAddedWhenFailedToAddSpare)
{
    // Given: when adding spare failed
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockDeviceManager mockDevMgr;
    MockEventScheduler mockEventScheduler;
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    string spareDevName = "spareDev";
    MockUBlockDevice* rawPtr = new MockUBlockDevice(spareDevName, 1024, nullptr);
    UblockSharedPtr mockSpareDev = shared_ptr<MockUBlockDevice>(rawPtr);
    int FAILED_TO_ADD_SPARE = -1;
    EXPECT_CALL(*mockState, CanAddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, AddSpare).WillOnce(Return(FAILED_TO_ADD_SPARE));
    EXPECT_CALL(mockEventScheduler, EnqueueEvent).Times(0);
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));

    Array array("mock", NULL, mockArrDevMgr, &mockDevMgr, mockPtnMgr, mockState, NULL, &mockEventScheduler, NULL);

    // When: we try to add a spare device
    int actual = array.AddSpare(spareDevName);

    // Then
    ASSERT_EQ(FAILED_TO_ADD_SPARE, actual);
}

TEST(Array, AddSpare_testIfSpareIsNotAddedWhenFailedToFlush)
{
    // Given: when flush failed
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockEventScheduler mockEventScheduler;
    MockDeviceManager mockDevMgr;
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    string spareDevName = "spareDev";
    MockUBlockDevice* rawPtr = new MockUBlockDevice(spareDevName, 1024, nullptr);
    UblockSharedPtr mockSpareDev = shared_ptr<MockUBlockDevice>(rawPtr);
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();
    int FAILED_TO_FLUSH = -1;
    vector<ArrayDevice*> devs;
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockState, CanAddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, AddSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(FAILED_TO_FLUSH));
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillOnce(ReturnRef((devs)));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));

    Array array("mock", NULL, mockArrDevMgr, &mockDevMgr, mockPtnMgr, mockState, NULL, &mockEventScheduler, NULL, mockPbrAdapter);

    // When: we try to add a spare device
    int actual = array.AddSpare(spareDevName);

    // Then
    ASSERT_EQ(FAILED_TO_FLUSH, actual);
}

TEST(Array, RemoveSpare_testIfSpareIsRemovedWhenInputsAreValid)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();
    string mockSpareName = "mock-spare";
    vector<ArrayDevice*> devs;

    EXPECT_CALL(*mockState, CanRemoveSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(mockSpareName)).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillOnce(ReturnRef((devs)));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(0));

    Array array("mock", NULL, mockArrDevMgr, NULL, mockPtnMgr, mockState, NULL, NULL, NULL, mockPbrAdapter);

    // When
    int actual = array.RemoveSpare(mockSpareName);

    // Then
    ASSERT_EQ(0, actual);
}

TEST(Array, RemoveSpare_testIfSpareIsNotRemovedWhenStateIsNotProper)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    string mockSpareName = "mock-spare";

    int STATE_CANNOT_REMOVE_SPARE = -1;
    EXPECT_CALL(*mockState, CanRemoveSpare).WillOnce(Return(STATE_CANNOT_REMOVE_SPARE));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(mockSpareName)).Times(0);

    Array array("mock", NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    int actual = array.RemoveSpare(mockSpareName);

    // Then
    ASSERT_EQ(STATE_CANNOT_REMOVE_SPARE, actual);
}

TEST(Array, RemoveSpare_testIfSpareIsNotRemovedWhenFailedToRemoveSpare)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    string mockSpareName = "mock-spare";

    int FAILED_TO_REMOVE_SPARE = -1;
    EXPECT_CALL(*mockState, CanRemoveSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(mockSpareName)).WillOnce(Return(FAILED_TO_REMOVE_SPARE));

    Array array("mock", NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    int actual = array.RemoveSpare(mockSpareName);

    // Then
    ASSERT_EQ(FAILED_TO_REMOVE_SPARE, actual);
}

TEST(Array, RemoveSpare_testIfSpareIsNotRemovedWhenFailedToFlush)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();

    string mockSpareName = "mock-spare";

    int FAILED_TO_FLUSH = -1;
    vector<ArrayDevice*> devs;
    EXPECT_CALL(*mockState, CanRemoveSpare).WillOnce(Return(0));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(mockSpareName)).WillOnce(Return(0));
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(FAILED_TO_FLUSH));
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillOnce(ReturnRef((devs)));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));

    Array array("mock", NULL, mockArrDevMgr, NULL, mockPtnMgr, mockState, NULL, NULL, NULL, mockPbrAdapter);

    // When
    int actual = array.RemoveSpare(mockSpareName);

    // Then
    ASSERT_EQ(FAILED_TO_FLUSH, actual);
}

TEST(Array, DetachDevice_testIfSpareDeviceIsSuccessfullyDetachedFromUnmountedArray)
{
    // Given: a spare device and unmounted array
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockDeviceManager mockSysDevMgr(nullptr);
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    string spareDevName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(spareDevName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    vector<ArrayDevice*> mockGetDevRes = {mockArrDev};

    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetName).WillRepeatedly(Return("mock-unvme"));
    EXPECT_CALL(*mockArrDev, GetType).WillOnce(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(*mockArrDevMgr, RemoveSpare(spareDevName)).WillOnce(Return(0));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(false));

    Array array("mock", mockArrayRebuilder, mockArrDevMgr, &mockSysDevMgr, mockPtnMgr, mockState, NULL, NULL, NULL);

    // When: detachDevice() is invoked
    int actual = array.DetachDevice(mockArrDev);

    // Then: the operation should be successful
    ASSERT_EQ(0, actual);
    DestroyFakeNvmeNamespace(fakeNs); // just for cleanup
    delete mockArrDev;
}

TEST(Array, DetachDevice_testIfDataDeviceIsSuccessfullyDetachedFromUnmountedArray)
{
    // Given: a data device and unmounted array
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockDeviceManager mockSysDevMgr(nullptr);
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);
    string devName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(devName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    vector<ArrayDevice*> mockGetDevRes = {mockArrDev};
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    EXPECT_CALL(*mockState, RaidStateUpdated).Times(1);
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::FAULT)).Times(1);
    EXPECT_CALL(*mockArrDev, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*mockArrDev, GetType).WillOnce(Return(ArrayDeviceType::DATA));

    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockArrDev, SetUblock).Times(1);
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(false));
    EXPECT_CALL(*mockPtnMgr, GetRaidState).WillOnce(Return(RaidState::DEGRADED));

    Array array("mock", mockArrayRebuilder, mockArrDevMgr, &mockSysDevMgr, mockPtnMgr, mockState, NULL, NULL, NULL);

    // When: DetachDevice() is invoked
    int actual = array.DetachDevice(mockArrDev);

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
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockDeviceManager mockSysDevMgr(nullptr);
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    string devName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(devName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    vector<ArrayDevice*> mockGetDevRes = {mockArrDev};
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();

    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillRepeatedly(ReturnRef(mockGetDevRes));
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockArrDev, GetName).WillRepeatedly(Return(devName));
    EXPECT_CALL(*mockArrDev, GetSerial).WillRepeatedly(Return(""));
    EXPECT_CALL(*mockArrDev, GetDataIndex).WillRepeatedly(Return(0));
    EXPECT_CALL(*mockArrDev, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::FAULT)).Times(1);
    EXPECT_CALL(*mockArrDev, SetUblock).Times(1);

    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockState, RaidStateUpdated).Times(1);
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(true));
    EXPECT_CALL(*mockPtnMgr, GetRaidState).WillOnce(Return(RaidState::NORMAL));
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(0));

    Array array("mock", mockArrayRebuilder, mockArrDevMgr, &mockSysDevMgr, mockPtnMgr, mockState, NULL, NULL, NULL, mockPbrAdapter);
    // When: DetachDevice() is invoked
    int actual = array.DetachDevice(mockArrDev);

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
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock");
    MockDeviceManager mockSysDevMgr(nullptr);
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();

    string devName = "mock-unvme";
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeUblockSharedPtr = make_shared<UnvmeSsd>(devName, 1024, nullptr, fakeNs, "mock-addr"); // ok to use real obj
    MockArrayDevice* mockArrDev = new MockArrayDevice(fakeUblockSharedPtr);                                        // the param here doesn't matter in fact
    vector<ArrayDevice*> mockGetDevRes = {mockArrDev};

    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(fakeUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockArrDev, GetName).WillOnce(Return(devName));
    EXPECT_CALL(*mockArrDev, GetType).WillOnce(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockState, RaidStateUpdated).Times(0); // should never be called

    Array array("mock", mockArrayRebuilder, mockArrDevMgr, &mockSysDevMgr, mockPtnMgr, mockState, NULL, NULL, NULL);

    // When: DetachDevice is invoked
    int actual = array.DetachDevice(mockArrDev);

    // Then: the operation should succeed(?): TODO(hsung): please check whether the return value should be zero.
    ASSERT_EQ(0, actual);
    delete mockArrDev;
}

TEST(Array, MountDone_testIfResumeRebuildEventIsSent)
{
    // Given: an array
    MockEventScheduler mockEventScheduler;
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDeviceManager* mockArrayDeviceManager = new MockArrayDeviceManager(NULL, "mock");
    string mockDevName = "mockDevName";
    MockUBlockDevice* mockUblockDevice = new MockUBlockDevice(mockDevName, 1024, NULL);
    UblockSharedPtr mockUblockSharedPtr = shared_ptr<UBlockDevice>(mockUblockDevice);
    MockArrayDevice* mockArrayDevice = new MockArrayDevice(NULL);
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();

    vector<ArrayDevice*> devs{mockArrayDevice};
    EXPECT_CALL(*mockArrayDeviceManager, GetDevs).WillRepeatedly(ReturnRef(devs));
    EXPECT_CALL(*mockArrayDevice, GetUblock).WillRepeatedly(Return(mockUblockSharedPtr));
    EXPECT_CALL(*mockArrayDevice, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockArrayDevice, GetState).WillRepeatedly(Return(ArrayDeviceState::REBUILD));
    EXPECT_CALL(*mockArrayDevice, GetName).WillRepeatedly(Return(mockDevName));
    EXPECT_CALL(*mockArrayDevice, GetDataIndex).WillRepeatedly(Return(0));
    EXPECT_CALL(*mockArrayDevice, GetSerial).WillRepeatedly(Return(""));
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(true));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(0));

    Array array("mock", NULL, mockArrayDeviceManager, NULL, mockPtnMgr, mockState, NULL, &mockEventScheduler, NULL, mockPbrAdapter);

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
    Array array("mock", NULL, NULL, NULL, NULL, mockState, NULL, NULL, NULL);

    int expected = 121212; // return some random event id
    EXPECT_CALL(*mockState, IsUnmountable).WillOnce(Return(expected));

    // When: CheckUnmountable is invoked
    int actual = array.CheckUnmountable();

    // Then: we should query state object and get the result
    ASSERT_EQ(expected, actual);
}

TEST(Array, GetSizeInfo_testIfPartitionManagerIsQueriedOn)
{
    // Given: an array and a partition manager
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    MockPartitionManager* mockPartMgr = new MockPartitionManager();
    PartitionType partType = PartitionType::USER_DATA;
    PartitionLogicalSize logicalSize;
    EXPECT_CALL(*mockPartMgr, GetSizeInfo(partType)).WillOnce(Return(&logicalSize));

    Array array("mock", NULL, NULL, NULL, mockPartMgr, NULL, NULL, NULL, NULL);

    // When: GetSizeInfo() is invoked
    array.GetSizeInfo(partType);

    // Then: we should ask partition manager to get us the result (defined by EXPECT_CALL)
}

TEST(Array, GetState_testIfStateIsQueriedOn)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    Array array("mock", NULL, NULL, NULL, NULL, mockState, NULL, NULL, NULL);

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
    int expectedProgress = 0;
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
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);

    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(true));
    EXPECT_CALL(*mockState, IsRecoverable).Times(0); // should never be called

    Array array("mock-array", mockArrayRebuilder, NULL, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    int actual = array.IsRecoverable(NULL, NULL);

    // Then
    ASSERT_EQ(actual, static_cast<int>(IoRecoveryRetType::FAIL));
}

TEST(Array, IsRecoverable_testIfUnmountedArrayIsNotRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDevice* mockArrDev = new MockArrayDevice(NULL);
    MockUBlockDevice* uBlockPtr = new MockUBlockDevice("mock-dev", 1024, NULL);
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);

    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(false));

    Array array("mock-array", mockArrayRebuilder, NULL, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    int actual = array.IsRecoverable(mockArrDev, uBlockPtr);

    // Then
    ASSERT_EQ(actual, static_cast<int>(IoRecoveryRetType::FAIL));
}

TEST(Array, IsRecoverable_testIfArrayFailingToTranslateIsNotRecoverable)
{
    // Given
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockArrayDevice* mockArrDev = new MockArrayDevice(NULL);
    MockUBlockDevice* uBlockPtr = nullptr; // Simulate translate/convert failure
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);

    EXPECT_CALL(*mockState, IsBroken).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillOnce(Return(true));

    Array array("mock-array", mockArrayRebuilder, NULL, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    bool actual = array.IsRecoverable(mockArrDev, uBlockPtr);

    // Then: TODO(hsung): please check whether "true" is intended. IsRecoverable() is returning true when uBlock is null.
    ASSERT_EQ(actual, static_cast<int>(IoRecoveryRetType::SUCCESS));
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
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
    MockArrayRebuilder* mockArrayRebuilder = new MockArrayRebuilder(NULL);
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();
    vector<ArrayDevice*> mockDevs;

    EXPECT_CALL(*mockState, IsBroken).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockState, IsMounted).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockState, RaidStateUpdated(RaidState::DEGRADED)).Times(1);
    EXPECT_CALL(*mockArrDev, GetUblock).WillRepeatedly(Return(mockUblockSharedPtr));
    EXPECT_CALL(*mockArrDev, GetName).WillRepeatedly(Return(dataDevName));
    EXPECT_CALL(*mockArrDev, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockArrDev, SetUblock).Times(1);
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::FAULT)).Times(1);
    EXPECT_CALL(mockSysDevMgr, RemoveDevice).Times(1);
    EXPECT_CALL(*mockState, IsRebuildable).WillOnce(Return(false));
    EXPECT_CALL(*mockState, IsRecoverable).WillOnce(Return(true));
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillRepeatedly(ReturnRef(mockDevs));
    EXPECT_CALL(*mockPtnMgr, GetRaidState).WillRepeatedly(Return(RaidState::DEGRADED));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(0));

    Array array("mock-array", mockArrayRebuilder, mockArrDevMgr, &mockSysDevMgr, mockPtnMgr, mockState, NULL, NULL, NULL, mockPbrAdapter);

    // When
    bool actual = array.IsRecoverable(mockArrDev, mockUblockDevPtr);

    // Then
    ASSERT_EQ(actual, static_cast<int>(IoRecoveryRetType::SUCCESS));
    delete mockArrDev;
}

TEST(Array, FindDevice_testIfArrayDevMgrIsQueriedAgainst)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
    string mockSerialNumber = "mock-sn";

    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    vector<ArrayDevice*> mockGetDevRes = {mockArrDev};
    EXPECT_CALL(*mockArrDevMgr, GetDevs()).WillOnce(ReturnRef(mockGetDevRes));
    EXPECT_CALL(*mockArrDev, GetSerial()).WillOnce(Return(mockSerialNumber));

    Array array("mock-array", NULL, mockArrDevMgr, NULL, NULL, NULL, NULL, NULL, NULL);

    // When
    IArrayDevice* actual = array.FindDevice("wrong_serial");

    // Then
    ASSERT_EQ(nullptr, actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfNonFaultyArrayDeviceCanSuccessfullyTriggerRebuild)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::NORMAL)); // NORMAL or REBUILD?

    Array array("mock-array", NULL, mockArrDevMgr, NULL, NULL, NULL, NULL, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(vector<IArrayDevice*>{mockArrDev});

    // Then
    ASSERT_TRUE(actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfFaultyArrayDeviceDoesNotRetryWhenTheStateIsntSetToRebuild)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(false));
    EXPECT_CALL(*mockArrDev, GetUblock).WillOnce(Return(nullptr));

    Array array("mock-array", NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(vector<IArrayDevice*>{mockArrDev});

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfFaultyArrayDeviceDoesNotRetryRebuildDueToReplaceFailure)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
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

    Array array("mock-array", NULL, mockArrDevMgr, NULL, NULL, mockState, NULL, NULL, NULL);

    // When
    bool actual = array.TriggerRebuild(vector<IArrayDevice*>{mockArrDev});

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;
}

TEST(Array, TriggerRebuild_testIfFaultyArrayDeviceDoesNotNeedToRetryAfterTriggeringRebuild)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockPartitionServices* mockSvc = new MockPartitionServices;
    MockArrayRebuilder mockArrayRebuilder(NULL);
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    pbr::MockPbrAdapter* mockPbrAdapter = new pbr::MockPbrAdapter();

    int REPLACE_SUCCESS = 0;
    std::list<RebuildTarget*> emptyTargets;
    vector<ArrayDevice*> mockDevs;

    EXPECT_CALL(*mockArrDev, GetState).WillOnce(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockArrDev, GetUblock).WillOnce(Return(nullptr));
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(true));
    EXPECT_CALL(*mockArrDevMgr, ReplaceWithSpare).WillOnce(Return(REPLACE_SUCCESS));
    EXPECT_CALL(*mockArrDev, SetState(ArrayDeviceState::REBUILD)).Times(1);
    EXPECT_CALL(*mockSvc, GetRebuildTargets).WillOnce(Return(emptyTargets));
    EXPECT_CALL(mockArrayRebuilder, Rebuild).Times(AtLeast(0)); // simply, ignore
    EXPECT_CALL(*mockPtnMgr, GetRaidType).WillRepeatedly(Return(RaidTypeEnum::RAID5));
    EXPECT_CALL(*mockPtnMgr, GetPartitions).WillOnce(Return(vector<Partition*>()));
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillOnce(ReturnRef(mockDevs));
    EXPECT_CALL(*mockPbrAdapter, Update).WillOnce(Return(0));

    Array array("mock-array", &mockArrayRebuilder, mockArrDevMgr, NULL, mockPtnMgr, mockState, mockSvc, NULL, NULL, mockPbrAdapter);

    // When
    bool actual = array.TriggerRebuild(vector<IArrayDevice*>{mockArrDev});

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;

    usleep(10000); // intentionally put some jitter to avoid signal propagated from internally-spawned thread
}

// TODO(srm): Fix the issue of spawning a thread during UT context
TEST(Array, DISABLED_ResumeRebuild_testIfResumeRebuildFailedWhenStateChangeFailed)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockPartitionServices* mockSvc = new MockPartitionServices;
    MockArrayRebuilder mockArrayRebuilder(NULL);
    std::list<RebuildTarget*> emptyTargets;

    int REPLACE_SUCCESS = 0;
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(false));
    EXPECT_CALL(*mockSvc, GetRebuildTargets).Times(0);
    EXPECT_CALL(mockArrayRebuilder, Rebuild).Times(0);

    Array array("mock-array", &mockArrayRebuilder, mockArrDevMgr, NULL, NULL, mockState, mockSvc, NULL, NULL);

    // When
    bool actual = array.ResumeRebuild(vector<IArrayDevice*>{mockArrDev});

    // Then
    ASSERT_FALSE(actual);
    delete mockArrDev;
}

// TODO(srm): Fix the issue of spawning a thread during UT context
TEST(Array, DISABLED_ResumeRebuild_testIfResumeRebuildProperly)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "name");
    MockArrayDevice* mockArrDev = new MockArrayDevice(nullptr);
    NiceMock<MockIStateControl> mockIStateControl;
    MockArrayState* mockState = new MockArrayState(&mockIStateControl);
    MockPartitionServices* mockSvc = new MockPartitionServices;
    MockArrayRebuilder mockArrayRebuilder(NULL);
    std::list<RebuildTarget*> emptyTargets;

    int REPLACE_SUCCESS = 0;
    EXPECT_CALL(*mockState, SetRebuild).WillOnce(Return(true));
    EXPECT_CALL(*mockSvc, GetRebuildTargets).WillRepeatedly(Return(emptyTargets));

    Array array("mock-array", &mockArrayRebuilder, mockArrDevMgr, NULL, NULL, mockState, mockSvc, NULL, NULL);

    // When
    bool actual = array.ResumeRebuild(vector<IArrayDevice*>{mockArrDev});

    // Then
    ASSERT_TRUE(actual);
    delete mockArrDev;
}

TEST(Array, Shutdown_testIfPartitionManagerCleansUp)
{
    // Given
    MockArrayDeviceManager* mockArrDevMgr = new MockArrayDeviceManager(NULL, "mock-array");
    MockPartitionManager* mockPtnMgr = new MockPartitionManager();
    MockArrayServiceLayer* mockArrayService = new MockArrayServiceLayer;

    NiceMock<MockIStateControl> mockStateControl;
    MockArrayState* mockArrayState = new MockArrayState(&mockStateControl);

    Array array("mock-array", NULL, mockArrDevMgr, NULL, mockPtnMgr, mockArrayState, NULL, NULL, mockArrayService);
    vector<ArrayDevice*> mockDevs;

    EXPECT_CALL(*mockArrayService, Unregister).Times(1);
    EXPECT_CALL(*mockArrDevMgr, GetDevs).WillOnce(ReturnRef(mockDevs));

    // When
    array.Shutdown();
    // Then

    // Cleanup
    delete mockArrayService;
}
} // namespace pos
