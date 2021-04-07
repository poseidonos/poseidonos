#include <iostream>
#include "mbr_manager.h"
#include "src/device/device_manager.h"
#include "src/array/meta/array_meta_manager.h"
#include <gtest/gtest.h>

using namespace ibofos;
using namespace std;

namespace
{

class ArrayMetaTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ArrayMetaTest, LoadTest)
{
    ArrayMetaManager *arrayMetaManager = new ArrayMetaManager();
//    arrayMetaManager->Init();

    DeviceSet<std::string> devices;
    uint64_t nvmUid = 1234567890123456780;
    uint64_t dataUid = 3876543210987654310;
    uint64_t spareUid = 1234567890987654320;

    devices.nvm = to_string(nvmUid + 1);

    devices.data.clear();
    for (uint64_t index = 0; index < 8; index++)
    {
        devices.data.push_back(to_string(dataUid + index));
    }

    for (uint64_t index = 0; index < 2; index++)
    {
        devices.spares.push_back(to_string(spareUid + index));
    }
    
    ArrayMeta arrayMeta;
    arrayMeta.devices = devices;

    arrayMetaManager->Store(&arrayMeta);

    ArrayMeta arrayMetaTest = *arrayMetaManager->Load();

    EXPECT_TRUE(arrayMeta.devices.nvm == arrayMetaTest.devices.nvm);
    
    std::vector<string>::iterator dataIt = arrayMeta.devices.data.begin();
    std::vector<string>::iterator spareIt = arrayMeta.devices.spares.begin();
    
    std::vector<string>::iterator dataItTest = arrayMetaTest.devices.data.begin();
    std::vector<string>::iterator spareItTest = arrayMetaTest.devices.spares.begin();

    for (int index = 0; index < 8; index++, dataItTest++, dataIt++)
    {
        EXPECT_TRUE(*dataItTest == *dataIt);
    }
    
    for (int index = 0; index < 2; index++, spareItTest++, spareIt++)
    {
        EXPECT_TRUE(*spareItTest == *spareIt);
    }
}


TEST_F(ArrayMetaTest, WriteTest)
{
#if 0
    MbrDataList mbrDataList;
    MbrData mbrData;

    mbrData = { .key = "hello", .value = "world"};
    mbrDataList.push_back(mbrData);
    mbrData = { .key = "good", .value = "bye6"};
    mbrDataList.push_back(mbrData);
    mbrData = { .key = "good1", .value = "bye5"};
    mbrDataList.push_back(mbrData);
    mbrData = { .key = "good2", .value = "bye4"};
    mbrDataList.push_back(mbrData);
    mbrData = { .key = "good3", .value = "bye3"};
    mbrDataList.push_back(mbrData);
    mbrData = { .key = "good4", .value = "bye2"};
    mbrDataList.push_back(mbrData);
    //MbrManager& mbrMgr = *MbrManager::Instance();
    MbrManager& mbrMgr = new MbrManager();
    mbrMgr.Write(mbrDataList);

    mbrMgr.Read();

    string ret;
    ret = mbrMgr.GetValue("hello");
    EXPECT_TRUE(ret == "world");

    ret = mbrMgr.GetValue("good");
    EXPECT_TRUE(ret == "bye6");
    
    ret = mbrMgr.GetValue("good1");
    EXPECT_TRUE(ret == "bye5");
    
    ret = mbrMgr.GetValue("good2");
    EXPECT_TRUE(ret == "bye4");
    
    ret = mbrMgr.GetValue("good3");
    EXPECT_TRUE(ret == "bye3");
    
    ret = mbrMgr.GetValue("good4");
    EXPECT_TRUE(ret == "bye2");
    
    MbrDataList mbrDataList1;

    mbrData = { .key = "hello999", .value = "world111"};
    mbrDataList1.push_back(mbrData);
    mbrData = { .key = "good9", .value = "bye"};
    mbrDataList1.push_back(mbrData);
    mbrData = { .key = "good8", .value = "bye1"};
    mbrDataList1.push_back(mbrData);
    mbrData = { .key = "good7", .value = "bye2"};
    mbrDataList1.push_back(mbrData);
    mbrData = { .key = "good6", .value = "bye3"};
    mbrDataList1.push_back(mbrData);
    mbrData = { .key = "good5", .value = "bye4"};
    mbrDataList1.push_back(mbrData);

    

    mbrMgr.Write(mbrDataList1);
   
    ret = mbrMgr.GetValue("hello");
    EXPECT_FALSE(ret == "world");

    ret = mbrMgr.GetValue("good");
    EXPECT_FALSE(ret == "bye6");
    
    ret = mbrMgr.GetValue("good1");
    EXPECT_FALSE(ret == "bye5");
    
    ret = mbrMgr.GetValue("good2");
    EXPECT_FALSE(ret == "bye4");
    
    ret = mbrMgr.GetValue("good3");
    EXPECT_FALSE(ret == "bye3");
    
    ret = mbrMgr.GetValue("good4");
    EXPECT_FALSE(ret == "bye2");
 
    ret = mbrMgr.GetValue("hello999");
    EXPECT_TRUE(ret == "world111");

    ret = mbrMgr.GetValue("good9");
    EXPECT_TRUE(ret == "bye");
    
    ret = mbrMgr.GetValue("good8");
    EXPECT_TRUE(ret == "bye1");
    
    ret = mbrMgr.GetValue("good7");
    EXPECT_TRUE(ret == "bye2");
    
    ret = mbrMgr.GetValue("good6");
    EXPECT_TRUE(ret == "bye3");
    
    ret = mbrMgr.GetValue("good5");
    EXPECT_TRUE(ret == "bye4");

    mbrData = { .key = "hello999", .value = "world2323"};
    mbrMgr.Write(mbrData);

    ret = mbrMgr.GetValue("hello999");
    EXPECT_TRUE(ret == "world2323");
    EXPECT_FALSE(ret == "world111");

    mbrData = { .key = "newkey_hello", .value = "newvalue_world"};
    mbrMgr.Write(mbrData);

    ret = mbrMgr.GetValue("newkey_hello");
    EXPECT_TRUE(ret == "newvalue_world");

    ret = mbrMgr.GetValue("good5");
    EXPECT_TRUE(ret == "bye4");

    ret = mbrMgr.GetValue("hello999");
    EXPECT_TRUE(ret == "world2323");
    EXPECT_FALSE(ret == "world111");
#endif
}



}

int MakePreCondition(void)
{
    // device manager scan mock devs

    // set data mock devs
    string deviceName = "mock";
    uint32_t testDevsNum = 4;
    vector<std::string> dataDevs;
    
    DeviceManager& devMgr = *DeviceManager::Instance();
    devMgr.ScanDevs();
    
    //MbrManager& mbrMgr = *MbrManager::Instance();

    for (uint32_t index = 0; index < testDevsNum; index++)
    {
        dataDevs.push_back(deviceName + to_string(index));
    }

    //mbrMgr.SetDataDevs(dataDevs);

    return 0;

}

int main(int argc, char **argv)
{
    printf("running main ");

    MakePreCondition();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

