#include "src/journal_manager/config/journal_configuration.h"

#include <gtest/gtest.h>

#include "test/unit-tests/metafs/mai/metafs_file_control_api_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(JournalConfiguration, JournalConfiguration_)
{
}

TEST(JournalConfiguration, Init_testIfLogBufferSetWhenLoadedLogBufferSizeIsZero)
{
    // Given
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    JournalConfiguration config("POSArray", &metaFsCtrl);

    uint64_t metaPageSize = 4032;
    uint64_t logBufferSize = 16 * 1024 * 1024;
    int numLogGroups = 2;

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetTheBiggestExtentSize).WillByDefault(Return(logBufferSize));

    // When
    config.Init(0);

    // Then
    uint64_t expectedSize = (logBufferSize - metaPageSize) / (metaPageSize * numLogGroups) * (metaPageSize * numLogGroups);
    EXPECT_EQ(config.GetLogBufferSize(), expectedSize);
}

TEST(JournalConfiguration, Init_testIfLogBufferSetWhenLoadedLogBufferSizeIsNotZero)
{
    // Given
    NiceMock<MockMetaFsFileControlApi> metaFsCtrl;
    JournalConfiguration config("POSArray", &metaFsCtrl);

    uint64_t metaPageSize = 4032;
    uint64_t logBufferSize = 16 * 1024 * 1024;

    ON_CALL(metaFsCtrl, EstimateAlignedFileIOSize).WillByDefault(Return(metaPageSize));
    ON_CALL(metaFsCtrl, GetTheBiggestExtentSize).WillByDefault(Return(logBufferSize));

    // When
    uint64_t loadedLogBufferSize = 16 * 1024;
    config.Init(loadedLogBufferSize);

    // Then
    EXPECT_EQ(config.GetLogBufferSize(), loadedLogBufferSize);
}

TEST(JournalConfiguration, IsEnabled_)
{
}

TEST(JournalConfiguration, GetNumLogGroups_)
{
}

TEST(JournalConfiguration, GetLogBufferSize_)
{
}

TEST(JournalConfiguration, GetLogGroupSize_)
{
}

TEST(JournalConfiguration, GetMetaPageSize_)
{
}

TEST(JournalConfiguration, UpdateLogBufferSize_)
{
}

TEST(JournalConfiguration, _ConfigureLogBufferSize_)
{
}

TEST(JournalConfiguration, _GetAlignedSize_)
{
}

TEST(JournalConfiguration, _ReadConfiguration_)
{
}

TEST(JournalConfiguration, _ReadMetaFsConfiguration_)
{
}

} // namespace pos
