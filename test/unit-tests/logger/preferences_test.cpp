#include "src/logger/preferences.h"

#include <gtest/gtest.h>

namespace pos_logger
{
TEST(Preferences, Preferences_)
{
}

TEST(Preferences, LogDir_)
{
}

TEST(Preferences, MinorLogFilePath_)
{
}

TEST(Preferences, MajorLogFilePath_)
{
}

TEST(Preferences, FilterFilePath_)
{
}

TEST(Preferences, LogFileSize_)
{
}

TEST(Preferences, LogRotation_)
{
}

TEST(Preferences, LogLevel_)
{
}

TEST(Preferences, SetLogLevel_)
{
}

TEST(Preferences, ToJson_)
{
}

TEST(Preferences, ApplyFilter_)
{
    // Given
    Preferences pref;
    // When
    pref.ApplyFilter("");
    // Then
}

TEST(Preferences, ShouldLog_)
{
}

TEST(Preferences, LogLevelToString_)
{
}

TEST(Preferences, StringToLogLevel_)
{
}

} // namespace pos_logger
