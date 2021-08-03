#include "src/meta_service/meta_service.h"

#include <gtest/gtest.h>

#include "test/unit-tests/meta_service/i_meta_updater_mock.h"

using ::testing::NiceMock;

namespace pos
{
TEST(MetaService, Register_testIfMapUpdaterIsRegisteredSuccessfully)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    service->Register(arrayName, arrayId, &metaUpdater);

    auto actualById = service->GetMetaUpdater(arrayId);
    EXPECT_EQ(actualById, &metaUpdater);

    auto actualByName = service->GetMetaUpdater(arrayName);
    EXPECT_EQ(actualByName, &metaUpdater);
}

TEST(MetaService, Register_testIfExecutedSuccessfullyWhenRegisteredTwice)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    service->Register(arrayName, arrayId, &metaUpdater);
    service->Register(arrayName, arrayId, &metaUpdater);

    auto actualById = service->GetMetaUpdater(arrayId);
    EXPECT_EQ(actualById, &metaUpdater);
}

TEST(MetaService, Unregister_testIfMapUpdaterIsUnregisteredSuccessfully)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    service->Register(arrayName, arrayId, &metaUpdater);
    service->Unregister(arrayName);

    auto actualById = service->GetMetaUpdater(arrayId);
    EXPECT_EQ(actualById, nullptr);

    auto actualByName = service->GetMetaUpdater(arrayName);
    EXPECT_EQ(actualByName, nullptr);
}

TEST(MetaService, Unregister_testIfExecutedSuccessfullyWhenUnregisteredTwice)
{
    MetaService* service = MetaServiceSingleton::Instance();

    std::string arrayName = "POSArray";
    int arrayId = 1;
    NiceMock<MockIMetaUpdater> metaUpdater;
    service->Register(arrayName, arrayId, &metaUpdater);
    service->Unregister(arrayName);
    service->Unregister(arrayName);

    auto actualById = service->GetMetaUpdater(arrayId);
    EXPECT_EQ(actualById, nullptr);
}
} // namespace pos
