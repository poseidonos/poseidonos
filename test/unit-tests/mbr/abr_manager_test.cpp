#include "src/mbr/abr_manager.h"

#include <gtest/gtest.h>

#include "test/unit-tests/mbr/mbr_manager_mock.h"
using ::testing::_;
using ::testing::NiceMock;
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
TEST(AbrManager, AbrManager_testIfAbrManagerCreatedSuccessfully)
{
    // Given : Nothing

    // When : Create abrMgr, there is one abr manager for one array manager
    AbrManager* abrMgr = new AbrManager();

    // Then : Nothing
}

TEST(AbrManager, LoadAbr_testIfAbrDataLoadedCorrectly)
{
    // Given : In mbr, array named with mockArrayname exists
    string mockArrayName = "POSArray";
    string mockDeviceName = "unvme-ns-0";
    MockMbrManager* mockMbrManager = new MockMbrManager();
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _)).WillOnce([=](string targetArrayName, struct ArrayBootRecord** abr) {
        using AbrPtr = struct ArrayBootRecord*;
        AbrPtr newAbr = new struct ArrayBootRecord; // Done with SaveAbr in product code
        newAbr->totalDevNum = 4;
        newAbr->dataDevNum = 3;
        newAbr->spareDevNum = 1;
        CopyData(newAbr->devInfo[0].deviceUid, mockDeviceName, DEVICE_UID_SIZE);
        *abr = newAbr;
        return 0;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : LoadAbr
    ArrayMeta meta;
    abrMgr->LoadAbr(mockArrayName, meta);

    // Then : meta have array data
    EXPECT_EQ(mockDeviceName, meta.devs.data.at(0).uid);
    delete mockMbrManager;
}

TEST(AbrManager, SaveAbr_testAbrDataUpdateAndDataCheck)
{
    // Given : In mbr, no array exists
    string mockArrayName = "POSArray";
    using AbrPtr = struct ArrayBootRecord*;
    AbrPtr newAbr = new struct ArrayBootRecord;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _)).WillRepeatedly([&](string targetArrayName, struct ArrayBootRecord** abr) {
        *abr = newAbr;
        return 0;
    });

    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    // When : save abr with updated information
    ArrayMeta meta = buildArrayMeta(mockArrayName, 3, 1); // array have own arraymeta in product code
    string builtDevName0 = mockArrayName + "_data_dev0";
    EXPECT_EQ(builtDevName0, meta.devs.data.at(0).uid);
    abrMgr->SaveAbr(mockArrayName, meta);
    // Then : abr data updated
    ArrayMeta newMeta;
    abrMgr->LoadAbr(mockArrayName, newMeta);
    EXPECT_EQ(builtDevName0, newMeta.devs.data.at(0).uid);
    delete mockMbrManager;
}

