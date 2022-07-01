#include "src/mbr/abr_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/mbr/mbr_manager_mock.h"
#include "test/unit-tests/mbr/mbr_map_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::Return;
namespace pos
{
ArrayMeta
buildArrayMeta(string arrayName, int numDataDevices, int numBufferDevices)
{
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = arrayName;

    for (int i = 0; i < numDataDevices; i += 1)
    {
        string uid = arrayName + "_data_dev" + std::to_string(i);
        DeviceMeta dm(uid);
        arrayMeta.devs.data.push_back(dm);
    }

    for (int i = 0; i < numBufferDevices; i += 1)
    {
        string uid = arrayName + "_buffer_dev" + std::to_string(i);
        DeviceMeta dm(uid);
        arrayMeta.devs.nvm.push_back(dm);
    }

    return arrayMeta;
}

TEST(AbrManager, AbrManager_testDefaultConstructor)
{
    // Given
    // When
    AbrManager abrManager;
    // Then
}

TEST(AbrManager, AbrManager_testIfAbrManagerCreatedSuccessfully)
{
    // Given : Nothing
    MockMbrManager mockMbrMgr(nullptr, "uuid", nullptr, nullptr, nullptr, nullptr);

    // When : Create abrMgr, there is one abr manager for one array manager
    AbrManager* abrMgr = new AbrManager(&mockMbrMgr);

    // Then : Nothing
}

TEST(AbrManager, LoadAbr_testIfAbrDataLoadedCorrectly)
{
    // Given : In mbr, array named with mockArrayname exists
    string mockArrayName = "POSArray";
    string mockNvmDeviceName = "unvme-nvm";
    string mockSpareDeviceName = "unvme-spare";
    string mockDeviceName = "unvme-ns-";
    MockMbrManager* mockMbrManager = new MockMbrManager(nullptr, "uuid", nullptr, nullptr, nullptr, nullptr);
    unsigned int arrayIndex;
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _, _)).WillOnce([=](string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex)
    {
        using AbrPtr = struct ArrayBootRecord*;
        AbrPtr newAbr = new struct ArrayBootRecord; // Done with SaveAbr in product code
        newAbr->totalDevNum = 5;
        newAbr->dataDevNum = 3;
        newAbr->spareDevNum = 1;
        newAbr->nvmDevNum = 1;
        CopyData(newAbr->devInfo[0].deviceUid, mockNvmDeviceName, DEVICE_UID_SIZE);
        CopyData(newAbr->devInfo[1].deviceUid, mockDeviceName + "0", DEVICE_UID_SIZE);
        CopyData(newAbr->devInfo[2].deviceUid, mockDeviceName + "1", DEVICE_UID_SIZE);
        CopyData(newAbr->devInfo[3].deviceUid, mockDeviceName + "2", DEVICE_UID_SIZE);
        CopyData(newAbr->devInfo[4].deviceUid, mockSpareDeviceName, DEVICE_UID_SIZE);

        *abr = newAbr;
        return 0;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : LoadAbr
    ArrayMeta meta;
    meta.arrayName = mockArrayName;
    abrMgr->LoadAbr(meta);

    // Then : meta have array data
    EXPECT_EQ(mockNvmDeviceName, meta.devs.nvm.at(0).uid);
    EXPECT_EQ(mockDeviceName + "0", meta.devs.data.at(0).uid);
    EXPECT_EQ(mockDeviceName + "1", meta.devs.data.at(1).uid);
    EXPECT_EQ(mockDeviceName + "2", meta.devs.data.at(2).uid);
    EXPECT_EQ(mockSpareDeviceName, meta.devs.spares.at(0).uid);
    delete mockMbrManager;
}

TEST(AbrManager, SaveAbr_testAbrDataUpdateAndDataCheck)
{
    // Given : In mbr, no array exists
    string mockArrayName = "POSArray";
    using AbrPtr = struct ArrayBootRecord*;
    AbrPtr newAbr = new struct ArrayBootRecord;
    MockMbrMapManager* mockMapManager = new MockMbrMapManager;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>(nullptr, "uuid", nullptr, nullptr, nullptr, mockMapManager);
    unsigned int arrayIndex;
    EXPECT_CALL(*mockMapManager, DeleteDevices).Times(1);
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _, _)).WillRepeatedly([&](string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex)
    {
        *abr = newAbr;
        return 0;
    });

    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    // When : save abr with updated information
    ArrayMeta meta = buildArrayMeta(mockArrayName, 3, 1); // array have own arraymeta in product code
    string builtDevName0 = mockArrayName + "_data_dev0";
    EXPECT_EQ(builtDevName0, meta.devs.data.at(0).uid);
    abrMgr->SaveAbr(meta);
    // Then : abr data updated
    ArrayMeta newMeta;
    newMeta.arrayName = mockArrayName;
    abrMgr->LoadAbr(newMeta);
    EXPECT_EQ(builtDevName0, newMeta.devs.data.at(0).uid);
    delete mockMbrManager;
}

