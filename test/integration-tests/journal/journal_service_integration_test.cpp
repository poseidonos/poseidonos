#include "src/journal_service/journal_service.h"

#include <string>

#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

namespace pos
{
class JournalServiceIntegrationTest : public ::testing::Test
{
public:
    void InitializeAndRegisterJournal(std::string arrayName, int arrayId, JournalConfigurationSpy* config);
    void ResetAndUnregisterJournal(std::string arrayName);

    void TestJournalEnabled(int id);
    void TestJournalDisabled(int id);

protected:
    virtual void SetUp(void);
    virtual void TearDown(void);

    TestInfo testInfo;
    std::unordered_map<std::string, JournalManagerTestFixture*> fixtures;
};

void
JournalServiceIntegrationTest::SetUp(void)
{
}

void
JournalServiceIntegrationTest::TearDown(void)
{
}

void
JournalServiceIntegrationTest::InitializeAndRegisterJournal(std::string arrayName, int arrayId, JournalConfigurationSpy* config)
{
    std::string logFileName = arrayName + "_" + GetLogFileName();
    JournalManagerTestFixture* testFixture = new JournalManagerTestFixture(logFileName);
    testFixture->InitializeJournal(config);
    fixtures[arrayName] = testFixture;

    JournalManagerSpy* journal = fixtures[arrayName]->GetJournal();
    JournalServiceSingleton::Instance()->Register(arrayName, arrayId,
        journal, journal->GetJournalWriter(), journal->GetStatusProvider());
}

void
JournalServiceIntegrationTest::ResetAndUnregisterJournal(std::string arrayName)
{
    JournalServiceSingleton::Instance()->Unregister(arrayName);

    if (fixtures.find(arrayName) != fixtures.end())
    {
        JournalManagerTestFixture* fixture = fixtures[arrayName];

        delete fixture;
    }
}

TEST_F(JournalServiceIntegrationTest, RegisterEnabledJournal)
{
    std::string arrayName = "POSArray";
    int arrayId = 0;
    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(true);

    InitializeAndRegisterJournal(arrayName, arrayId, builder.Build());

    EXPECT_TRUE(JournalServiceSingleton::Instance()->IsEnabled(arrayName) == true);

    int logWriteResult = fixtures[arrayName]->AddDummyLog();
    EXPECT_TRUE(logWriteResult == 0);

    ResetAndUnregisterJournal(arrayName);
}

TEST_F(JournalServiceIntegrationTest, RegisterDisabledJournal)
{
    std::string arrayName = "POSArray";
    int arrayId = 0;

    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(false);

    InitializeAndRegisterJournal(arrayName, arrayId, builder.Build());

    EXPECT_TRUE(JournalServiceSingleton::Instance()->IsEnabled(arrayName) == false);

    int logWriteResult = fixtures[arrayName]->AddDummyLog();
    EXPECT_TRUE(logWriteResult != 0);

    ResetAndUnregisterJournal(arrayName);
}

TEST_F(JournalServiceIntegrationTest, RegisterEnabledJournals)
{
    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(true);

    int numArrays = ArrayMgmtPolicy::MAX_ARRAY_CNT;
    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        InitializeAndRegisterJournal(arrayName, id, builder.Build());
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        EXPECT_TRUE(JournalServiceSingleton::Instance()->IsEnabled(arrayName) == true);

        int logWriteResult = fixtures[arrayName]->AddDummyLog();
        EXPECT_TRUE(logWriteResult == 0);
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        ResetAndUnregisterJournal(arrayName);
    }
}

TEST_F(JournalServiceIntegrationTest, RegisterDisabledJournals)
{
    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(false);

    int numArrays = ArrayMgmtPolicy::MAX_ARRAY_CNT;
    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        InitializeAndRegisterJournal(arrayName, id, builder.Build());
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        EXPECT_TRUE(JournalServiceSingleton::Instance()->IsEnabled(arrayName) == false);

        int logWriteResult = fixtures[arrayName]->AddDummyLog();
        EXPECT_TRUE(logWriteResult < 0);
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        ResetAndUnregisterJournal(arrayName);
    }
}

TEST_F(JournalServiceIntegrationTest, GetServiceById)
{
    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(true);

    int numArrays = ArrayMgmtPolicy::MAX_ARRAY_CNT;
    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        InitializeAndRegisterJournal(arrayName, id, builder.Build());
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);

        IJournalWriter* journalByName = JournalServiceSingleton::Instance()->GetWriter(arrayName);
        IJournalWriter* journalById = JournalServiceSingleton::Instance()->GetWriter(id);

        EXPECT_EQ(journalByName, journalById);
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        ResetAndUnregisterJournal(arrayName);
    }
}
} // namespace pos
