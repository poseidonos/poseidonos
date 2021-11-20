#include "src/master_context/config_manager.h"

#include <gtest/gtest.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdio>
#include <unistd.h>
#include "src/helper/json/json_helper.h"
#include "src/include/pos_event_id.h"


namespace pos
{
struct ConfigKeyValue
{
    std::string key;
    std::string value;
};
struct ConfigModuleData
{
    std::string moduleName;
    vector<ConfigKeyValue> keyAndValueList;
};
vector<ConfigKeyValue> testData = {
    {"boolTest", "true"},
    {"intTest", "123"},
    {"strTest", "\"testStr\""}
};
std::vector<ConfigModuleData> testConfig = {
    {"testModule", testData}
};

vector<ConfigKeyValue> formatErrorTestData = {
    {"boolTest", ",true"},
    {"intTest", "1#23"},
    {"strTest", "\"testStr\""}
};
std::vector<ConfigModuleData> formatErrorTestConfig = {
    {"formatErrorTestModule", formatErrorTestData}
};

TEST(ConfigManager, ConfigManager_)
{
    ConfigManager* configManager = new ConfigManager;
    delete configManager;
}

TEST(ConfigManager, ReadFile_createDefaultConfigFileWhenNoConfigFile)
{
    string backupFilePath = "/etc/pos/pos_backup.conf";
    string filePath = "/etc/pos/pos.conf";
    string defaultConfigFilePath = "/etc/pos/default_pos.conf";

    // given deleta config and default config file
    std::rename(filePath.data(), backupFilePath.data());
    std::remove(defaultConfigFilePath.data());
    EXPECT_TRUE(access(defaultConfigFilePath.c_str(), F_OK) != 0);

    // when read file
    ConfigManager configManager;
    EXPECT_TRUE(configManager.ReadFile() == EID(CONFIG_FILE_READ_DONE));

    // then create default config file
    EXPECT_TRUE(access(defaultConfigFilePath.c_str(), F_OK) == 0);

    std::rename(backupFilePath.data(), filePath.data());
}

TEST(ConfigManager, ReadFile_useDefaultConfigFileWhenNoConfigFile)
{
    string backupFilePath = "/etc/pos/pos_backup.conf";
    string filePath = "/etc/pos/pos.conf";
    string defaultConfigFilePath = "/etc/pos/default_pos.conf";

    // given default config file
    std::rename(filePath.data(), backupFilePath.data());
    std::remove(defaultConfigFilePath.data());
    EXPECT_TRUE(access(defaultConfigFilePath.c_str(), F_OK) != 0);

    // when read default config file
    ConfigManager configManager;
    EXPECT_TRUE(configManager.ReadFile() == EID(CONFIG_FILE_READ_DONE));
    EXPECT_TRUE(access(defaultConfigFilePath.c_str(), F_OK) == 0);

    // then get valid default config
    string moduleName = "user_nvme_driver";
    string key = "use_config";
    bool boolValue;
    ConfigType configType = CONFIG_TYPE_BOOL;
    EXPECT_TRUE(configManager.GetValue(moduleName, key, &boolValue, configType) == EID(SUCCESS));
    EXPECT_TRUE(boolValue == true);

    std::rename(backupFilePath.data(), filePath.data());
}

TEST(ConfigManager, ReadFile_readConfigFileSuccessWithTestConfigFile)
{
    string backupFilePath = "/etc/pos/pos_backup.conf";
    string filePath = "/etc/pos/pos.conf";

    // given test config file
    std::rename(filePath.data(), backupFilePath.data());
    std::ofstream outfile(filePath.data());
    JsonElement testConfigElem("");

    for (auto& moduleIter : testConfig)
    {
        string moduleName = moduleIter.moduleName;
        JsonElement moduleElem(moduleName);

        for (auto& dataIter : moduleIter.keyAndValueList)
        {
            string key = dataIter.key;
            string value = dataIter.value;
            JsonAttribute attribute(key, value);
            moduleElem.SetAttribute(attribute);
        }
        testConfigElem.SetElement(moduleElem);
    }
    outfile <<
        testConfigElem.ToJson(JsonFormatType::JSON_FORMAT_TYPE_READABLE)
            << std::endl;
    outfile.close();

    // when read config file
    // then read done
    ConfigManager configManager;
    EXPECT_TRUE(configManager.ReadFile() == EID(CONFIG_FILE_READ_DONE));

    std::rename(backupFilePath.data(), filePath.data());
}

TEST(ConfigManager, ReadFile_returnErrorWithFormatErrorConfigFile)
{
    string backupFilePath = "/etc/pos/pos_backup.conf";
    string filePath = "/etc/pos/pos.conf";

    // given format error config file
    std::rename(filePath.data(), backupFilePath.data());
    std::ofstream outfile(filePath.data());
    JsonElement testConfigElem("");

    for (auto& moduleIter : formatErrorTestConfig)
    {
        string moduleName = moduleIter.moduleName;
        JsonElement moduleElem(moduleName);

        for (auto& dataIter : moduleIter.keyAndValueList)
        {
            string key = dataIter.key;
            string value = dataIter.value;
            JsonAttribute attribute(key, value);
            moduleElem.SetAttribute(attribute);
        }
        testConfigElem.SetElement(moduleElem);
    }
    outfile <<
        testConfigElem.ToJson(JsonFormatType::JSON_FORMAT_TYPE_READABLE)
            << std::endl;
    outfile.close();

    // when read config file
    // then get return error
    ConfigManager configManager;
    EXPECT_TRUE(configManager.ReadFile() == EID(CONFIG_FILE_FORMAT_ERROR));

    std::rename(backupFilePath.data(), filePath.data());
}

TEST(ConfigManager, GetValue_getValueSuccessWithTestConfigFile)
{
    string backupFilePath = "/etc/pos/pos_backup.conf";
    string filePath = "/etc/pos/pos.conf";

    // given test config file
    std::rename(filePath.data(), backupFilePath.data());
    std::ofstream outfile(filePath.data());
    JsonElement testConfigElem("");

    for (auto& moduleIter : testConfig)
    {
        string moduleName = moduleIter.moduleName;
        JsonElement moduleElem(moduleName);

        for (auto& dataIter : moduleIter.keyAndValueList)
        {
            string key = dataIter.key;
            string value = dataIter.value;
            JsonAttribute attribute(key, value);
            moduleElem.SetAttribute(attribute);
        }
        testConfigElem.SetElement(moduleElem);
    }
    outfile <<
        testConfigElem.ToJson(JsonFormatType::JSON_FORMAT_TYPE_READABLE)
            << std::endl;
    outfile.close();

    ConfigManager configManager;

    // when get value
    // then get right value
    string moduleName = "testModule";
    string key = "intTest";
    int intValue;
    ConfigType configType = CONFIG_TYPE_INT;
    EXPECT_TRUE(configManager.GetValue(moduleName, key, &intValue, configType) == EID(SUCCESS));
    EXPECT_TRUE(intValue == 123);

    moduleName = "testModule";
    key = "boolTest";
    bool boolValue;
    configType = CONFIG_TYPE_BOOL;
    EXPECT_TRUE(configManager.GetValue(moduleName, key, &boolValue, configType) == EID(SUCCESS));
    EXPECT_TRUE(boolValue == true);

    moduleName = "testModule";
    key = "strTest";
    string strValue;
    configType = CONFIG_TYPE_STRING;
    EXPECT_TRUE(configManager.GetValue(moduleName, key, &strValue, configType) == EID(SUCCESS));
    EXPECT_TRUE(strValue == "testStr");

    std::rename(backupFilePath.data(), filePath.data());
}

TEST(ConfigManager, GetValue_failGetValueUsingDifferentConfigType)
{
    string backupFilePath = "/etc/pos/pos_backup.conf";
    string filePath = "/etc/pos/pos.conf";

    // given test config file 
    std::rename(filePath.data(), backupFilePath.data());
    std::ofstream outfile(filePath.data());
    JsonElement testConfigElem("");

    for (auto& moduleIter : testConfig)
    {
        string moduleName = moduleIter.moduleName;
        JsonElement moduleElem(moduleName);

        for (auto& dataIter : moduleIter.keyAndValueList)
        {
            string key = dataIter.key;
            string value = dataIter.value;
            JsonAttribute attribute(key, value);
            moduleElem.SetAttribute(attribute);
        }
        testConfigElem.SetElement(moduleElem);
    }
    outfile <<
        testConfigElem.ToJson(JsonFormatType::JSON_FORMAT_TYPE_READABLE)
            << std::endl;
    outfile.close();

    ConfigManager configManager;

    // when get value using falut config type
    string moduleName = "testModule";
    string key = "intTest";
    bool boolValue;
    ConfigType configType = CONFIG_TYPE_BOOL;
    // then return config type error
    EXPECT_TRUE(configManager.GetValue(moduleName, key, &boolValue, configType) == EID(CONFIG_VALUE_TYPE_ERROR));

    std::rename(backupFilePath.data(), filePath.data());
}

} // namespace pos
