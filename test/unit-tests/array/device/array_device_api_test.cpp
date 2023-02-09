#include "src/array/device/array_device_api.h"

#include <gtest/gtest.h>

#include "src/device/unvme/unvme_ssd.h"
#include "test/unit-tests/array/device/array_device_mock.h"
#include "test/unit-tests/utils/spdk_util.h"
#include "src/include/pos_event_id.h"
#include "src/include/array_config.h"

using ::testing::Return;
namespace pos
{
TEST(ArrayDeviceApi, ArrayDeviceApi_testFindDevByNameWhenThereIsAMatchingDevice)
{
    // Given
    string targetName = "dev3";

    MockArrayDevice* expectedTarget = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev4 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetName).WillRepeatedly(Return("dev1"));
    EXPECT_CALL(*mockDev2, GetName).WillRepeatedly(Return("dev2"));
    EXPECT_CALL(*expectedTarget, GetName).WillRepeatedly(Return(targetName));
    EXPECT_CALL(*mockDev4, GetName).WillRepeatedly(Return("dev4"));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, expectedTarget, mockDev4 };

    // When
    ArrayDevice* dev = ArrayDeviceApi::FindDevByName(targetName, devs);

    // Then
    ASSERT_EQ(dev, expectedTarget);

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testFindDevByNameWhenThereIsNoMatchingDevice)
{
    // Given
    string wrongTargetName = "dev10";

    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetName).WillRepeatedly(Return("dev1"));
    EXPECT_CALL(*mockDev2, GetName).WillRepeatedly(Return("dev2"));
    EXPECT_CALL(*mockDev3, GetName).WillRepeatedly(Return("dev3"));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    ArrayDevice* dev = ArrayDeviceApi::FindDevByName(wrongTargetName, devs);

    // Then
    ASSERT_EQ(nullptr, dev);

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testFindDevBySnWhenThereIsAMatchingDevice)
{
    // Given
    string targetSn = "sn3";

    MockArrayDevice* expectedTarget = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev4 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetSerial).WillRepeatedly(Return("sn1"));
    EXPECT_CALL(*mockDev2, GetSerial).WillRepeatedly(Return("sn2"));
    EXPECT_CALL(*expectedTarget, GetSerial).WillRepeatedly(Return(targetSn));
    EXPECT_CALL(*mockDev4, GetSerial).WillRepeatedly(Return("sn4"));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, expectedTarget, mockDev4 };

    // When
    ArrayDevice* dev = ArrayDeviceApi::FindDevBySn(targetSn, devs);

    // Then
    ASSERT_EQ(dev, expectedTarget);

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testFindDevBySnWhenThereIsNoMatchingDevice)
{
    // Given
    string wrongTargetSn= "sn10";

    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetSerial).WillRepeatedly(Return("sn1"));
    EXPECT_CALL(*mockDev2, GetSerial).WillRepeatedly(Return("sn2"));
    EXPECT_CALL(*mockDev3, GetSerial).WillRepeatedly(Return("sn3"));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    ArrayDevice* dev = ArrayDeviceApi::FindDevBySn(wrongTargetSn, devs);

    // Then
    ASSERT_EQ(nullptr, dev);

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testExtractDevicesByState)
{
    // Given
    string wrongTargetSn= "sn10";

    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::REBUILD));
    EXPECT_CALL(*mockDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::REBUILD));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    auto normalDevs = ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::NORMAL, devs);
    auto rebuildDevs = ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::REBUILD, devs);
    auto faultDevs = ArrayDeviceApi::ExtractDevicesByState(ArrayDeviceState::FAULT, devs);

    // Then
    ASSERT_EQ(1, normalDevs.size());
    ASSERT_EQ(2, rebuildDevs.size());
    ASSERT_EQ(0, faultDevs.size());

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testExtractDevicesByType)
{
    // Given
    string wrongTargetSn= "sn10";

    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    auto nvms = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::NVM, devs);
    auto dataDevs = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::DATA, devs);
    auto spareDevs = ArrayDeviceApi::ExtractDevicesByType(ArrayDeviceType::SPARE, devs);

    // Then
    ASSERT_EQ(0, nvms.size());
    ASSERT_EQ(2, dataDevs.size());
    ASSERT_EQ(1, spareDevs.size());

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}


TEST(ArrayDeviceApi, ArrayDeviceApi_testGetMinimumCapacity)
{
    // Given
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    uint64_t minSize = 1024 * 1024;
    EXPECT_CALL(*mockDev1, GetSize).WillRepeatedly(Return(minSize * 2));
    EXPECT_CALL(*mockDev2, GetSize).WillRepeatedly(Return(minSize * 3));
    EXPECT_CALL(*mockDev3, GetSize).WillRepeatedly(Return(minSize));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    uint64_t actual = ArrayDeviceApi::GetMinimumCapacity(devs);

    // Then
    ASSERT_EQ(actual, minSize);

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }

}

