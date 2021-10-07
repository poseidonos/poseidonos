#include "src/meta_service/meta_service.h"

#include <gtest/gtest.h>

#include "test/unit-tests/meta_service/i_meta_updater_mock.h"
#include "test/unit-tests/journal_manager/i_journal_status_provider_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(MetaService, Register_testIfMapUpdaterIsRegisteredSuccessfully)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    NiceMock<MockIJournalStatusProvider> journalStatus;
    service->Register(arrayName, arrayId, &metaUpdater, &journalStatus);

    EXPECT_EQ(service->GetMetaUpdater(arrayId), &metaUpdater);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayId), &journalStatus);

    EXPECT_EQ(service->GetMetaUpdater(arrayName), &metaUpdater);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayName), &journalStatus);

    MetaServiceSingleton::ResetInstance();
}

TEST(MetaService, Register_testIfExecutedSuccessfullyWhenRegisteredTwice)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    NiceMock<MockIJournalStatusProvider> journalStatus;
    service->Register(arrayName, arrayId, &metaUpdater, &journalStatus);
    service->Register(arrayName, arrayId, &metaUpdater, &journalStatus);

    EXPECT_EQ(service->GetMetaUpdater(arrayId), &metaUpdater);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayId), &journalStatus);

    EXPECT_EQ(service->GetMetaUpdater(arrayName), &metaUpdater);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayName), &journalStatus);

    MetaServiceSingleton::ResetInstance();
}

TEST(MetaService, Unregister_testIfMapUpdaterIsUnregisteredSuccessfully)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    NiceMock<MockIJournalStatusProvider> journalStatus;
    service->Register(arrayName, arrayId, &metaUpdater, &journalStatus);
    service->Unregister(arrayName);

    EXPECT_EQ(service->GetMetaUpdater(arrayId), nullptr);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayId), nullptr);

    EXPECT_EQ(service->GetMetaUpdater(arrayName), nullptr);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayName), nullptr);

    MetaServiceSingleton::ResetInstance();
}

TEST(MetaService, Unregister_testIfExecutedSuccessfullyWhenUnregisteredTwice)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    NiceMock<MockIJournalStatusProvider> journalStatus;
    service->Register(arrayName, arrayId, &metaUpdater, &journalStatus);
    service->Unregister(arrayName);
    service->Unregister(arrayName);

    EXPECT_EQ(service->GetMetaUpdater(arrayId), nullptr);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayId), nullptr);

    EXPECT_EQ(service->GetMetaUpdater(arrayName), nullptr);
    EXPECT_EQ(service->GetJournalStatusProvider(arrayName), nullptr);

    MetaServiceSingleton::ResetInstance();
}
} // namespace pos
