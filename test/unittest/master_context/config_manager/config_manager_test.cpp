#include <iostream>
#include <fstream>
#include "config_manager.h"
#include "src/include/pos_event_id.h"
#include <gtest/gtest.h>

using namespace pos;
using namespace std;


namespace
{

class ConfigTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        ConfigManagerSingleton::ResetInstance();
    }
};

TEST_F(ConfigTest, ReadTest)
{
    string ip;
    int ioUnitSize;
    int ret;
    
    system("cp ./default.conf /etc/pos/pos.conf");

    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    ret = configMgr.ReadFile();
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_FILE_READ_DONE == ret);

    ret = configMgr.GetValue("nvmfSubsystem", "ip", 
                             (void*)&ip, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_GET_VALUE_DONE == ret);

    ret = configMgr.GetValue("nvmfTransport", "ioUnitSize", 
                             (void*)&ioUnitSize, CONFIG_TYPE_INT);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_GET_VALUE_DONE == ret);

    EXPECT_TRUE(ip == "172.16.1.1");
    EXPECT_TRUE(ioUnitSize == 131072);

}

TEST_F(ConfigTest, ValidateTrueTest)
{
    string ip;
    int ioUnitSize;
    int ret;

    system("cp ./default.conf /etc/pos/pos.conf");

    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    
    ret = configMgr.GetValue("nvmfSubsystem", "ip", 
                             (void*)&ip, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_GET_VALUE_DONE == ret);
    
    ret = configMgr.GetValue("nvmfTransport", "ioUnitSize", 
                             (void*)&ioUnitSize, CONFIG_TYPE_INT);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_GET_VALUE_DONE == ret);
}

TEST_F(ConfigTest, ValidateFalseTest)
{
    string ip = "false_value";
    int ioUnitSize = 0xFA;
    int ret;

    system("cp ./default.conf /etc/pos/pos.conf");

    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_VALIDATE_VALUE_FAIL == ret);
}

TEST_F(ConfigTest, FormatErrorTest)
{
    system("cp ./format_fault.conf /etc/pos/pos.conf");
    
    string ip;
    int ret;
    
    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    ret = configMgr.ReadFile();
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_FILE_FORMAT_ERROR == ret);

    ConfigManagerSingleton::ResetInstance();
    ConfigManager& configMgr1 = *ConfigManagerSingleton::Instance();
    
    ret = configMgr1.GetValue("nvmfSubsystem", "ip", 
                              (void*)&ip, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_FILE_FORMAT_ERROR == ret);

}

TEST_F(ConfigTest, FileNotExistErrorTest)
{
    system("rm /etc/pos/ibofos.conf");
    
    string ip;
    int ret;
    
    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    ret = configMgr.ReadFile();
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_FILE_OPEN_FAIL == ret);

    ConfigManagerSingleton::ResetInstance();
    ConfigManager& configMgr1 = *ConfigManagerSingleton::Instance();
    
    ret = configMgr1.GetValue("nvmfSubsystem", "ip", 
                              (void*)&ip, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_FILE_OPEN_FAIL == ret);
}

TEST_F(ConfigTest, ValueTypeFaultTest)
{
    string ip;
    int port;
    bool isInit;
    int ret;

    system("cp ./default.conf /etc/pos/pos.conf");

    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    
    ret = configMgr.GetValue("nvmfSubsystem", "ip", 
                             (void*)&ip, CONFIG_TYPE_INT);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR == ret);
    
    ret = configMgr.GetValue("nvmfSubsystem", "port", 
                             (void*)&port, CONFIG_TYPE_BOOL);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR == ret);

    ret = configMgr.GetValue("nvmfSubsystem", "init", 
                             (void*)&isInit, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR == ret);
}

TEST_F(ConfigTest, ModuleNameFaultTest)
{
    string ip;
    int port;
    bool isInit;
    int ret;

    system("cp ./default.conf /etc/pos/pos.conf");

    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    
    ret = configMgr.GetValue("faultModule", "ip", 
                             (void*)&ip, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_REQUEST_MODULE_ERROR == ret);
    
    ret = configMgr.GetValue("faultModule", "port", 
                             (void*)&port, CONFIG_TYPE_INT);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_REQUEST_MODULE_ERROR == ret);

    ret = configMgr.GetValue("faultModule", "init", 
                             (void*)&isInit, CONFIG_TYPE_BOOL);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_REQUEST_MODULE_ERROR == ret);
}

TEST_F(ConfigTest, KeyNameFaultTest)
{
    string ip;
    int port;
    bool isInit;
    int ret;

    system("cp ./default.conf /etc/pos/pos.conf");

    ConfigManager& configMgr = *ConfigManagerSingleton::Instance();
    
    ret = configMgr.GetValue("nvmfSubsystem", "faultKey", 
                             (void*)&ip, CONFIG_TYPE_STRING);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_REQUEST_KEY_ERROR == ret);
    
    ret = configMgr.GetValue("nvmfTransport", "faultKey1", 
                             (void*)&port, CONFIG_TYPE_INT);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_REQUEST_KEY_ERROR == ret);

    ret = configMgr.GetValue("nvmfSubsystem", "faultKey2", 
                             (void*)&isInit, CONFIG_TYPE_BOOL);
    EXPECT_TRUE((int)POS_EVENT_ID::CONFIG_REQUEST_KEY_ERROR == ret);
}


}
int main(int argc, char **argv)
{
    printf("running main ");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