TEST(ArrayDeviceApi, ArrayDeviceApi_testImportInspectionWhenNvmNotExist)
{
    // Given
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    int actual = ArrayDeviceApi::ImportInspection(devs);

    // Then
    ASSERT_EQ(actual, EID(IMPORT_DEVICE_NVM_DOES_NOT_EXIST));

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testImportInspectionWhenNvmExistButIsNullptr)
{
    // Given
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetUblock).WillRepeatedly(Return(nullptr));
    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    int actual = ArrayDeviceApi::ImportInspection(devs);

    // Then
    ASSERT_EQ(actual, EID(IMPORT_DEVICE_NVM_DOES_NOT_EXIST));

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testImportInspectionWhenNoActiveDataSsd)
{
    // Given
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeNvmUblock = make_shared<UnvmeSsd>("nvm", 1024, nullptr, fakeNs, "mock-addr");

    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    EXPECT_CALL(*mockDev1, GetUblock).WillRepeatedly(Return(fakeNvmUblock)); /* to pass the nullptr check of the nvm ublock */
    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::REBUILD));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    int actual = ArrayDeviceApi::ImportInspection(devs);

    // Then
    ASSERT_EQ(actual, EID(IMPORT_DEVICE_NO_AVAILABLE_DEVICE));

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testImportInspectionWhenSsdSizeIsLessThanMinimum)
{
    // Given
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeNvmUblock = make_shared<UnvmeSsd>("nvm", 1024, nullptr, fakeNs, "mock-addr");

    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);

    uint64_t satisfyMinSize = ArrayConfig::MINIMUM_SSD_SIZE_BYTE;
    uint64_t lessThanMinSize = ArrayConfig::MINIMUM_SSD_SIZE_BYTE - 1;

    EXPECT_CALL(*mockDev1, GetUblock).WillRepeatedly(Return(fakeNvmUblock)); /* to pass the nullptr check of the nvm ublock */
    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev2, GetSize).WillRepeatedly(Return(satisfyMinSize));
    EXPECT_CALL(*mockDev3, GetSize).WillRepeatedly(Return(lessThanMinSize));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3 };

    // When
    int actual = ArrayDeviceApi::ImportInspection(devs);

    // Then
    ASSERT_EQ(actual, EID(IMPORT_DEVICE_SSD_CAPACITY_IS_LT_MIN));

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testImportInspectionWhenSpareSsdSizeIsSmallerThanTheSmallestAmongDataSsds)
{
    // Given
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeNvmUblock = make_shared<UnvmeSsd>("nvm", 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev4 = new MockArrayDevice(nullptr);

    uint64_t minDataSsdSize = ArrayConfig::MINIMUM_SSD_SIZE_BYTE * 10; /* Large enough ssd size */
    uint64_t smallerSizeThanTheSmallestAmongDataSsds = minDataSsdSize - 1; /* Minimum size is satisfied, but smaller than the smallest device among data ssds */

    EXPECT_CALL(*mockDev1, GetUblock).WillRepeatedly(Return(fakeNvmUblock)); /* to pass the nullptr check of the nvm ublock */
    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev4, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(*mockDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev4, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev2, GetSize).WillRepeatedly(Return(minDataSsdSize * 2));
    EXPECT_CALL(*mockDev3, GetSize).WillRepeatedly(Return(minDataSsdSize));
    EXPECT_CALL(*mockDev4, GetSize).WillRepeatedly(Return(smallerSizeThanTheSmallestAmongDataSsds));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3, mockDev4 };

    // When
    int actual = ArrayDeviceApi::ImportInspection(devs);

    // Then
    ASSERT_EQ(actual, EID(IMPORT_DEVICE_SPARE_CAPACITY_IS_LT_DATA));

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}

TEST(ArrayDeviceApi, ArrayDeviceApi_testImportInspectionWhenSpareSsdSizeIsUnsatisfiedButItIsInactive)
{
    // Given
    struct spdk_nvme_ns* fakeNs = BuildFakeNvmeNamespace();
    UblockSharedPtr fakeNvmUblock = make_shared<UnvmeSsd>("nvm", 1024, nullptr, fakeNs, "mock-addr");
    MockArrayDevice* mockDev1 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev2 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev3 = new MockArrayDevice(nullptr);
    MockArrayDevice* mockDev4 = new MockArrayDevice(nullptr);

    uint64_t minDataSsdSize = ArrayConfig::MINIMUM_SSD_SIZE_BYTE * 10; /* Large enough ssd size */
    uint64_t lessThanMinSize = ArrayConfig::MINIMUM_SSD_SIZE_BYTE - 1;

    EXPECT_CALL(*mockDev1, GetUblock).WillRepeatedly(Return(fakeNvmUblock)); /* to pass the nullptr check of the nvm ublock */
    EXPECT_CALL(*mockDev1, GetType).WillRepeatedly(Return(ArrayDeviceType::NVM));
    EXPECT_CALL(*mockDev2, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev3, GetType).WillRepeatedly(Return(ArrayDeviceType::DATA));
    EXPECT_CALL(*mockDev4, GetType).WillRepeatedly(Return(ArrayDeviceType::SPARE));
    EXPECT_CALL(*mockDev1, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev2, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev3, GetState).WillRepeatedly(Return(ArrayDeviceState::NORMAL));
    EXPECT_CALL(*mockDev4, GetState).WillRepeatedly(Return(ArrayDeviceState::FAULT));
    EXPECT_CALL(*mockDev2, GetSize).WillRepeatedly(Return(minDataSsdSize * 2));
    EXPECT_CALL(*mockDev3, GetSize).WillRepeatedly(Return(minDataSsdSize));
    EXPECT_CALL(*mockDev4, GetSize).WillRepeatedly(Return(lessThanMinSize));

    vector<ArrayDevice*> devs { mockDev1, mockDev2, mockDev3, mockDev4 };

    // When
    int actual = ArrayDeviceApi::ImportInspection(devs);

    // Then
    ASSERT_EQ(actual, 0);

    // Clean up
    for (auto d : devs)
    {
        delete d;
    }
}
}