TEST(AbrManager, GetMfsInit_testGettingMfsInitZero)
{
    // Given : In mbr, array named with mockArrayname exists
    string mockArrayName = "POSArray";
    int expectedMfsInitVal = 0;
    MockMbrManager* mockMbrManager = new MockMbrManager();
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _)).WillOnce([=](string targetArrayName, struct ArrayBootRecord** abr) {
        using AbrPtr = struct ArrayBootRecord*;
        AbrPtr newAbr = new struct ArrayBootRecord; // Done with SaveAbr in product code
        newAbr->mfsInit = expectedMfsInitVal;
        *abr = newAbr;
        return 0;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : GetMfsInit
    bool mfsinitVal = abrMgr->GetMfsInit(mockArrayName);
    // Then : The value is same as expected
    EXPECT_EQ(expectedMfsInitVal, mfsinitVal);
    delete mockMbrManager;
}

TEST(AbrManager, GetMfsInit_testGettingMfsInitOne)
{
    // Given : In mbr, array named with mockArrayname exists
    string mockArrayName = "POSArray";
    int expectedMfsInitVal = 1;
    MockMbrManager* mockMbrManager = new MockMbrManager();
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _)).WillOnce([=](string targetArrayName, struct ArrayBootRecord** abr) {
        using AbrPtr = struct ArrayBootRecord*;
        AbrPtr newAbr = new struct ArrayBootRecord; // Done with SaveAbr in product code
        newAbr->mfsInit = expectedMfsInitVal;
        *abr = newAbr;
        return 0;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : GetMfsInit
    bool mfsinitVal = abrMgr->GetMfsInit(mockArrayName);
    // Then : The value is same as expected
    EXPECT_EQ(expectedMfsInitVal, mfsinitVal);
    delete mockMbrManager;
}

TEST(AbrManager, SetMfsInit_testIfMfsInitValSetToZero)
{
    // Given : In mbr, no array exists
    string mockArrayName = "POSArray";
    int expectedMfsInitVal = 0;
    using AbrPtr = struct ArrayBootRecord*;
    AbrPtr newAbr = new struct ArrayBootRecord;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _)).WillRepeatedly([&](string targetArrayName, struct ArrayBootRecord** abr) {
        *abr = newAbr;
        return 0;
    });

    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    // When : set mfsinit value
    abrMgr->SetMfsInit(mockArrayName, expectedMfsInitVal);
    // Then : check mfsinit value
    bool mfsinitVal = abrMgr->GetMfsInit(mockArrayName);
    EXPECT_EQ(expectedMfsInitVal, mfsinitVal);
    delete mockMbrManager;
}

TEST(AbrManager, SetMfsInit_testIfMfsInitValSetToOne)
{
    // Given : In mbr, no array exists
    string mockArrayName = "POSArray";
    int expectedMfsInitVal = 1;
    using AbrPtr = struct ArrayBootRecord*;
    AbrPtr newAbr = new struct ArrayBootRecord;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    EXPECT_CALL(*mockMbrManager, GetAbr(_, _)).WillRepeatedly([&](string targetArrayName, struct ArrayBootRecord** abr) {
        *abr = newAbr;
        return 0;
    });

    AbrManager* abrMgr = new AbrManager(mockMbrManager);

    // When : set mfsinit value
    abrMgr->SetMfsInit(mockArrayName, expectedMfsInitVal);
    // Then : check mfsinit value
    bool mfsinitVal = abrMgr->GetMfsInit(mockArrayName);
    EXPECT_EQ(expectedMfsInitVal, mfsinitVal);
    delete mockMbrManager;
}

TEST(AbrManager, CreateAbr_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : Call CreateAbr
    abrMgr->CreateAbr(mockArrayName, arrayMeta);
    // Then : Nothing
}

TEST(AbrManager, DeleteAbr_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : Call DeleteAbr
    abrMgr->DeleteAbr(mockArrayName, arrayMeta);
    // Then : Nothing
}

TEST(AbrManager, ResetMbr_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : Call ResetMbr
    abrMgr->ResetMbr();
    // Then : Nothing
}

TEST(AbrManager, GetAbrList_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    ArrayMeta arrayMeta;
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : Call GetAbrList
    vector<ArrayBootRecord> abrList;
    abrMgr->GetAbrList(abrList);
    // Then : Nothing
}

TEST(AbrManager, FindArray_testCommandPassing)
{
    // Given : AbrManger
    string mockArrayName = "POSArray";
    string devName = "unvme-ns-0";
    NiceMock<MockMbrManager>* mockMbrManager = new NiceMock<MockMbrManager>();
    EXPECT_CALL(*mockMbrManager, FindArray(_)).WillOnce([=](string devName) {
        return mockArrayName;
    });
    AbrManager* abrMgr = new AbrManager(mockMbrManager);
    // When : Call FindArray
    string result = abrMgr->FindArray(devName);
    // Then : Compare Result
    EXPECT_EQ(mockArrayName, result);
}

} // namespace pos
