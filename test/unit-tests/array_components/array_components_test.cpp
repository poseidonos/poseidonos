#include "src/array_components/array_components.h"

#include <gtest/gtest.h>

#include <functional>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/gc/garbage_collector_mock.h"
#include "test/unit-tests/metadata/metadata_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"
#include "test/unit-tests/state/state_control_mock.h"
#include "test/unit-tests/state/state_manager_mock.h"
#include "test/unit-tests/volume/volume_manager_mock.h"
#include "test/unit-tests/array_components/array_mount_sequence_mock.h"
#include "test/unit-tests/array/rebuild/i_array_rebuilder_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
static auto mockMetaFsFactory = [](Array* array, bool isLoaded) {
    return nullptr; // returning null MetaFs intentionally
};

TEST(ArrayComponents, ArrayComponents_testShortConstructorWithNullPtrs)
{
    // Given: nothing
    NiceMock<MockStateManager> mockStateManager;

    // When
    ArrayComponents arrayComps("mock-array", nullptr, nullptr);
    // Then
}

TEST(ArrayComponents, ArrayComponents_testConstructorWithNullPtrs)
{
    // Given: nothing
    NiceMock<MockStateManager> mockStateManager;

    // When
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    // Then
}

TEST(ArrayComponents, Create_testIfRemoveStateIsInvokedWhenCreationFails)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    EXPECT_CALL(*mockArray, Create).WillOnce(Return(EID(ARRAY_BROKEN_ERROR)));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(1); // one by Create() and the other by destructor

    // When
    int actual = arrayComps.Create(DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(EID(ARRAY_BROKEN_ERROR), actual);
}

TEST(ArrayComponents, Create_testIfGetStateAndSubscribeIsInvokedWhenCreationSucceeds)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockStateControl> mockStateControl;
    MockVolumeManager* mockVolMgr = new MockVolumeManager(nullptr, &mockStateControl);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, &mockStateControl, mockArray,
        mockVolMgr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    EXPECT_CALL(*mockArray, Create).WillOnce([=](DeviceSet<string> nameSet, string dataRaidType, unsigned int& arrayIndex)
    {
        arrayIndex = 0;
        return 0;
    });
    EXPECT_CALL(mockStateManager, GetStateControl).WillOnce(Return(&mockStateControl));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(1);

    // When
    int actual = arrayComps.Create(DeviceSet<string>(), "RAID10", "RAID5");

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayComponents, Load_testIfLoadFailureIsPropagated)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    int LOAD_FAILURE = 123;
    EXPECT_CALL(*mockArray, Load).WillOnce(Return(LOAD_FAILURE));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(1);

    // When
    int actual = arrayComps.Load();

    // Then
    ASSERT_EQ(LOAD_FAILURE, actual);
}

TEST(ArrayComponents, Load_testIfSuccessfulLoadSetsMountSequence)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockStateControl> mockStateControl;
    MockVolumeManager* mockVolMgr = new MockVolumeManager(nullptr, &mockStateControl);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, &mockStateControl, mockArray,
        mockVolMgr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    EXPECT_CALL(*mockArray, Load).WillOnce([=](unsigned int& arrayIndex)
    {
        arrayIndex = 0;
        return 0;
    });
    EXPECT_CALL(mockStateManager, GetStateControl).WillOnce(Return(&mockStateControl));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(1);

    // When
    int actual = arrayComps.Load();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayComponents, Mount_testUsingArrayMountSequenceMock)
{
    // Given
    vector<IMountSequence*> emptySeq;
    MockStateControl stateControl;
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();
    EXPECT_CALL(stateControl, Subscribe).Times(1);
    EXPECT_CALL(stateControl, Unsubscribe).Times(1);
    MockArrayMountSequence* mockArrayMountSequence = new MockArrayMountSequence(emptySeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, nullptr, mockRebuilder);
    EXPECT_CALL(*mockArrayMountSequence, Mount).WillOnce(Return(0));

    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*mockArray, SetPreferences).Times(1);
    EXPECT_CALL(*mockArray, MountDone).Times(1);
    ArrayComponents arrayComponents("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr, nullptr, mockArrayMountSequence);
    // When
    int result = arrayComponents.Mount(false);

    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
}