TEST(AbrManager, CreateAbr_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;
    MockMbrMapManager mockMapManager;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>(nullptr, "uuid", nullptr, nullptr, nullptr, &mockMapManager);
    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    EXPECT_CALL(mockMapManager, CheckAllDevices).WillOnce(Return(0));
    EXPECT_CALL(mockMapManager, InsertDevices).Times(AtLeast(1));

    // When : Call CreateAbr
    abrMgr->CreateAbr(arrayMeta);
    // Then : Nothing
}

TEST(AbrManager, DeleteAbr_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>(nullptr, "uuid", nullptr, nullptr, nullptr, nullptr);
    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    // When : Call DeleteAbr
    abrMgr->DeleteAbr(mockArrayName);
    // Then : Nothing
}

TEST(AbrManager, ResetMbr_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    MockDeviceManager mockDevManager(nullptr);
    MockMbrMapManager mockMapManager;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>(nullptr, "uuid", nullptr, nullptr, &mockDevManager, &mockMapManager);
    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    EXPECT_CALL(mockMapManager, ResetMap).Times(1);
    EXPECT_CALL(mockDevManager, IterateDevicesAndDoFunc).WillOnce(Return(0));

    // When : Call ResetMbr
    abrMgr->ResetMbr();
    // Then : Nothing
}

TEST(AbrManager, GetAbrList_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    MockDeviceManager mockDevManager(nullptr);
    MockMbrMapManager* mockMapManager = new MockMbrMapManager;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>(nullptr, "uuid", nullptr, nullptr, &mockDevManager, mockMapManager);
    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    EXPECT_CALL(mockDevManager, IterateDevicesAndDoFunc).WillOnce(Return(0));

    // When : Call GetAbrList
    vector<ArrayBootRecord> abrList;
    abrMgr->GetAbrList(abrList);
    // Then : Nothing
}

TEST(AbrManager, FindArrayWithDeviceSN_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    string devName = "unvme-ns-0";
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>(nullptr, "uuid", nullptr, nullptr, nullptr, nullptr);
    EXPECT_CALL(*mockMbrManager, FindArrayWithDeviceSN(_)).WillOnce([=](string devName)
    {
        return mockArrayName;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : Call FindArray
    string result = abrMgr->FindArrayWithDeviceSN(devName);
    // Then : Compare Result
    EXPECT_EQ(mockArrayName, result);
    delete mockMbrManager;
}

TEST(AbrManager, GetLastUpdatedDateTime_testIfLastUpdateDateTimeLoadedWell)
{
    // Given : In mbr, array named with mockArrayname exists
    string mockArrayName = "POSArray";
    string lastUpdated = "2021-01-01";

    MockMbrManager* mockMbrManager = new MockMbrManager(nullptr, "uuid", nullptr, nullptr, nullptr, nullptr);
    unsigned int arrayindex;
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _, _)).WillOnce([=](string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex)
    {
        using AbrPtr = struct ArrayBootRecord*;
        AbrPtr newAbr = new struct ArrayBootRecord;
        CopyData(newAbr->updateDatetime, lastUpdated.c_str(), DATE_SIZE);
        *abr = newAbr;
        return 0;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : GetLastUpdatedDateTime
    string lastUpdatedFromAbr = abrMgr->GetLastUpdatedDateTime(mockArrayName);
    // Then : The value is same as expected
    EXPECT_EQ(lastUpdated, lastUpdatedFromAbr);
    delete mockMbrManager;
}

TEST(AbrManager, GetCreatedDateTime_testIfCreatedDateTimeLoadedWell)
{
    // Given : In mbr, array named with mockArrayname exists
    string mockArrayName = "POSArray";
    string created = "2021-01-01";

    MockMbrManager* mockMbrManager = new MockMbrManager(nullptr, "uuid", nullptr, nullptr, nullptr, nullptr);
    unsigned int arrayindex;
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _, _)).WillOnce([=](string targetArrayName, struct ArrayBootRecord** abr, unsigned int& arrayIndex)
    {
        using AbrPtr = struct ArrayBootRecord*;
        AbrPtr newAbr = new struct ArrayBootRecord;
        CopyData(newAbr->createDatetime, created.c_str(), DATE_SIZE);
        *abr = newAbr;
        return 0;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : GetCreatedDateTime
    string createdFromAbr = abrMgr->GetCreatedDateTime(mockArrayName);
    // Then : The value is same as expected
    EXPECT_EQ(created, createdFromAbr);
    delete mockMbrManager;
}
} // namespace pos
