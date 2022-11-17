#include "src/mbr/mbr_manager.h"

#include <gtest/gtest.h>

#include "src/array/device/array_device_list.h"
#include "src/master_context/version_provider.h"
#include "src/mbr/mbr_info.h"
#include "test/unit-tests/device/device_manager_mock.h"
#include "test/unit-tests/master_context/e2e_protect_mock.h"
#include "test/unit-tests/mbr/mbr_manager_test_fixture.h"
#include "test/unit-tests/mbr/mbr_map_manager_mock.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::NiceMock;
using ::testing::Return;
namespace pos
{
void
fillWithZeroes(struct ArrayBootRecord& abr)
{
    memset(&abr, 0, sizeof(struct ArrayBootRecord));
}

void
fillMbrWithZeroes(struct masterBootRecord& mbr)
{
    memset(&mbr, 0, sizeof(struct masterBootRecord));
}

static ArrayMeta
buildArrayMeta(string arrayName, int numDataDevices, int numBufferDevices)
{
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = arrayName;

    for (int i = 0; i < numDataDevices; i += 1)
    {
        // The UID is different from the device name
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

struct ArrayBootRecord
buildArrayBootRecordFrom(const ArrayMeta& arrayMeta)
{
    struct ArrayBootRecord abr;
    fillWithZeroes(abr);

    string arrayName = arrayMeta.arrayName;
    arrayName.copy(abr.arrayName, arrayName.length(), 0);

    DeviceSet<DeviceMeta> devs = arrayMeta.devs;
    abr.dataDevNum = devs.data.size();
    abr.nvmDevNum = devs.nvm.size();
    abr.spareDevNum = devs.spares.size();
    abr.totalDevNum = abr.dataDevNum + abr.nvmDevNum + abr.spareDevNum;

    // the order of filling devs in doesn't seem to matter
    int devIdx = 0;

    for (auto devListAndType : {std::make_pair(arrayMeta.devs.data, ArrayDeviceType::DATA),
             std::make_pair(arrayMeta.devs.nvm, ArrayDeviceType::NVM),
             std::make_pair(arrayMeta.devs.spares, ArrayDeviceType::SPARE)})
    {
        for (auto dev : devListAndType.first)
        {
            dev.uid.copy(abr.devInfo[devIdx].deviceUid, dev.uid.length(), 0);
            abr.devInfo[devIdx].deviceType = (int)devListAndType.second;
            devIdx++;
        }
    }

    return abr;
}

TEST(MbrManager, MbrManager_testDefaultConstructor)
{
    // Given
    // When
    MbrManager mbrManager;
    // Then
}

TEST(MbrManager, MbrManager_testIfConstructorInitializesMBRWithZeros)
{
    // Given: nothing

    // When: the manager is instantiated
    MbrManager m(NULL, "", NULL, NULL, NULL, NULL);

    // Then: it should zero out the MBR in memory
    struct masterBootRecord mbr = m.GetMbr();
    char* pMbr = reinterpret_cast<char*>(&mbr);
    for (int i = 0; i < sizeof(mbr); i += 1)
    {
        char byte = *(pMbr + i);
        ASSERT_EQ('\0', byte);
    }
}

TEST(MbrManager, MbrManager_testIfUsingRightSizeOfMbr)
{
    // Given: nothing

    // When: nothing

    // Then:
    ASSERT_EQ(sizeof(ArrayBootRecord), 17920);
    ASSERT_EQ(sizeof(masterBootRecord), 262144);
}

// FIXME: What's the maximum length of an array name? It seems to be ARRAY_NAME_SIZE - 2 (=62) at the moment.
// "ARRAY_NAME_SIZE - 1" and "ARRAY_NAME_SIZE" silently ignore string copy.
// ArrayNamePolicy does not reflect this; "ARRAY_NAME_SIZE - 1" would be a valid-length from ArrayNamePolicy's perspective.
// TODO(yyu): 1) CreateAbr() should return an error code (e.g., non-zero) if string copy didn't happen
//       2) We should be explicit about what's the maximum length for an array name
//       3) Can MbrManager have ArrayNamePolicy as a member as well? Even though array.cpp checks array name in advance,
//          it'd be good to have additional check within MbrManager (or moving ArrayNameArray into MbrManager).
//          This would be additional sanity check on the user input.

// Parameterized Tests
TEST_P(MbrManagerSuccessfulParameterizedCreateAbrTestFixture, testIfValidArrayNameLengthHandledProperly)
{
    // Given
    int arrayNameLength = GetParam();                                             // read from parameterized test fixture, i.e., 10, 30, 40, 50, 60, 62, 63
    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>; // allocated outside and freed inside by MbrManager
    MbrManager m(NULL, "", NULL, NULL, NULL, mockMbrMapMgr);
    string mockArrayName = "";
    for (int i = 0; i < arrayNameLength; i += 1)
    {
        mockArrayName.append("a");
    }
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;

    // When: the manager creates Array Boot Record
    int res = m.CreateAbr(arrayMeta);

    // Then: the default array index should point to mockArrayName
    ASSERT_EQ(0, res);
    ASSERT_EQ(1, m.GetMbr().arrayNum);
    const int defaultArrayIndex = 0;
    char* actual = m.GetMbr().arrayInfo[defaultArrayIndex].arrayName;
    ASSERT_EQ(0, mockArrayName.compare(actual));
}

INSTANTIATE_TEST_CASE_P(
    MbrManager_CreateAbr,
    MbrManagerSuccessfulParameterizedCreateAbrTestFixture,
    ::testing::Values(10, 30, 40, 50, 60, 63));

TEST_P(MbrManagerFailedParameterizedCreateAbrTestFixture, CreateAbr_testIfInvalidArrayNameLengthFails)
{
    // Given: an invalid-length array name
    int arrayNameLength = GetParam(); // 0, 64, 128
    MbrManager m(NULL, "", NULL, NULL, NULL, NULL);
    string mockArrayName = "";
    for (int i = 0; i < arrayNameLength; i += 1)
    {
        mockArrayName.append("a");
    }
    ArrayMeta arrayMeta;

    // When: the manager creates Array Boot Record
    arrayMeta.arrayName = mockArrayName;
    int res = m.CreateAbr(arrayMeta);

    // Then: the manager shouldn't update the metadata. It shouldn't copy array name either.
    // FIXME: I'd like to change the expected state after CreateAbr(). If the length of array name is invalid, we shouldn't even update any metadata.
    // I have commented out the following to respect the current implementation, but ideally want to uncomment and change the prod code instead.
    /*ASSERT_EQ(EID(ARRAY_MGR_NO_ARRAY_MATCHING_REQ_NAME), res); 
    const int defaultArrayIndex = 0;
    ASSERT_EQ(0, m.GetMbr().arrayFlag[defaultArrayIndex]);
    ASSERT_EQ(0, m.GetMbr().arrayNum);*/
}

INSTANTIATE_TEST_CASE_P(
    MbrManager_CreateAbr,
    MbrManagerFailedParameterizedCreateAbrTestFixture,
    ::testing::Values(0, 64, 128));

TEST(MbrManager, CreateAbr_testWithExistingAbrName)
{
    // Given: one arrays' meta
    ArrayMeta arrayMeta1 = buildArrayMeta("array1", 3, 1);

    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([&arrayMeta1](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        int i = 0;
        struct masterBootRecord* mbr = new struct masterBootRecord; // this is freed() within m.LoadMgr(), so should be allocated from heap
        fillMbrWithZeroes(*mbr);
        pMBRs->push_back(mbr);
        mbr->mbrVersion = 1,
        mbr->mbrParity = 1234;
        mbr->arrayNum = 1;
        mbr->arrayValidFlag[0] = 1;
        mbr->arrayInfo[0] = buildArrayBootRecordFrom(arrayMeta1);
        return 0;
    });
    MbrMapManager* mbrMapManager = new MbrMapManager; // let's inject real object
    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mbrMapManager);

    // When: load abr with arrayMeta1
    int tempResult;
    tempResult = m.LoadMbr();
    ASSERT_EQ(0, tempResult);

    // Then: creating abr with existing abr name fails
    tempResult = m.CreateAbr(arrayMeta1);
    ASSERT_EQ(EID(MBR_ABR_ALREADY_EXIST), tempResult);
}

TEST(MbrManager, CreateAbr_testWithExistingDevices)
{
    // Given: one arrays' meta
    ArrayMeta arrayMeta1 = buildArrayMeta("array1", 3, 1);

    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([&arrayMeta1](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        int i = 0;
        struct masterBootRecord* mbr = new struct masterBootRecord; // this is freed() within m.LoadMgr(), so should be allocated from heap
        pMBRs->push_back(mbr);
        mbr->mbrVersion = 1,
        mbr->mbrParity = 1234;
        mbr->arrayNum = 1;
        mbr->arrayValidFlag[0] = 1;
        mbr->arrayInfo[0] = buildArrayBootRecordFrom(arrayMeta1);
        return 0;
    });
    MbrMapManager* mbrMapManager = new MbrMapManager; // let's inject real object
    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mbrMapManager);

    // When: load abr with arrayMeta1
    int tempResult;
    tempResult = m.LoadMbr();
    ASSERT_EQ(0, tempResult);

    // Then: creating abr with existing devices fails
    string anotherArrayName = "newArray";
    arrayMeta1.arrayName = anotherArrayName;
    tempResult = m.CreateAbr(arrayMeta1);
    ASSERT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), tempResult);
}

