#include "src/array/partition/partition_manager.h"

#include <gtest/gtest.h>

#include "src/array/partition/partition_services.h"
#include "src/array/device/array_device.h"
#include "src/include/array_config.h"
#include "src/include/array_device_state.h"
#include "src/helper/calc/calc.h"
#include "test/unit-tests/array/partition/partition_services_mock.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/array/interface/i_abr_control_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "test/unit-tests/cpu_affinity/affinity_manager_mock.h"
#include "test/unit-tests/utils/mock_builder.h"
#include "test/unit-tests/io_scheduler/io_dispatcher_mock.h"

using ::testing::_;
using ::testing::Return;
namespace pos
{

TEST(PartitionManager, PartitionManager_testConstructor)
{
    // Given
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();

    // When
    PartitionManager pm;

    // Then
}

TEST(PartitionManager, GetSizeInfo_testIfNullIsReturnedForUninitializedPartition)
{
    // Given
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    PartitionManager pm;

    // When
    auto actual = pm.GetSizeInfo(PartitionType::META_NVM);

    // Then
    ASSERT_EQ(nullptr, actual);
}

// strategy 1) use real Partition(s) objects which are instantiated within a test target method
// strategy 2) inject factory that instantiates Partition(s)
// I'm choosing strategy 1 to minimize changes on the src side, but the latter may be more preferred in general
TEST(PartitionManager, CreateAll_DeleteAll_testIfAllPartitionsAreNewlyCreatedAndDeletedSuccessfully)
{
    // Given 1
    using MockUblockSharedPtr = std::shared_ptr<MockUBlockDevice>;

    MockIAbrControl mockIAbrControl;
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    PartitionManager pm;

    shared_ptr<MockUBlockDevice> ptrMockUblockDev1 = make_shared<MockUBlockDevice>("mock-dataDev1", 1024, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev2 = make_shared<MockUBlockDevice>("mock-dataDev2", 1024, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev3 = make_shared<MockUBlockDevice>("mock-dataDev3", 1024, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev4 = make_shared<MockUBlockDevice>("mock-dataDev4", 1024, nullptr);
    MockArrayDevice dataDev1(ptrMockUblockDev1), dataDev2(ptrMockUblockDev2), dataDev3(ptrMockUblockDev3), dataDev4(ptrMockUblockDev4);
    vector<ArrayDevice*> data = {&dataDev1, &dataDev2, &dataDev3, &dataDev4};
    EXPECT_CALL(dataDev1, GetUblock).WillRepeatedly(Return(ptrMockUblockDev1));
    EXPECT_CALL(dataDev2, GetUblock).WillRepeatedly(Return(ptrMockUblockDev2));
    EXPECT_CALL(dataDev3, GetUblock).WillRepeatedly(Return(ptrMockUblockDev3));
    EXPECT_CALL(dataDev4, GetUblock).WillRepeatedly(Return(ptrMockUblockDev4));

    MockArrayDevice bufDev(nullptr);
    vector<ArrayDevice*> buf = {&bufDev};
    shared_ptr<MockUBlockDevice> ptrMockUblockNvmDev = make_shared<MockUBlockDevice>("mock-nvmDev", 0, nullptr);
    EXPECT_CALL(bufDev, GetUblock).WillRepeatedly(Return(ptrMockUblockNvmDev)); // 'cause this would become "baseline"

    uint64_t DATA_DEV_SIZE = ArrayConfig::SSD_SEGMENT_SIZE_BYTE * 10; // in bytes
    uint64_t NVM_DEV_SIZE = ArrayConfig::SSD_SEGMENT_SIZE_BYTE * 1;  // in bytes

    MockPartitionServices svc;

    EXPECT_CALL(dataDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev4, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));

    // set up mocks for _CreateMetaSsd
    EXPECT_CALL(*ptrMockUblockDev1, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev2, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev3, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev4, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));

    EXPECT_CALL(svc, AddTranslator(PartitionType::JOURNAL_SSD, _)).Times(1);
    EXPECT_CALL(svc, AddRecover(PartitionType::JOURNAL_SSD, _)).Times(1);

    EXPECT_CALL(svc, AddTranslator(PartitionType::META_SSD, _)).Times(1);
    EXPECT_CALL(svc, AddRecover(PartitionType::META_SSD, _)).Times(1);

    // set up mocks for _CreateUserData
    EXPECT_CALL(*ptrMockUblockNvmDev.get(), GetSize).WillRepeatedly(Return(NVM_DEV_SIZE));
    EXPECT_CALL(svc, AddTranslator(PartitionType::USER_DATA, _)).Times(1);
    EXPECT_CALL(svc, AddRecover(PartitionType::USER_DATA, _)).Times(1);

    // set up mocks for _CreateMetaNvm
    EXPECT_CALL(svc, AddTranslator(PartitionType::META_NVM, _)).Times(1);

    // set up mocks for _CreateWriteBuffer
    EXPECT_CALL(svc, AddTranslator(PartitionType::WRITE_BUFFER, _)).Times(1);

    // verify the number of invocations
    EXPECT_CALL(svc, AddRebuildTarget).Times(3); // journal ssd ,meta ssd, and user data ssd

    // When 1: PartitionManager creates all partitions
    int actual = pm.CreatePartitions(buf.front(), data, RaidTypeEnum::RAID10, RaidTypeEnum::RAID5, &svc);
    // Then 1: validate against expected number of segments of each partition type.
    ASSERT_EQ(0, actual);
    uint32_t expectedMbrSegments = ArrayConfig::MBR_SIZE_BYTE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;

    uint32_t expectedJournalPartSegments = ArrayConfig::JOURNAL_PART_SEGMENT_SIZE;
    ASSERT_EQ(expectedJournalPartSegments, pm.GetSizeInfo(PartitionType::JOURNAL_SSD)->totalSegments);

    uint32_t expectedMetaPartTotalSegments = DIV_ROUND_UP(
        DATA_DEV_SIZE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE * ArrayConfig::META_SSD_SIZE_RATIO,
        (uint64_t)(100));
    ASSERT_EQ(expectedMetaPartTotalSegments, pm.GetSizeInfo(PartitionType::META_SSD)->totalSegments);

    uint32_t expectedDataTotalSegments =
        DATA_DEV_SIZE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE - expectedMbrSegments - expectedJournalPartSegments - expectedMetaPartTotalSegments;
    ASSERT_EQ(expectedDataTotalSegments, pm.GetSizeInfo(PartitionType::USER_DATA)->totalSegments);

    uint32_t expectedMetaNvmTotalSegments = ArrayConfig::NVM_SEGMENT_SIZE;
    ASSERT_EQ(expectedMetaNvmTotalSegments, pm.GetSizeInfo(PartitionType::META_NVM)->totalSegments);

    uint32_t expectedWriteBufferTotalSegments = ArrayConfig::NVM_SEGMENT_SIZE;
    ASSERT_EQ(expectedWriteBufferTotalSegments, pm.GetSizeInfo(PartitionType::WRITE_BUFFER)->totalSegments);

    uint64_t ssdPartStartLba = ArrayConfig::SSD_PARTITION_START_LBA;
    const PartitionPhysicalSize* jourPs = pm.GetPhysicalSize(PartitionType::JOURNAL_SSD);
    const PartitionPhysicalSize* metaPs = pm.GetPhysicalSize(PartitionType::META_SSD);
    const PartitionPhysicalSize* dataPs = pm.GetPhysicalSize(PartitionType::USER_DATA);
    uint64_t expectedDataLastLba = dataPs->startLba + 
        static_cast<uint64_t>(ArrayConfig::SECTORS_PER_BLOCK) *
        dataPs->blksPerChunk * dataPs->stripesPerSegment *
        expectedDataTotalSegments -1;

    ASSERT_EQ(ssdPartStartLba, jourPs->startLba);
    ASSERT_EQ(jourPs->lastLba + 1, metaPs->startLba);
    ASSERT_EQ(metaPs->lastLba + 1, dataPs->startLba);
    ASSERT_EQ(expectedDataLastLba, pm.GetPhysicalSize(PartitionType::USER_DATA)->lastLba);

    // When 2: PartitionManager deletes all partitions
    pm.DeletePartitions();

    // Then 2
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::JOURNAL_SSD));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::USER_DATA));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::META_NVM));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::META_SSD));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::WRITE_BUFFER));
}

