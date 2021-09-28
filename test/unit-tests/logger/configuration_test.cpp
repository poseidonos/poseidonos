#include "src/logger/configuration.h"

#include <gtest/gtest.h>

namespace pos_logger
{
TEST(Configuration, LogSizePerFileInMB_)
{
}

TEST(Configuration, NumOfLogFilesForRotation_)
{
}

TEST(Configuration, LogLevel_testIfThereIsNoConfigAboutLogLevel)
{
    // Given
    Configuration* config = new Configuration();
    // When
    string ret = config->LogLevel();
    // Then
    ASSERT_EQ("debug", ret);
}

} // namespace pos_logger
