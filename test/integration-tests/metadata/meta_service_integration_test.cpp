#include "src/meta_service/meta_service.h"

#include <string>

#include "gtest/gtest.h"
#include "test/integration-tests/journal/fixture/journal_manager_test_fixture.h"

namespace pos
{
class MetaServiceIntegrationTest : public ::testing::Test
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
MetaServiceIntegrationTest::SetUp(void)
{
}

void
MetaServiceIntegrationTest::TearDown(void)
{
}

void
MetaServiceIntegrationTest::InitializeAndRegisterJournal(std::string arrayName, int arrayId, JournalConfigurationSpy* config)
{
    std::string logFileName = arrayName + "_" + GetLogFileName();
    JournalManagerTestFixture* testFixture = new JournalManagerTestFixture(logFileName);
    testFixture->InitializeJournal(config);
    fixtures[arrayName] = testFixture;

    JournalManagerSpy* journal = fixtures[arrayName]->GetJournal();
    MetaServiceSingleton::Instance()->Register(arrayName, arrayId, nullptr, journal->GetStatusProvider());
}

void
MetaServiceIntegrationTest::ResetAndUnregisterJournal(std::string arrayName)
{
    MetaServiceSingleton::Instance()->Unregister(arrayName);

    if (fixtures.find(arrayName) != fixtures.end())
    {
        JournalManagerTestFixture* fixture = fixtures[arrayName];

        delete fixture;
    }
}

TEST_F(MetaServiceIntegrationTest, RegisterEnabledJournal)
{
    std::string arrayName = "POSArray";
    int arrayId = 0;
    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(true);

    InitializeAndRegisterJournal(arrayName, arrayId, builder.Build());

    auto statusProvider = MetaServiceSingleton::Instance()->GetJournalStatusProvider(arrayId);
    EXPECT_TRUE(statusProvider->IsJournalEnabled() == true);

    int logWriteResult = fixtures[arrayName]->AddDummyLog();
    EXPECT_TRUE(logWriteResult == 0);

    ResetAndUnregisterJournal(arrayName);
}

TEST_F(MetaServiceIntegrationTest, RegisterDisabledJournal)
{
    std::string arrayName = "POSArray";
    int arrayId = 0;

    JournalConfigurationBuilder builder(&testInfo);
    builder.SetJournalEnable(false);

    InitializeAndRegisterJournal(arrayName, arrayId, builder.Build());

    auto statusProvider = MetaServiceSingleton::Instance()->GetJournalStatusProvider(arrayId);
    EXPECT_TRUE(statusProvider->IsJournalEnabled() == false);

    int logWriteResult = fixtures[arrayName]->AddDummyLog();
    EXPECT_TRUE(logWriteResult != 0);

    ResetAndUnregisterJournal(arrayName);
}

TEST_F(MetaServiceIntegrationTest, RegisterEnabledJournals)
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
        auto statusProvider = MetaServiceSingleton::Instance()->GetJournalStatusProvider(arrayName);
        EXPECT_TRUE(statusProvider->IsJournalEnabled() == true);

        int logWriteResult = fixtures[arrayName]->AddDummyLog();
        EXPECT_TRUE(logWriteResult == 0);
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        ResetAndUnregisterJournal(arrayName);
    }
}

TEST_F(MetaServiceIntegrationTest, RegisterDisabledJournals)
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
        auto statusProvider = MetaServiceSingleton::Instance()->GetJournalStatusProvider(arrayName);
        EXPECT_TRUE(statusProvider->IsJournalEnabled() == false);

        int logWriteResult = fixtures[arrayName]->AddDummyLog();
        EXPECT_TRUE(logWriteResult < 0);
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        ResetAndUnregisterJournal(arrayName);
    }
}

TEST_F(MetaServiceIntegrationTest, GetServiceById)
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

        IJournalStatusProvider* journalByName = MetaServiceSingleton::Instance()->GetJournalStatusProvider(arrayName);
        IJournalStatusProvider* journalById = MetaServiceSingleton::Instance()->GetJournalStatusProvider(id);

        EXPECT_EQ(journalByName, journalById);
    }

    for (int id = 0; id < numArrays; id++)
    {
        std::string arrayName = "array" + std::to_string(id);
        ResetAndUnregisterJournal(arrayName);
    }
}
} // namespace pos