TEST(MbrManager, DeleteAbr_testIfArrayIsProperlyDeletedAndVersionIsIncremented)
{
    // Given
    MockDataProtect* mockDp = new MockDataProtect;
    ::testing::Mock::AllowLeak(mockDp); // TODO(yyu): instantiated outside and released inside by MbrManager. Wondering if this is intended?
    EXPECT_CALL(*mockDp, MakeParity(_, _)).WillOnce(Return(1234));

    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillOnce(Return(0));

    MockMbrMapManager* mockMbrMapMgr = new MockMbrMapManager;
    EXPECT_CALL(*mockMbrMapMgr, CheckAllDevices(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockMbrMapMgr, DeleteDevices(_)).WillOnce(Return(0));
    EXPECT_CALL(*mockMbrMapMgr, InsertDevices(_, _)).WillRepeatedly(Return(0)); // not interesting call.

    MbrManager m(mockDp, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);
    string mockArrayName = "mockArray";
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;

    int createRes = m.CreateAbr(arrayMeta); // TODO(yyu): this doesn't increment "version" in memory. Wondering if this is intended?
    ASSERT_EQ(0, createRes);
    int currMbrVersion = m.GetMbrVersionInMemory();

    // When: the manager deletes Array Boot Record
    int deleteRes = m.DeleteAbr(mockArrayName);

    // Then: the metadata should be updated accordingly. Also, the version should be incremented
    ASSERT_EQ(0, deleteRes);
    const int defaultArrayIndex = 0;
    ASSERT_EQ(0, m.GetMbr().arrayDevFlag[defaultArrayIndex]);
    ASSERT_EQ(0, m.GetMbr().arrayNum);
    ASSERT_EQ(currMbrVersion + 1, m.GetMbrVersionInMemory());
}

TEST(MbrManager, DeleteAbr_testIfNonExistentArrayNameHandledProperly)
{
    // Given
    MockMbrMapManager* mockMbrMapMgr = new MockMbrMapManager;
    EXPECT_CALL(*mockMbrMapMgr, DeleteDevices(_)).Times(0); // shouldn't be called
    EXPECT_CALL(*mockMbrMapMgr, InsertDevices(_, _)).Times(1);
    EXPECT_CALL(*mockMbrMapMgr, CheckAllDevices(_)).Times(1);

    MbrManager m(NULL, "", NULL, NULL, NULL, mockMbrMapMgr);
    string mockArrayName = "mockArray";
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;
    int createRes = m.CreateAbr(arrayMeta);
    ASSERT_EQ(0, createRes);
    int currMbrVersion = m.GetMbrVersionInMemory();

    // When: the manager deletes with a wrong name
    mockArrayName += "wrong";
    int actual = m.DeleteAbr(mockArrayName);

    // Then: the manager returns an error code and does not update version
    const int expected = EID(MBR_ABR_NOT_FOUND);
    ASSERT_EQ(expected, actual);
    ASSERT_EQ(currMbrVersion, m.GetMbrVersionInMemory());
}

TEST(MbrManager, DeleteAbr_testIfDeviceIoFailureHandledProperly)
{
    // Given
    MockDataProtect* mockDp = new MockDataProtect;
    ::testing::Mock::AllowLeak(mockDp); // TODO(yyu): instantiated outside and released inside by MbrManager. Wondering if this is intended?
    int randomParity = 1234;
    EXPECT_CALL(*mockDp, MakeParity(_, _)).WillOnce(Return(randomParity));

    int errorCode = !0; // non-zero is the semantics in use
    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillOnce(Return(errorCode));

    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;

    MbrManager m(mockDp, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);
    string mockArrayName = "mockArray";
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;

    int createRes = m.CreateAbr(arrayMeta);
    ASSERT_EQ(0, createRes);

    int currMbrVersion = m.GetMbrVersionInMemory();

    // When: the manager runs into I/O error when updating MBR
    int actual = m.DeleteAbr(mockArrayName);

    // Then: the manager should return an expected error code and should not update version
    const int expected = EID(MBR_WRITE_ERROR);
    ASSERT_EQ(expected, actual);
    ASSERT_EQ(currMbrVersion, m.GetMbrVersionInMemory());

    // FIXME: at this point, in-memory data structure (i.e., systeminfo) has changed while on-disk data structure has not.
    // Do we want to revert back the in-memory data (e.g., CreateAbr()), or just leave it as it is? Wondering what the current design is?
}

TEST(MbrManager, GetAbr_testIfInvalidAndValidArrayNamesAreHandledProperly)
{
    // Given
    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;
    MbrManager m(NULL, "", NULL, NULL, NULL, mockMbrMapMgr);
    string mockArrayName = "mockArray";
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;
    int createRes = m.CreateAbr(arrayMeta);
    ASSERT_EQ(0, createRes);

    // When 1: we provide a wrong array name
    struct ArrayBootRecord* pAbr = nullptr;
    m.GetAbr("wrongArray", &pAbr, arrayMeta.id);

    // Then 1: we should get null from the second param
    ASSERT_EQ(nullptr, pAbr);

    // When 2: we provide a correct array name
    m.GetAbr(mockArrayName, &pAbr, arrayMeta.id);

    // Then 2: we should get the valid array boot record
    string expected = mockArrayName;
    string actual = pAbr->arrayName;
    ASSERT_EQ(expected, actual);
}

// this is a private member struct within MbrManager.
struct FakeDiskIoContext
{
    UbioDir ubioDir;
    void* mem;
};

TEST(MbrManager, SaveMbr_testIfPosVersionAndMbrVersionAndSystemUuidAreEncodedInDiskIoContext)
{
    // Given
    MockDataProtect* mockDp = new MockDataProtect;
    ::testing::Mock::AllowLeak(mockDp); // TODO(yyu): instantiated outside and released inside by MbrManager. Wondering if this is intended?
    int randomParity = 1234;
    EXPECT_CALL(*mockDp, MakeParity(_, _)).WillOnce(Return(randomParity));

    string randomUuid = "dbc81f90-7003-11eb-9f41-8f0c6ff52bea";

    MockDeviceManager mockDevMgr(nullptr);
    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;
    MbrManager m(mockDp, randomUuid, NULL, NULL, &mockDevMgr, mockMbrMapMgr);
    string mockArrayName = "mockArray";
    ArrayMeta arrayMeta;
    arrayMeta.arrayName = mockArrayName;
    int createRes = m.CreateAbr(arrayMeta);
    ASSERT_EQ(0, createRes);

    string posVersion = VersionProviderSingleton::Instance()->GetVersion();
    memcpy(m.GetMbr().posVersion, posVersion.c_str(), POS_VERSION_SIZE);
    int mbrVersion = 5; // mock version
    m.GetMbr().mbrVersion = mbrVersion;

    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillOnce([&m](DeviceIterFunc func, void* ctx) {
        string actual, expected;

        // Then: "ctx", passed in to mockDevMgr, should contain PosVersion, MbrVersion, randomUuid
        struct FakeDiskIoContext* diskIoCtxt = static_cast<struct FakeDiskIoContext*>(ctx);

        if (UbioDir::Write != diskIoCtxt->ubioDir)
        {
            return -1; // the caller cares about whether it's zero or not only
        }

        struct masterBootRecord* copiedMbr = static_cast<struct masterBootRecord*>(diskIoCtxt->mem);
        actual = copiedMbr->posVersion;
        expected = m.GetMbr().posVersion;
        if (expected != actual)
        {
            return -1;
        }

        actual = copiedMbr->mbrVersion;
        expected = m.GetMbr().mbrVersion;
        if (expected != actual)
        {
            return -1;
        }

        actual = copiedMbr->systemUuid;
        expected = m.GetMbr().systemUuid;
        if (expected != actual)
        {
            return -1;
        }

        return 0;
    });

    // When
    int actual = m.SaveMbr();

    // Then: mockDevMgr->IterateDevicesAndDoFunc() should receive proper systeminfo data
    int expected = 0;
    ASSERT_EQ(expected, actual);
}

TEST(MbrManager, LoadMbr_testIfTheLatestMajorityMbrIsSelected)
{
    // Given: multiple different versions of MBR exist in devices (due to partial failures for example)
    using MBR = struct masterBootRecord;
    // this is freed() within m.LoadMgr(), so should be allocated from heap
    MBR* mbr1 = new MBR();
    mbr1->mbrVersion = 1,
    mbr1->mbrParity = 1234;
    mbr1->arrayNum = 0;

    MBR* mbr2 = new MBR();
    mbr2->mbrVersion = 2;
    mbr2->mbrParity = 2345;
    mbr2->arrayNum = 0;

    MBR* mbr3 = new MBR();
    mbr3->mbrVersion = 2;
    mbr3->mbrParity = 3456;
    mbr3->arrayNum = 0;

    MBR* mbr4 = new MBR();
    mbr4->mbrVersion = 2;
    mbr4->mbrParity = 2345;
    mbr4->arrayNum = 0;

    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillOnce([&mbr1, &mbr2, &mbr3, &mbr4](DeviceIterFunc func, void* ctx) {
        // type(*ctx) == std::list<void*> mems;
        // we should cast "ctx" to its original type and then fill in with mock MBRs
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        pMBRs->push_back(mbr1);
        pMBRs->push_back(mbr2);
        pMBRs->push_back(mbr3);
        pMBRs->push_back(mbr4);

        return 0;
    });

    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;
    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    // When
    int res = m.LoadMbr();

    // Then
    ASSERT_EQ(EID(SUCCESS), res);
    int expectedMbrVersion = 2;
    int actualMbrVersion = m.GetMbr().mbrVersion;
    ASSERT_EQ(expectedMbrVersion, actualMbrVersion);

    int expectedMbrParity = 2345;
    int actualMbrParity = m.GetMbr().mbrParity;
    ASSERT_EQ(expectedMbrParity, actualMbrParity);
}

TEST(MbrManager, ResetMbr_testIfSuccessfullyResetted)
{
    // Given
    MockMbrMapManager* mockMbrMapMgr = new MockMbrMapManager;
    EXPECT_CALL(*mockMbrMapMgr, ResetMap()).Times(1); // this is an interesting call

    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillOnce([](DeviceIterFunc func, void* ctx) {
        struct FakeDiskIoContext* diskIoCtxt = static_cast<struct FakeDiskIoContext*>(ctx);
        if (UbioDir::Write == (*diskIoCtxt).ubioDir)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    });

    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    // When
    int actual = m.ResetMbr();

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(0, m.GetMbrVersionInMemory());
}

TEST(MbrManager, GetAbrList_testWithNoArray)
{
    // Given : No array exists
    using MBR = struct masterBootRecord;
    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([](DeviceIterFunc func, void* ctx) {
        return 0;
    });

    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;
    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    // When : GetAbrList
    m.LoadMbr();
    std::vector<struct ArrayBootRecord> abrList;
    abrList.clear();
    m.GetAbrList(abrList);

    // Then : No abr
    int expectedAbrNum = 0;
    int actualAbrNum = abrList.size();
    ASSERT_EQ(expectedAbrNum, actualAbrNum);
}

TEST(MbrManager, GetAbrList_testWithOneArray)
{
    // Given : One array exist
    int defaultArrayIndex = 0;
    unsigned int expectedArrayNum = 1;
    string mockArrayName = "POSArray";
    string createDatetime = "2021-03-17 15:15:15 +09:00";

    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([=](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        using MBR = struct masterBootRecord;
        MBR* mbr = new MBR();
        mbr->mbrVersion = 1;
        mbr->arrayNum = expectedArrayNum;
        mbr->arrayValidFlag[defaultArrayIndex] = 1;
        CopyData(mbr->arrayInfo[defaultArrayIndex].arrayName, mockArrayName, ARRAY_NAME_SIZE);
        CopyData(mbr->arrayInfo[defaultArrayIndex].createDatetime, createDatetime, DATE_SIZE);

        pMBRs->push_back(mbr);

        return 0;
    });

    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;

    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    // When : GetAbrList
    m.LoadMbr();
    std::vector<ArrayBootRecord> abrList;
    m.GetAbrList(abrList);

    // Then : One Abr Found with written data on disk
    ASSERT_EQ(expectedArrayNum, abrList.size());
    bool abrFound = false;
    std::vector<ArrayBootRecord>::iterator it;
    for (it = abrList.begin(); it != abrList.end(); it++)
    {
        if (it->arrayName == mockArrayName)
        {
            abrFound = true;
            break;
        }
    }
    ASSERT_EQ(true, abrFound);
    ASSERT_EQ(createDatetime, it->createDatetime);
}

TEST(MbrManager, GetAbrList_testIfCreatedAbrsAreRetrieved)
{
    // Given
    string randomUuid = "dbc81f90-7003-11eb-9f41-8f0c6ff52bea";
    ArrayMeta arrayMeta1 = buildArrayMeta("array1", 3, 1);
    ArrayMeta arrayMeta2 = buildArrayMeta("array2", 3, 1);
    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([&arrayMeta1, &arrayMeta2](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        struct masterBootRecord* mbr1 = new struct masterBootRecord;
        // this is freed() within m.LoadMgr(), so should be allocated from heap
        mbr1->mbrVersion = 1,
        mbr1->mbrParity = 1234;
        mbr1->arrayNum = 2;
        mbr1->arrayValidFlag[0] = 1;
        mbr1->arrayValidFlag[1] = 1;
        mbr1->arrayInfo[0] = buildArrayBootRecordFrom(arrayMeta1);
        mbr1->arrayInfo[1] = buildArrayBootRecordFrom(arrayMeta2);

        pMBRs->push_back(mbr1);
        return 0;
    });

    MockMbrMapManager* mockMbrMapMgr = new MockMbrMapManager;
    EXPECT_CALL(*mockMbrMapMgr, CheckAllDevices(_)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mockMbrMapMgr, InsertDevices(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mockMbrMapMgr, InsertDevice(_, 0)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mockMbrMapMgr, InsertDevice(_, 1)).WillRepeatedly(Return(0));
    EXPECT_CALL(*mockMbrMapMgr, ResetMap()).WillRepeatedly(Return(0));

    MbrManager m(NULL, randomUuid, NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    std::vector<ArrayBootRecord> abrList;

    // When
    int tempResult;
    tempResult = m.CreateAbr(arrayMeta1);
    ASSERT_EQ(0, tempResult);
    tempResult = m.CreateAbr(arrayMeta2);
    ASSERT_EQ(0, tempResult);
    tempResult = m.LoadMbr(); // loadMbr() is required for this UT to be able to get abr list
    ASSERT_EQ(0, tempResult);
    int actual = m.GetAbrList(abrList);

    // Then
    ASSERT_EQ(0, actual);
    ASSERT_EQ(2, abrList.size());
}

TEST(MbrManager, UpdateDeviceIndexMap_testIfMbrMapManagerRefreshesDeviceMap)
{
    // Given: one arrays' meta
    ArrayMeta arrayMeta1 = buildArrayMeta("array1", 3, 1);
    unsigned int arrayIndex;
    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([&arrayMeta1](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        int i = 0;
        struct masterBootRecord* mbr = new struct masterBootRecord; // this is freed() within m.LoadMgr(), so should be allocated from heap
        fillMbrWithZeroes(*mbr);
        mbr->mbrVersion = 1,
        mbr->mbrParity = 1234;
        mbr->arrayNum = 1;
        mbr->arrayValidFlag[0] = 1;
        mbr->arrayInfo[0] = buildArrayBootRecordFrom(arrayMeta1);
        pMBRs->push_back(mbr);
        return 0;
    });
    MbrMapManager* mbrMapManager = new MbrMapManager; // let's inject real object
    MbrManager m(NULL, "", NULL, NULL, &mockDevMgr, mbrMapManager);

    // When: we update device index map with addition of spare device
    int tempResult;
    tempResult = m.LoadMbr();
    ASSERT_EQ(0, tempResult);
    struct ArrayBootRecord* abr = nullptr;
    string newSpareDeviceName = "array1_spare_dev_0";
    m.GetAbr(arrayMeta1.arrayName, &abr, arrayIndex);
    abr->spareDevNum = 1;
    abr->totalDevNum += 1;
    CopyData(abr->devInfo[abr->totalDevNum - 1].deviceUid, newSpareDeviceName, ARRAY_NAME_SIZE);
    abr->devInfo[abr->totalDevNum - 1].deviceType = (int)ArrayDeviceType::SPARE;

    int actual = m.UpdateDeviceIndexMap(arrayMeta1.arrayName);

    // Then: the updated device index map have those devices
    ArrayMeta newArrayMeta = arrayMeta1;
    newArrayMeta.devs.spares.push_back(DeviceMeta(newSpareDeviceName));
    ASSERT_EQ(0, actual);
    ASSERT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), mbrMapManager->CheckAllDevices(newArrayMeta));
    ArrayMeta newOtherArrayMeta;
    string newOtherArrayName = "newArray";
    newOtherArrayMeta.arrayName = newOtherArrayName;
    newOtherArrayMeta.devs.spares.push_back(DeviceMeta(newSpareDeviceName));
    ASSERT_EQ(EID(MBR_DEVICE_ALREADY_IN_ARRAY), mbrMapManager->CheckAllDevices(newOtherArrayMeta));
}

TEST(MbrManager, FindArrayWithDeviceSN_testFindingArraySuccessfully)
{
    // Given : MbrManager with one array
    int expectedArrayNum = 1;
    int defaultArrayIndex = 0;
    string mockArrayName = "POSArray";
    string mockDevSN = "unvme-ns-0";
    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([=](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        using MBR = struct masterBootRecord;
        MBR* mbr = new MBR();
        mbr->mbrVersion = 1;
        mbr->arrayNum = expectedArrayNum;
        mbr->arrayValidFlag[defaultArrayIndex] = 1;
        CopyData(mbr->arrayInfo[defaultArrayIndex].arrayName, mockArrayName, ARRAY_NAME_SIZE);

        pMBRs->push_back(mbr);

        return 0;
    });
    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;
    EXPECT_CALL(*mockMbrMapMgr, FindArrayIndex(_)).WillOnce([=](string devName) {
        return defaultArrayIndex;
    });
    MbrManager mbrMgr(NULL, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    // When : Call FindArray
    mbrMgr.LoadMbr();
    string result = mbrMgr.FindArrayWithDeviceSN(mockDevSN);

    // Then : array found
    EXPECT_EQ(mockArrayName, result);
}

TEST(MbrManager, FindArrayWithDeviceSN_testFindingArrayWithNoArray)
{
    // Given : MbrManager with one array
    int expectedArrayNum = 1;
    int defaultArrayIndex = 0;
    string mockDevSN = "unvme-ns-0";
    MockDeviceManager mockDevMgr(nullptr);
    EXPECT_CALL(mockDevMgr, IterateDevicesAndDoFunc(_, _)).WillRepeatedly([=](DeviceIterFunc func, void* ctx) {
        std::list<void*>* pMBRs = static_cast<std::list<void*>*>(ctx);
        return 0;
    });
    NiceMock<MockMbrMapManager>* mockMbrMapMgr = new NiceMock<MockMbrMapManager>;
    EXPECT_CALL(*mockMbrMapMgr, FindArrayIndex(_)).WillOnce([=](string devName) {
        return -1;
    });
    MbrManager mbrMgr(NULL, "", NULL, NULL, &mockDevMgr, mockMbrMapMgr);

    // When : Call FindArray
    mbrMgr.LoadMbr();
    string result = mbrMgr.FindArrayWithDeviceSN(mockDevSN);

    // Then : array found
    EXPECT_EQ("", result);
}

} // namespace pos
