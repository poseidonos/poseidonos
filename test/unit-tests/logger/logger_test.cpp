#include "src/logger/logger.h"

#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "test/unit-tests/logger/logger_mock.h"

using json = nlohmann::json;
using ::testing::NiceMock;

namespace pos
{
TEST(Logger, Logger_testWhetherTheResultOfGetlevelAndInputOfSetlevelAreTheSame)
{
    Logger* logger = new Logger();
    string inputLevel = "warning";
    string expectedOutput = inputLevel;
    logger->SetLevel(inputLevel);
    string actualOutput = logger->GetLevel();

    ASSERT_EQ(expectedOutput, actualOutput);
}

TEST(Logger, SetLevel_tesIfLevelOffIsSetWhenSetTheWrongLoglevel)
{
    Logger* logger = new Logger();
    string inputLevel = "wrongLevel";
    string expectedOutput = "debug";
    logger->SetLevel(inputLevel);
    string actualOutput = logger->GetLevel();

    ASSERT_EQ(expectedOutput, actualOutput);
}

TEST(Logger, GetLogDir_testIfLogDirIsSameAsPreferences)
{
    Logger* logger = new Logger();
    string dirInPreferences = "/var/log/pos/";
    string actualOutput = logger->GetLogDir();

    ASSERT_EQ(dirInPreferences, actualOutput);
}

TEST(Logger, PosLog_checkIfPosLogOfLoggerWritesWell)
{
    // Backup existing log
    string posLog = "/var/log/pos/pos.log";
    string posLogBackup = "/var/log/pos/pos.log.bak.test";
    rename(posLog.c_str(), posLogBackup.c_str());

    // When
    Logger* logger = new Logger();
    logger->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, 9999, "test string");
    ifstream f(posLog.c_str());
    bool exists = f.good();

    // Then
    ASSERT_TRUE(exists);

    // Rollback previous log
    remove(posLog.c_str());
    rename(posLogBackup.c_str(), posLog.c_str());
}

TEST(Reporter, PosLog_checkIfPosLogOfReporterWritesWell)
{
    // Backup existing report
    string posReport = "/var/log/pos/report.log";
    string posReportBackup = "/var/log/pos/report.log.bak.test";
    rename(posReport.c_str(), posReportBackup.c_str());

    // When
    Reporter* reporter = new Reporter();
    reporter->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, 9999, "test string");
    ifstream f(posReport.c_str());
    bool exists = f.good();

    // Then
    ASSERT_TRUE(exists);

    // Rollback previous report
    remove(posReport.c_str());
    rename(posReportBackup.c_str(), posReport.c_str());
}

TEST(Logger, ApplyFilter_testIfFilterIsAppliedWellByGetPreferencesAfterApplyingTheFilter)
{
    // Backup existing filter
    string filterPath = "/var/log/pos/filter";
    string filterBackup = "/var/log/pos/filter.bak.test";
    rename(filterPath.c_str(), filterBackup.c_str());

    // Given
    ofstream f;
    f.open(filterPath, ios::out);
    if (f.is_open())
    {
        string filterInclude = "include:1000-1010";
        string filterExclude = "exclude:1005,1006";
        f << filterInclude << endl << filterExclude;
    }
    f.close();

    Logger* logger = new Logger();

    // When
    logger->ApplyFilter();

    // Rollback previous filter
    remove(filterPath.c_str());
    rename(filterBackup.c_str(), filterPath.c_str());

    // Then
    JsonElement preferences = logger->GetPreference();
    string jsonString = "{" + preferences.ToJson() + "}";
    json jsonDoc = json::parse(jsonString.c_str());

    string filterIncluded = "";
    if (jsonDoc["data"].contains("filterIncluded"))
    {
        filterIncluded = jsonDoc["data"]["filterIncluded"].get<string>();
    }

    string filterExcluded = "";
    if (jsonDoc["data"].contains("filterExcluded"))
    {
        filterExcluded = jsonDoc["data"]["filterExcluded"].get<string>();
    }

    int filterEnabled = 0;
    if (jsonDoc["data"].contains("filterEnabled"))
    {
        filterEnabled = jsonDoc["data"]["filterEnabled"].get<int>();
    }

    ASSERT_EQ(filterEnabled, 1);
    ASSERT_EQ(filterIncluded, "1000-1010");
    ASSERT_EQ(filterExcluded, "1005,1006");
}


TEST(Logger, ShouldLog_testIfFilterIsApplied)
{
    // Backup existing filter
    string filterPath = "/var/log/pos/filter";
    string filterBackup = "/var/log/pos/filter.bak.test";
    rename(filterPath.c_str(), filterBackup.c_str());

    // Given
    ofstream f;
    f.open(filterPath, ios::out);
    if (f.is_open())
    {
        string filterInclude = "include:1000-1010";
        string filterExclude = "exclude:3000,3001";
        f << filterInclude << endl << filterExclude;
    }
    f.close();

    Logger* logger = new Logger();

    // When
    logger->ApplyFilter();

    // Rollback previous filter
    remove(filterPath.c_str());
    rename(filterBackup.c_str(), filterPath.c_str());

    // Then
    bool include = logger->ShouldLog(spdlog::level::info, 1005);
    bool exclude = logger->ShouldLog(spdlog::level::info, 3000);

    ASSERT_EQ(include, true);
    ASSERT_EQ(exclude, false);
}

TEST(ChangeLogger, LoggingStateChangeConditionally_testIfLoggerWillNotPrintAnyLogWhenCurrentStateIsTheSame)
{
    NiceMock<Logger> testLogger;
    ChangeLogger<int> logger(&testLogger, 0);
    POS_TRACE_ERROR_CONDITIONALLY(&logger, 0, 0, "test");

    // when the current value is the same as prev value
    // the count will increase
    int expectedCount = 1;
    EXPECT_EQ(logger.GetCount(), expectedCount);
}

TEST(ChangeLogger, LoggingStateChangeConditionally_testIfLoggerWillPrintLogWhenCurrentStateIsTheSame)
{
    NiceMock<Logger> testLogger;
    ChangeLogger<int> logger(&testLogger, 0);
    POS_TRACE_ERROR_CONDITIONALLY(&logger, 0, 1, "test");

    // when the current value is not the same as prev value
    // the count will be cleared
    int expectedCount = 0;
    EXPECT_EQ(logger.GetCount(), expectedCount);
}
} // namespace pos