TEST(PartitionManager, Create_testIfBetweenHeterogeneousDevicesSuccessfully)
{
    // Given 1
    using MockUblockSharedPtr = std::shared_ptr<MockUBlockDevice>;

    MockIAbrControl mockIAbrControl;
    MockAffinityManager mockAffMgr = BuildDefaultAffinityManagerMock();
    PartitionManager pm;

    shared_ptr<MockUBlockDevice> ptrMockUblockDev1 = make_shared<MockUBlockDevice>("mock-dataDev1", 1024, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev2 = make_shared<MockUBlockDevice>("mock-dataDev2", 1024, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev3 = make_shared<MockUBlockDevice>("mock-dataDev3", 1024, nullptr);
    shared_ptr<MockUBlockDevice> ptrMockUblockDev4 = make_shared<MockUBlockDevice>("mock-dataDev4", 1024, nullptr);
    MockArrayDevice dataDev1(ptrMockUblockDev1), dataDev2(ptrMockUblockDev2), dataDev3(ptrMockUblockDev3), dataDev4(ptrMockUblockDev4);
    vector<ArrayDevice*> data = {&dataDev1, &dataDev2, &dataDev3, &dataDev4};
    EXPECT_CALL(dataDev1, GetUblock).WillRepeatedly(Return(ptrMockUblockDev1));
    EXPECT_CALL(dataDev2, GetUblock).WillRepeatedly(Return(ptrMockUblockDev2));
    EXPECT_CALL(dataDev3, GetUblock).WillRepeatedly(Return(ptrMockUblockDev3));
    EXPECT_CALL(dataDev4, GetUblock).WillRepeatedly(Return(ptrMockUblockDev4));

    MockArrayDevice bufDev(nullptr);
    vector<ArrayDevice*> buf = {&bufDev};
    shared_ptr<MockUBlockDevice> ptrMockUblockNvmDev = make_shared<MockUBlockDevice>("mock-nvmDev", 0, nullptr);
    EXPECT_CALL(bufDev, GetUblock).WillRepeatedly(Return(ptrMockUblockNvmDev)); // 'cause this would become "baseline"

    uint64_t DATA_DEV_SIZE = ArrayConfig::SSD_SEGMENT_SIZE_BYTE * 20; // in bytes
    uint64_t HETERO_DEV_SIZE = ArrayConfig::SSD_SEGMENT_SIZE_BYTE * 15; // in bytes
    uint64_t NVM_DEV_SIZE = ArrayConfig::SSD_SEGMENT_SIZE_BYTE * 1;  // in bytes

    MockPartitionServices svc;

    EXPECT_CALL(dataDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(dataDev4, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));

    // set up mocks for _CreateMetaSsd
    EXPECT_CALL(*ptrMockUblockDev1, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev2, GetSize).WillRepeatedly(Return(HETERO_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev3, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));
    EXPECT_CALL(*ptrMockUblockDev4, GetSize).WillRepeatedly(Return(DATA_DEV_SIZE));
    EXPECT_CALL(svc, AddTranslator(PartitionType::JOURNAL_SSD, _)).Times(1);
    EXPECT_CALL(svc, AddRecover(PartitionType::JOURNAL_SSD, _)).Times(1);

    EXPECT_CALL(svc, AddTranslator(PartitionType::META_SSD, _)).Times(1);
    EXPECT_CALL(svc, AddRecover(PartitionType::META_SSD, _)).Times(1);

    // set up mocks for _CreateUserData
    EXPECT_CALL(*ptrMockUblockNvmDev.get(), GetSize).WillRepeatedly(Return(NVM_DEV_SIZE));
    EXPECT_CALL(svc, AddTranslator(PartitionType::USER_DATA, _)).Times(1);
    EXPECT_CALL(svc, AddRecover(PartitionType::USER_DATA, _)).Times(1);

    // set up mocks for _CreateMetaNvm
    EXPECT_CALL(svc, AddTranslator(PartitionType::META_NVM, _)).Times(1);

    // set up mocks for _CreateWriteBuffer
    EXPECT_CALL(svc, AddTranslator(PartitionType::WRITE_BUFFER, _)).Times(1);

    // verify the number of invocations
    EXPECT_CALL(svc, AddRebuildTarget).Times(3); // journal ssd ,meta ssd, and user data ssd

    // When 1: PartitionManager creates all partitions
    int actual = pm.CreatePartitions(buf.front(), data, RaidTypeEnum::RAID10, RaidTypeEnum::RAID5, &svc);
    // Then 1: validate against expected number of segments of each partition type.
    ASSERT_EQ(0, actual);
    uint32_t expectedMbrSegments = ArrayConfig::MBR_SIZE_BYTE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE;

    uint32_t expectedJournalPartSegments = ArrayConfig::JOURNAL_PART_SEGMENT_SIZE;
    ASSERT_EQ(expectedJournalPartSegments, pm.GetSizeInfo(PartitionType::JOURNAL_SSD)->totalSegments);

    uint32_t expectedMetaPartTotalSegments = DIV_ROUND_UP(
        HETERO_DEV_SIZE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE * ArrayConfig::META_SSD_SIZE_RATIO,
        (uint64_t)(100));
    ASSERT_EQ(expectedMetaPartTotalSegments, pm.GetSizeInfo(PartitionType::META_SSD)->totalSegments);

    uint32_t expectedDataTotalSegments =
        HETERO_DEV_SIZE / ArrayConfig::SSD_SEGMENT_SIZE_BYTE - expectedMbrSegments - expectedJournalPartSegments - expectedMetaPartTotalSegments;
    ASSERT_EQ(expectedDataTotalSegments, pm.GetSizeInfo(PartitionType::USER_DATA)->totalSegments);

    int expectedMetaNvmTotalSegments = ArrayConfig::NVM_SEGMENT_SIZE;
    ASSERT_EQ(expectedMetaNvmTotalSegments, pm.GetSizeInfo(PartitionType::META_NVM)->totalSegments);

    int expectedWriteBufferTotalSegments = ArrayConfig::NVM_SEGMENT_SIZE;
    ASSERT_EQ(expectedWriteBufferTotalSegments, pm.GetSizeInfo(PartitionType::WRITE_BUFFER)->totalSegments);

    uint64_t ssdPartStartLba = ArrayConfig::SSD_PARTITION_START_LBA;
    const PartitionPhysicalSize* jourPs = pm.GetPhysicalSize(PartitionType::JOURNAL_SSD);
    const PartitionPhysicalSize* metaPs = pm.GetPhysicalSize(PartitionType::META_SSD);
    const PartitionPhysicalSize* dataPs = pm.GetPhysicalSize(PartitionType::USER_DATA);
    uint64_t expectedDataLastLba = dataPs->startLba + 
        static_cast<uint64_t>(ArrayConfig::SECTORS_PER_BLOCK) *
        dataPs->blksPerChunk * dataPs->stripesPerSegment *
        expectedDataTotalSegments -1;

    ASSERT_EQ(ssdPartStartLba, jourPs->startLba);
    ASSERT_EQ(jourPs->lastLba + 1, metaPs->startLba);
    ASSERT_EQ(metaPs->lastLba + 1, dataPs->startLba);
    ASSERT_EQ(expectedDataLastLba, pm.GetPhysicalSize(PartitionType::USER_DATA)->lastLba);

    // When 2: PartitionManager deletes all partitions
    pm.DeletePartitions();

    // Then 2
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::JOURNAL_SSD));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::USER_DATA));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::META_NVM));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::META_SSD));
    ASSERT_EQ(nullptr, pm.GetSizeInfo(PartitionType::WRITE_BUFFER));
}

} // namespace pos