TEST(ArrayComponents, Unmount_testUsingArrayMountSequenceMock)
{
    // Given
    vector<IMountSequence*> emptySeq;
    MockStateControl stateControl;
    MockIArrayRebuilder* mockRebuilder = new MockIArrayRebuilder();
    EXPECT_CALL(stateControl, Subscribe).Times(1);
    EXPECT_CALL(stateControl, Unsubscribe).Times(1);
    MockArrayMountSequence* mockArrayMountSequence = new MockArrayMountSequence(emptySeq, &stateControl, "mock-array", nullptr, nullptr, nullptr, nullptr, mockRebuilder);
    EXPECT_CALL(*mockArrayMountSequence, Unmount).WillOnce(Return(0));

    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComponents("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr, mockArrayMountSequence);
    // When
    int result = arrayComponents.Unmount();

    // Then
    int SUCCESS = 0;
    EXPECT_EQ(SUCCESS, result);
}

TEST(ArrayComponents, Delete_testIfDeleteResultIsPropagated)
{
    // Given
    NiceMock<MockStateManager> mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    int DELETE_RESULT = 123;
    EXPECT_CALL(*mockArray, Delete).WillOnce(Return(DELETE_RESULT));

    // When
    int actual = arrayComps.Delete();

    // Then
    ASSERT_EQ(DELETE_RESULT, actual);
}

TEST(ArrayComponents, PrepareRebuild_testIfGcIsPausedAndResumedAroundAllocatorPreparingRebuild)
{
    // Given
    MockStateManager mockStateManager;
    NiceMock<MockStateControl> mockStateControl;
    NiceMock<MockIArrayInfo> mockIArrayInfo;
    MockMetadata* mockMetadata = new MockMetadata;
    MockGarbageCollector* mockGc = new MockGarbageCollector(nullptr, nullptr);

    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, nullptr,
        nullptr, mockGc, mockMetadata, nullptr, mockMetaFsFactory, nullptr);
    int PREPARE_RESULT = 234; // the actual value does not matter
    StateContext expected("test", SituationEnum::REBUILDING);
    EXPECT_CALL(mockStateControl, GetState).WillOnce(Return(&expected));

    std::string ARRAY_NAME = "mock-array";
    EXPECT_CALL(mockIArrayInfo, GetName).WillRepeatedly(Return(ARRAY_NAME));
    EXPECT_CALL(*mockGc, Pause).Times(1);
    EXPECT_CALL(*mockMetadata, PrepareRebuild).WillOnce(Return(PREPARE_RESULT));
    EXPECT_CALL(*mockMetadata, NeedRebuildAgain).WillOnce(Return(false));
    EXPECT_CALL(*mockGc, Resume).Times(1);

    // When
    bool resume = false;
    int actual = arrayComps.PrepareRebuild(resume);

    // Then
    ASSERT_EQ(PREPARE_RESULT, actual);
    ASSERT_EQ(false, resume);
}

TEST(ArrayComponents, RebuildDone_)
{
    // Given
    NiceMock<MockStateManager> mockStateManager;
    NiceMock<MockIArrayInfo> mockIArrayInfo;
    MockMetadata* mockMetadata = new MockMetadata;

    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, nullptr,
        nullptr, nullptr, mockMetadata, nullptr, mockMetaFsFactory, nullptr);

    std::string ARRAY_NAME = "mock-array";
    EXPECT_CALL(mockIArrayInfo, GetName).WillRepeatedly(Return(ARRAY_NAME));
    EXPECT_CALL(*mockMetadata, StopRebuilding).Times(1);

    // When
    arrayComps.RebuildDone();

    // Then: verify the mock method invocations
}

TEST(ArrayComponents, GetArray_testGetter)
{
    // Given
    NiceMock<MockStateManager> mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory, nullptr);

    // When
    Array* actual = arrayComps.GetArray();

    // Then
    ASSERT_TRUE(mockArray == actual);
}

} // namespace pos
