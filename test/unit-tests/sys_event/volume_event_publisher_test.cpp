#include "src/sys_event/volume_event_publisher.h"

#include "src/include/pos_event_id.h"
#include "test/unit-tests/allocator/allocator_mock.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/journal_manager/log_write/i_journal_volume_event_handler_mock.h"
#include "test/unit-tests/mapper/i_mapper_volume_event_handler_mock.h"
#include "test/unit-tests/metadata/meta_volume_event_handler_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(VolumeEventPublisher, VolumeEventPublisher_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    // When
    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    delete volumeEventPublisher;
}

TEST(VolumeEventPublisher, NotifyVolumeCreated_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    
    VolumeEventBase volumeEventBase = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    VolumeEventPerf volumeEventPerf = {
        .maxiops = 1024*1024,
        .maxbw = 1024*1024};

    VolumeArrayInfo volumeArrayInfo = {
        .arrayId = arrayID,
        .arrayName = arrayName};

    NiceMock<MockMetaVolumeEventHandler> metaVolumeEventHandler(&info, &mapper, &allocator, &journal);

    volumeEventPublisher->RegisterSubscriber(&metaVolumeEventHandler, arrayName, arrayID);

    bool expected, actual;

    // When
    ON_CALL(metaVolumeEventHandler, VolumeCreated(_, _, _)).WillByDefault(Return(EID(VOL_EVENT_FAIL)));

    expected = volumeEventPublisher->NotifyVolumeCreated(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);
    actual = false;

    // Then
    ASSERT_EQ(actual, expected);

    delete volumeEventPublisher;
}

TEST(VolumeEventPublisher, NotifyVolumeUpdated_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    
    VolumeEventBase volumeEventBase = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    VolumeEventPerf volumeEventPerf = {
        .maxiops = 1024*1024,
        .maxbw = 1024*1024};

    VolumeArrayInfo volumeArrayInfo = {
        .arrayId = arrayID,
        .arrayName = arrayName};

    NiceMock<MockMetaVolumeEventHandler> metaVolumeEventHandler(&info, &mapper, &allocator, &journal);

    volumeEventPublisher->RegisterSubscriber(&metaVolumeEventHandler, arrayName, arrayID);

    bool expected, actual;

    // When
    ON_CALL(metaVolumeEventHandler, VolumeUpdated(_, _, _)).WillByDefault(Return(EID(VOL_EVENT_FAIL)));

    expected = volumeEventPublisher->NotifyVolumeUpdated(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);
    actual = false;

    // Then
    ASSERT_EQ(actual, expected);

    delete volumeEventPublisher;
}

TEST(VolumeEventPublisher, NotifyVolumeDeleted_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    
    VolumeEventBase volumeEventBase = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    VolumeArrayInfo volumeArrayInfo = {
        .arrayId = arrayID,
        .arrayName = arrayName};

    NiceMock<MockMetaVolumeEventHandler> metaVolumeEventHandler(&info, &mapper, &allocator, &journal);

    volumeEventPublisher->RegisterSubscriber(&metaVolumeEventHandler, arrayName, arrayID);

    bool expected, actual;

    // When
    ON_CALL(metaVolumeEventHandler, VolumeDeleted(_,_)).WillByDefault(Return(EID(VOL_EVENT_FAIL)));

    expected = volumeEventPublisher->NotifyVolumeDeleted(&volumeEventBase,&volumeArrayInfo);
    actual = false;

    // Then
    ASSERT_EQ(actual, expected);

    delete volumeEventPublisher;
}

TEST(VolumeEventPublisher, NotifyVolumeMounted_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    
    VolumeEventBase volumeEventBase = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    VolumeEventPerf volumeEventPerf = {
        .maxiops = 1024*1024,
        .maxbw = 1024*1024};

    VolumeArrayInfo volumeArrayInfo = {
        .arrayId = arrayID,
        .arrayName = arrayName};

    NiceMock<MockMetaVolumeEventHandler> metaVolumeEventHandler(&info, &mapper, &allocator, &journal);

    volumeEventPublisher->RegisterSubscriber(&metaVolumeEventHandler, arrayName, arrayID);

    bool expected, actual;

    // When
    ON_CALL(metaVolumeEventHandler, VolumeMounted(_, _, _)).WillByDefault(Return(EID(VOL_EVENT_FAIL)));

    expected = volumeEventPublisher->NotifyVolumeMounted(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);
    actual = false;

    // Then
    ASSERT_EQ(actual, expected);

    delete volumeEventPublisher;
}

TEST(VolumeEventPublisher, NotifyVolumeUnmounted_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    
    VolumeEventBase volumeEventBase = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    VolumeArrayInfo volumeArrayInfo = {
        .arrayId = arrayID,
        .arrayName = arrayName};

    NiceMock<MockMetaVolumeEventHandler> metaVolumeEventHandler(&info, &mapper, &allocator, &journal);

    volumeEventPublisher->RegisterSubscriber(&metaVolumeEventHandler, arrayName, arrayID);

    bool expected, actual;

    // When
    ON_CALL(metaVolumeEventHandler, VolumeUnmounted(_,_)).WillByDefault(Return(EID(VOL_EVENT_FAIL)));

    expected = volumeEventPublisher->NotifyVolumeUnmounted(&volumeEventBase, &volumeArrayInfo);
    actual = false;

    // Then
    ASSERT_EQ(actual, expected);

    delete volumeEventPublisher;
}

TEST(VolumeEventPublisher, NotifyVolumeLoaded_)
{
    // Given
    std::string arrayName = "";
    int arrayID = 0;
    std::string tag = "TestTag";

    VolumeEventPublisher* volumeEventPublisher = new VolumeEventPublisher();

    NiceMock<MockIArrayInfo> info;
    NiceMock<MockIMapperVolumeEventHandler> mapper;
    NiceMock<MockAllocator> allocator;
    NiceMock<MockIJournalVolumeEventHandler> journal;
    
    VolumeEventBase volumeEventBase = {
        .volId = 2,
        .volSizeByte = 100,
        .volName = "testVolume",
        .uuid = "",
        .subnqn = ""};

    VolumeEventPerf volumeEventPerf = {
        .maxiops = 1024*1024,
        .maxbw = 1024*1024};

    VolumeArrayInfo volumeArrayInfo = {
        .arrayId = arrayID,
        .arrayName = arrayName};

    NiceMock<MockMetaVolumeEventHandler> metaVolumeEventHandler(&info, &mapper, &allocator, &journal);

    volumeEventPublisher->RegisterSubscriber(&metaVolumeEventHandler, arrayName, arrayID);

    bool expected, actual;

    // When
    ON_CALL(metaVolumeEventHandler, VolumeLoaded(_, _, _)).WillByDefault(Return(EID(VOL_EVENT_FAIL)));

    expected = volumeEventPublisher->NotifyVolumeLoaded(&volumeEventBase, &volumeEventPerf, &volumeArrayInfo);
    actual = false;

    // Then
    ASSERT_EQ(actual, expected);

    delete volumeEventPublisher;
}

} // namespace pos
