#include <gtest/gtest.h>
#include <vector>
#include "src/array_mgmt/numa_awared_array_creation.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/device/base/ublock_device_mock.h"
#include "src/include/pos_event_id.h"

using ::testing::_;
using ::testing::Return;

namespace pos
{
TEST(NumaAwaredArrayCreation, NumaAwaredArrayCreation_testIfOneOptionIsAvailable)
{
    // Given
    MockDeviceManager mockDevMgr(nullptr);

    shared_ptr<MockUBlockDevice> mockWriteBufferDev = make_shared<MockUBlockDevice>("uram0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev1 = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev2 = make_shared<MockUBlockDevice>("unvme-ns-1", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev3 = make_shared<MockUBlockDevice>("unvme-ns-2", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev1 = make_shared<MockUBlockDevice>("unvme-ns-3", 0, nullptr);

    EXPECT_CALL(*mockWriteBufferDev.get(), GetName).WillRepeatedly(Return("uram0"));
    EXPECT_CALL(*mockDataDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-0"));
    EXPECT_CALL(*mockDataDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-1"));
    EXPECT_CALL(*mockDataDev3.get(), GetName).WillRepeatedly(Return("unvme-ns-2"));
    EXPECT_CALL(*mockSpareDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-3"));

    uint64_t homoSize = (uint64_t)100 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    EXPECT_CALL(*mockDataDev1.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev2.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev3.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockSpareDev1.get(), GetSize).WillRepeatedly(Return(homoSize));

    int homoNumaId = 1;
    EXPECT_CALL(*mockWriteBufferDev.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev2.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev3.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockSpareDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));

    DeviceClass system = DeviceClass::SYSTEM;
    EXPECT_CALL(*mockWriteBufferDev.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev2.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev3.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev1.get(), GetClass).WillRepeatedly(Return(system));

    std::vector<shared_ptr<UBlockDevice>> devs;
    devs.push_back(mockDataDev1);
    devs.push_back(mockDataDev2);
    devs.push_back(mockDataDev3);
    devs.push_back(mockSpareDev1);

    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockWriteBufferDev));
    EXPECT_CALL(mockDevMgr, GetDevs).WillRepeatedly(Return(devs));

    // When
    NumaAwaredArrayCreation naac({"uram0"}, 3, 1, &mockDevMgr);
    NumaAwaredArrayCreationResult result = naac.GetResult();
    int actual = result.code;

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
    ASSERT_EQ(1, result.options.size());
}

TEST(NumaAwaredArrayCreation, NumaAwaredArrayCreation_testIfMultipleOptionsInSameNumaAreAvailable)
{
    // Given
    MockDeviceManager mockDevMgr(nullptr);

    shared_ptr<MockUBlockDevice> mockWriteBufferDev = make_shared<MockUBlockDevice>("uram0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev1 = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev2 = make_shared<MockUBlockDevice>("unvme-ns-1", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev3 = make_shared<MockUBlockDevice>("unvme-ns-2", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev4 = make_shared<MockUBlockDevice>("unvme-ns-3", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev5 = make_shared<MockUBlockDevice>("unvme-ns-4", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev6 = make_shared<MockUBlockDevice>("unvme-ns-5", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev7 = make_shared<MockUBlockDevice>("unvme-ns-6", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev8 = make_shared<MockUBlockDevice>("unvme-ns-7", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev2 = make_shared<MockUBlockDevice>("unvme-ns-8", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev1 = make_shared<MockUBlockDevice>("unvme-ns-9", 0, nullptr);

    EXPECT_CALL(*mockWriteBufferDev.get(), GetName).WillRepeatedly(Return("uram0"));
    EXPECT_CALL(*mockDataDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-0"));
    EXPECT_CALL(*mockDataDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-1"));
    EXPECT_CALL(*mockDataDev3.get(), GetName).WillRepeatedly(Return("unvme-ns-2"));
    EXPECT_CALL(*mockDataDev4.get(), GetName).WillRepeatedly(Return("unvme-ns-3"));
    EXPECT_CALL(*mockDataDev5.get(), GetName).WillRepeatedly(Return("unvme-ns-4"));
    EXPECT_CALL(*mockDataDev6.get(), GetName).WillRepeatedly(Return("unvme-ns-5"));
    EXPECT_CALL(*mockDataDev7.get(), GetName).WillRepeatedly(Return("unvme-ns-6"));
    EXPECT_CALL(*mockDataDev8.get(), GetName).WillRepeatedly(Return("unvme-ns-7"));
    EXPECT_CALL(*mockSpareDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-8"));
    EXPECT_CALL(*mockSpareDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-9"));

    uint64_t size = (uint64_t)100 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    uint64_t anotherSize = (uint64_t)200 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    EXPECT_CALL(*mockDataDev1.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockDataDev2.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockDataDev3.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockDataDev4.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockDataDev5.get(), GetSize).WillRepeatedly(Return(anotherSize));
    EXPECT_CALL(*mockDataDev6.get(), GetSize).WillRepeatedly(Return(anotherSize));
    EXPECT_CALL(*mockDataDev7.get(), GetSize).WillRepeatedly(Return(anotherSize));
    EXPECT_CALL(*mockDataDev8.get(), GetSize).WillRepeatedly(Return(anotherSize));
    EXPECT_CALL(*mockSpareDev1.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockSpareDev2.get(), GetSize).WillRepeatedly(Return(anotherSize));

    int homoNumaId = 1;
    EXPECT_CALL(*mockWriteBufferDev.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev2.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev3.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev4.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev5.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev6.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev7.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev8.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockSpareDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockSpareDev2.get(), GetNuma).WillRepeatedly(Return(homoNumaId));

    DeviceClass system = DeviceClass::SYSTEM;
    EXPECT_CALL(*mockWriteBufferDev.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev2.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev3.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev4.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev5.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev6.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev7.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev8.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev2.get(), GetClass).WillRepeatedly(Return(system));

    std::vector<shared_ptr<UBlockDevice>> devs;
    devs.push_back(mockDataDev1);
    devs.push_back(mockDataDev2);
    devs.push_back(mockDataDev3);
    devs.push_back(mockDataDev4);
    devs.push_back(mockDataDev5);
    devs.push_back(mockDataDev6);
    devs.push_back(mockDataDev7);
    devs.push_back(mockDataDev8);
    devs.push_back(mockSpareDev1);
    devs.push_back(mockSpareDev2);

    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockWriteBufferDev));
    EXPECT_CALL(mockDevMgr, GetDevs).WillRepeatedly(Return(devs));

    // When
    NumaAwaredArrayCreation naac({"uram0"}, 3, 1, &mockDevMgr);
    NumaAwaredArrayCreationResult result = naac.GetResult();
    int actual = result.code;

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
    ASSERT_EQ(2, result.options.size());
}

TEST(NumaAwaredArrayCreation, NumaAwaredArrayCreation_testIfMultipleOptionsInterNumaAreAvailable)
{
    // Given
    MockDeviceManager mockDevMgr(nullptr);

    shared_ptr<MockUBlockDevice> mockWriteBufferDev = make_shared<MockUBlockDevice>("uram0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev1 = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev2 = make_shared<MockUBlockDevice>("unvme-ns-1", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev3 = make_shared<MockUBlockDevice>("unvme-ns-2", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev4 = make_shared<MockUBlockDevice>("unvme-ns-3", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev5 = make_shared<MockUBlockDevice>("unvme-ns-4", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev6 = make_shared<MockUBlockDevice>("unvme-ns-5", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev7 = make_shared<MockUBlockDevice>("unvme-ns-6", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev8 = make_shared<MockUBlockDevice>("unvme-ns-7", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev2 = make_shared<MockUBlockDevice>("unvme-ns-8", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev1 = make_shared<MockUBlockDevice>("unvme-ns-9", 0, nullptr);

    EXPECT_CALL(*mockWriteBufferDev.get(), GetName).WillRepeatedly(Return("uram0"));
    EXPECT_CALL(*mockDataDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-0"));
    EXPECT_CALL(*mockDataDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-1"));
    EXPECT_CALL(*mockDataDev3.get(), GetName).WillRepeatedly(Return("unvme-ns-2"));
    EXPECT_CALL(*mockDataDev4.get(), GetName).WillRepeatedly(Return("unvme-ns-3"));
    EXPECT_CALL(*mockDataDev5.get(), GetName).WillRepeatedly(Return("unvme-ns-4"));
    EXPECT_CALL(*mockDataDev6.get(), GetName).WillRepeatedly(Return("unvme-ns-5"));
    EXPECT_CALL(*mockDataDev7.get(), GetName).WillRepeatedly(Return("unvme-ns-6"));
    EXPECT_CALL(*mockDataDev8.get(), GetName).WillRepeatedly(Return("unvme-ns-7"));
    EXPECT_CALL(*mockSpareDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-8"));
    EXPECT_CALL(*mockSpareDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-9"));

    uint64_t homoSize = (uint64_t)100 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    EXPECT_CALL(*mockDataDev1.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev2.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev3.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev4.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev5.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev6.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev7.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev8.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockSpareDev1.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockSpareDev2.get(), GetSize).WillRepeatedly(Return(homoSize));

    int numaId = 1;
    int anotherNumaId = 0;
    EXPECT_CALL(*mockWriteBufferDev.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev1.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev2.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev3.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev4.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev5.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));
    EXPECT_CALL(*mockDataDev6.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));
    EXPECT_CALL(*mockDataDev7.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));
    EXPECT_CALL(*mockDataDev8.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));
    EXPECT_CALL(*mockSpareDev1.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockSpareDev2.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));

    DeviceClass system = DeviceClass::SYSTEM;
    EXPECT_CALL(*mockWriteBufferDev.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev2.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev3.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev4.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev5.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev6.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev7.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev8.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev2.get(), GetClass).WillRepeatedly(Return(system));

    std::vector<shared_ptr<UBlockDevice>> devs;
    devs.push_back(mockDataDev1);
    devs.push_back(mockDataDev2);
    devs.push_back(mockDataDev3);
    devs.push_back(mockDataDev4);
    devs.push_back(mockDataDev5);
    devs.push_back(mockDataDev6);
    devs.push_back(mockDataDev7);
    devs.push_back(mockDataDev8);
    devs.push_back(mockSpareDev1);
    devs.push_back(mockSpareDev2);

    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockWriteBufferDev));
    EXPECT_CALL(mockDevMgr, GetDevs).WillRepeatedly(Return(devs));

    // When
    NumaAwaredArrayCreation naac({"uram0"}, 3, 1, &mockDevMgr);
    NumaAwaredArrayCreationResult result = naac.GetResult();
    int actual = result.code;

    // Then
    ASSERT_EQ(EID(SUCCESS), actual);
    ASSERT_EQ(1, result.options.size());
}

TEST(NumaAwaredArrayCreation, NumaAwaredArrayCreation_testIfNotEnoughNumaDevices)
{
    // Given
    MockDeviceManager mockDevMgr(nullptr);

    shared_ptr<MockUBlockDevice> mockWriteBufferDev = make_shared<MockUBlockDevice>("uram0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev1 = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev2 = make_shared<MockUBlockDevice>("unvme-ns-1", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev3 = make_shared<MockUBlockDevice>("unvme-ns-2", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev1 = make_shared<MockUBlockDevice>("unvme-ns-3", 0, nullptr);

    EXPECT_CALL(*mockWriteBufferDev.get(), GetName).WillRepeatedly(Return("uram0"));
    EXPECT_CALL(*mockDataDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-0"));
    EXPECT_CALL(*mockDataDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-1"));
    EXPECT_CALL(*mockDataDev3.get(), GetName).WillRepeatedly(Return("unvme-ns-2"));
    EXPECT_CALL(*mockSpareDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-3"));

    uint64_t homoSize = (uint64_t)100 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    EXPECT_CALL(*mockDataDev1.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev2.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev3.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockSpareDev1.get(), GetSize).WillRepeatedly(Return(homoSize));

    int numaId = 1;
    int anotherNumaId = 0;
    EXPECT_CALL(*mockDataDev1.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev2.get(), GetNuma).WillRepeatedly(Return(numaId));
    EXPECT_CALL(*mockDataDev3.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));
    EXPECT_CALL(*mockSpareDev1.get(), GetNuma).WillRepeatedly(Return(anotherNumaId));

    DeviceClass system = DeviceClass::SYSTEM;
    EXPECT_CALL(*mockDataDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev2.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev3.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev1.get(), GetClass).WillRepeatedly(Return(system));

    std::vector<shared_ptr<UBlockDevice>> devs;
    devs.push_back(mockDataDev1);
    devs.push_back(mockDataDev2);
    devs.push_back(mockDataDev3);
    devs.push_back(mockSpareDev1);

    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockWriteBufferDev));
    EXPECT_CALL(mockDevMgr, GetDevs).WillRepeatedly(Return(devs));

    // When
    NumaAwaredArrayCreation naac({"uram0"}, 3, 1, &mockDevMgr);
    NumaAwaredArrayCreationResult result = naac.GetResult();
    int actual = result.code;

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_INSUFFICIENT_NUMA_DEVS), actual);
}

TEST(NumaAwaredArrayCreation, NumaAwaredArrayCreation_testIfNotEnoughHomoSizeDevices)
{
    // Given
    MockDeviceManager mockDevMgr(nullptr);

    shared_ptr<MockUBlockDevice> mockWriteBufferDev = make_shared<MockUBlockDevice>("uram0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev1 = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev2 = make_shared<MockUBlockDevice>("unvme-ns-1", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev3 = make_shared<MockUBlockDevice>("unvme-ns-2", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev1 = make_shared<MockUBlockDevice>("unvme-ns-3", 0, nullptr);

    EXPECT_CALL(*mockWriteBufferDev.get(), GetName).WillRepeatedly(Return("uram0"));
    EXPECT_CALL(*mockDataDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-0"));
    EXPECT_CALL(*mockDataDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-1"));
    EXPECT_CALL(*mockDataDev3.get(), GetName).WillRepeatedly(Return("unvme-ns-2"));
    EXPECT_CALL(*mockSpareDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-3"));

    uint64_t size = (uint64_t)100 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    uint64_t anotherSize = (uint64_t)200 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    EXPECT_CALL(*mockDataDev1.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockDataDev2.get(), GetSize).WillRepeatedly(Return(size));
    EXPECT_CALL(*mockDataDev3.get(), GetSize).WillRepeatedly(Return(anotherSize));
    EXPECT_CALL(*mockSpareDev1.get(), GetSize).WillRepeatedly(Return(anotherSize));

    int homoNumaId = 1;
    EXPECT_CALL(*mockDataDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev2.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev3.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockSpareDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));

    DeviceClass system = DeviceClass::SYSTEM;
    EXPECT_CALL(*mockDataDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev2.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev3.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockSpareDev1.get(), GetClass).WillRepeatedly(Return(system));

    std::vector<shared_ptr<UBlockDevice>> devs;
    devs.push_back(mockDataDev1);
    devs.push_back(mockDataDev2);
    devs.push_back(mockDataDev3);
    devs.push_back(mockSpareDev1);

    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockWriteBufferDev));
    EXPECT_CALL(mockDevMgr, GetDevs).WillRepeatedly(Return(devs));

    // When
    NumaAwaredArrayCreation naac({"uram0"}, 3, 1, &mockDevMgr);
    NumaAwaredArrayCreationResult result = naac.GetResult();
    int actual = result.code;

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_INSUFFICIENT_NUMA_DEVS), actual);
}

TEST(NumaAwaredArrayCreation, NumaAwaredArrayCreation_testIfNotEnoughSystemDevices)
{
    // Given
    MockDeviceManager mockDevMgr(nullptr);

    shared_ptr<MockUBlockDevice> mockWriteBufferDev = make_shared<MockUBlockDevice>("uram0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev1 = make_shared<MockUBlockDevice>("unvme-ns-0", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev2 = make_shared<MockUBlockDevice>("unvme-ns-1", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockDataDev3 = make_shared<MockUBlockDevice>("unvme-ns-2", 0, nullptr);
    shared_ptr<MockUBlockDevice> mockSpareDev1 = make_shared<MockUBlockDevice>("unvme-ns-3", 0, nullptr);

    EXPECT_CALL(*mockWriteBufferDev.get(), GetName).WillRepeatedly(Return("uram0"));
    EXPECT_CALL(*mockDataDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-0"));
    EXPECT_CALL(*mockDataDev2.get(), GetName).WillRepeatedly(Return("unvme-ns-1"));
    EXPECT_CALL(*mockDataDev3.get(), GetName).WillRepeatedly(Return("unvme-ns-2"));
    EXPECT_CALL(*mockSpareDev1.get(), GetName).WillRepeatedly(Return("unvme-ns-3"));

    uint64_t homoSize = (uint64_t)100 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024;
    EXPECT_CALL(*mockDataDev1.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev2.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockDataDev3.get(), GetSize).WillRepeatedly(Return(homoSize));
    EXPECT_CALL(*mockSpareDev1.get(), GetSize).WillRepeatedly(Return(homoSize));

    int homoNumaId = 1;
    EXPECT_CALL(*mockDataDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev2.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockDataDev3.get(), GetNuma).WillRepeatedly(Return(homoNumaId));
    EXPECT_CALL(*mockSpareDev1.get(), GetNuma).WillRepeatedly(Return(homoNumaId));

    DeviceClass system = DeviceClass::SYSTEM;
    DeviceClass array = DeviceClass::ARRAY;
    EXPECT_CALL(*mockDataDev1.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev2.get(), GetClass).WillRepeatedly(Return(system));
    EXPECT_CALL(*mockDataDev3.get(), GetClass).WillRepeatedly(Return(array));
    EXPECT_CALL(*mockSpareDev1.get(), GetClass).WillRepeatedly(Return(system));

    std::vector<shared_ptr<UBlockDevice>> devs;
    devs.push_back(mockDataDev1);
    devs.push_back(mockDataDev2);
    devs.push_back(mockDataDev3);
    devs.push_back(mockSpareDev1);

    EXPECT_CALL(mockDevMgr, GetDev).WillRepeatedly(Return(mockWriteBufferDev));
    EXPECT_CALL(mockDevMgr, GetDevs).WillRepeatedly(Return(devs));

    // When
    NumaAwaredArrayCreation naac({"uram0"}, 3, 1, &mockDevMgr);
    NumaAwaredArrayCreationResult result = naac.GetResult();
    int actual = result.code;

    // Then
    ASSERT_EQ(EID(CREATE_ARRAY_INSUFFICIENT_NUMA_DEVS), actual);
}
}  // namespace pos
