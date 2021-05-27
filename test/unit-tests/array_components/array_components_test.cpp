#include "src/array_components/array_components.h"

#include <gtest/gtest.h>

#include <functional>

#include "src/include/pos_event_id.h"
#include "test/unit-tests/allocator/allocator_mock.h"
#include "test/unit-tests/allocator/i_context_manager_mock.h"
#include "test/unit-tests/allocator/i_wbstripe_allocator_mock.h"
#include "test/unit-tests/array/array_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/gc/garbage_collector_mock.h"
#include "test/unit-tests/state/interface/i_state_control_mock.h"
#include "test/unit-tests/state/state_control_mock.h"
#include "test/unit-tests/state/state_manager_mock.h"
#include "test/unit-tests/volume/volume_manager_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
static auto mockMetaFsFactory = [](Array* array, bool isLoaded) {
    return nullptr; // returning null MetaFs intentionally
};

TEST(ArrayComponents, ArrayComponents_testConstructorWithNullPtrs)
{
    // Given: nothing
    NiceMock<MockStateManager> mockStateManager;

    // When
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

    // Then
}

TEST(ArrayComponents, Create_testIfRemoveStateIsInvokedWhenCreationFails)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

    EXPECT_CALL(*mockArray, Create).WillOnce(Return(EID(ARRAY_BROKEN_ERROR)));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(2); // one by Create() and the other by destructor

    // When
    int actual = arrayComps.Create(DeviceSet<string>(), "mock-raidtype");

    // Then
    ASSERT_EQ(EID(ARRAY_BROKEN_ERROR), actual);
}

TEST(ArrayComponents, Create_testIfGetStateAndSubscribeIsInvokedWhenCreationSucceeds)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockStateControl> mockStateControl;
    MockVolumeManager* mockVolMgr = new MockVolumeManager(nullptr, &mockStateControl);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, &mockStateControl, mockArray,
        mockVolMgr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

    EXPECT_CALL(*mockArray, Create).WillOnce(Return(0));
    EXPECT_CALL(mockStateManager, GetStateControl).WillOnce(Return(&mockStateControl));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(1);

    // When
    int actual = arrayComps.Create(DeviceSet<string>(), "mock-raidtype");

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayComponents, Load_testIfLoadFailureIsPropagated)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

    int LOAD_FAILURE = 123;
    EXPECT_CALL(*mockArray, Load).WillOnce(Return(LOAD_FAILURE));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(2);

    // When
    int actual = arrayComps.Load();

    // Then
    ASSERT_EQ(LOAD_FAILURE, actual);
}

TEST(ArrayComponents, Load_testIfSuccessfulLoadSetsMountSequence)
{
    // Given
    MockStateManager mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    NiceMock<MockStateControl> mockStateControl;
    MockVolumeManager* mockVolMgr = new MockVolumeManager(nullptr, &mockStateControl);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, &mockStateControl, mockArray,
        mockVolMgr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

    EXPECT_CALL(*mockArray, Load).WillOnce(Return(0));
    EXPECT_CALL(mockStateManager, GetStateControl).WillOnce(Return(&mockStateControl));
    EXPECT_CALL(mockStateManager, RemoveStateControl).Times(1);

    // When
    int actual = arrayComps.Load();

    // Then
    ASSERT_EQ(0, actual);
}

TEST(ArrayComponents, Mount_skipped)
{
    // TODO(srm): ArrayMountSequence isn't injected from the outside.
    // Given that the business logic of Mount() is trivial, I will be skipping this test case for now.
}

TEST(ArrayComponents, Unmount_skipped)
{
    // TODO(srm): ArrayMountSequence isn't injected from the outside.
    // Given that the business logic of Unmount() is trivial, I will be skipping this test case for now.
}

TEST(ArrayComponents, Delete_testIfDeleteResultIsPropagated)
{
    // Given
    NiceMock<MockStateManager> mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

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
    NiceMock<MockStateManager> mockStateManager;
    NiceMock<MockIArrayInfo> mockIArrayInfo;
    MockAllocator* mockAllocator = new MockAllocator(nullptr, nullptr, nullptr, nullptr, &mockIArrayInfo, nullptr);
    MockGarbageCollector* mockGc = new MockGarbageCollector(nullptr, nullptr);
    MockIWBStripeAllocator mockIwbStripeAllocator;
    MockIContextManager mockIContextManager;

    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr,
        nullptr, nullptr, mockGc, nullptr, mockAllocator, nullptr, nullptr, mockMetaFsFactory);
    int PREPARE_RESULT = 234; // the actual value does not matter
    std::string ARRAY_NAME = "mock-array";
    EXPECT_CALL(mockIArrayInfo, GetName).WillRepeatedly(Return(ARRAY_NAME));
    EXPECT_CALL(*mockAllocator, GetIWBStripeAllocator).WillOnce(Return(&mockIwbStripeAllocator));
    EXPECT_CALL(*mockAllocator, GetIContextManager).WillOnce(Return(&mockIContextManager));
    EXPECT_CALL(*mockGc, Pause).Times(1);
    EXPECT_CALL(mockIwbStripeAllocator, PrepareRebuild).WillOnce(Return(PREPARE_RESULT));
    EXPECT_CALL(mockIContextManager, NeedRebuildAgain).WillOnce(Return(false));
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
    MockAllocator* mockAllocator = new MockAllocator(nullptr, nullptr, nullptr, nullptr, &mockIArrayInfo, nullptr);
    MockIWBStripeAllocator mockIwbStripeAllocator;

    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr,
        nullptr, nullptr, nullptr, nullptr, mockAllocator, nullptr, nullptr, mockMetaFsFactory);

    std::string ARRAY_NAME = "mock-array";
    EXPECT_CALL(mockIArrayInfo, GetName).WillRepeatedly(Return(ARRAY_NAME));
    EXPECT_CALL(*mockAllocator, GetIWBStripeAllocator).WillOnce(Return(&mockIwbStripeAllocator));
    EXPECT_CALL(mockIwbStripeAllocator, StopRebuilding).Times(1);

    // When
    arrayComps.RebuildDone();

    // Then: verify the mock method invocations
}

TEST(ArrayComponents, GetArray_testGetter)
{
    // Given
    NiceMock<MockStateManager> mockStateManager;
    MockArray* mockArray = new MockArray("mock-array", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    ArrayComponents arrayComps("mock-array", nullptr, nullptr, &mockStateManager, nullptr, mockArray,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, mockMetaFsFactory);

    // When
    Array* actual = arrayComps.GetArray();

    // Then
    ASSERT_TRUE(mockArray == actual);
}

} // namespace pos

