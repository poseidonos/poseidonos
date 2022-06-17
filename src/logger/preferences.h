/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <string>

#include "filter.h"
#include "spdlog/spdlog.h"
#include "src/helper/json/json_helper.h"

using namespace std;
namespace pos_logger
{
class Preferences
{
public:
    Preferences();
    string
    LogDir()
    {
        return LOG_PATH;
    }
    string
    MinorLogFilePath()
    {
        return LOG_PATH + MINOR_LOG_NAME;
    }
    string
    MajorLogFilePath()
    {
        return LOG_PATH + MAJOR_LOG_NAME;
    }
    string
    FilterFilePath()
    {
        return LOG_PATH + FILTER_NAME;
    }
    uint32_t
    LogFileSize()
    {
        return logfileSize;
    }
    uint32_t
    LogRotation()
    {
        return logRotation;
    }
    string
    LogLevel()
    {
        return LogLevelToString(logLevel);
    }
    bool
    IsFiltered()
    {
        return filter.IsFiltered();
    }
    string
    IncludeRule()
    {
        return filter.IncludeRule();
    }
    string
    ExcludeRule()
    {
        return filter.ExcludeRule();
    }

    int SetLogLevel(shared_ptr<spdlog::logger> logger, string value);
    int SetStrLogging(bool input);
    bool IsStrLoggingEnabled() { return EnableStructuredLogging; }
    JsonElement ToJson();
    int ApplyFilter();
    int ApplyFilter(string filePath);
    bool ShouldLog(spdlog::level::level_enum lvl, int id);
    string LogLevelToString(spdlog::level::level_enum lvl);
    spdlog::level::level_enum StringToLogLevel(string lvl);

private:
    const string LOG_PATH = "/var/log/pos/";
    const string MINOR_LOG_NAME = "pos.log";
    const string MAJOR_LOG_NAME = "pos_major.log";
    const string FILTER_NAME = "filter";
    static const int LOG_LEVEL_SIZE = 7;
    const string LOG_LEVEL_NAME[LOG_LEVEL_SIZE] = {
        "debug", "info", "trace", "warning", "error", "critical", "off"};

    uint32_t logfileSize;
    uint32_t logRotation;
    spdlog::level::level_enum logLevel;
    Filter filter;
    bool EnableStructuredLogging;
};
} // namespace pos_logger
