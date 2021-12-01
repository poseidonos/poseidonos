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

#include "logger.h"

#include <memory>
#include <vector>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "src/helper/file/directory.h"
#include "src/include/pos_event_id.h"
#include "src/include/memory.h"

namespace pos
{
Logger::Logger(void)
{
    if (DirExists(preferences.LogDir()) == false)
    {
        MakeDir(preferences.LogDir());
    }

    const string pattern = "[%Y-%m-%d %H:%M:%S.%e][%q][%l] %v at %@"; // Format: [2021-12-01 07:00:01.234][EventID]][LogLevel] Message at SourceFile and LineNumber
    std::vector<spdlog::sink_ptr> sinks;
    auto console_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);
    console_sink->set_pattern(pattern);
    auto minor_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
        preferences.MinorLogFilePath(), preferences.LogFileSize() * SZ_1MB, preferences.LogRotation());
    minor_sink->set_level(spdlog::level::debug);
    minor_sink->set_pattern(pattern);
    auto major_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
        preferences.MajorLogFilePath(), preferences.LogFileSize() * SZ_1MB, preferences.LogRotation());
    major_sink->set_level(spdlog::level::warn);
    major_sink->set_pattern(pattern);
    sinks.push_back(console_sink);
    sinks.push_back(minor_sink);
    sinks.push_back(major_sink);

    for (uint32_t i = 0;
         i < static_cast<uint32_t>(ModuleInDebugLogDump::MAX_SIZE); i++)
    {
        std::string str = "Logger";
        str += to_string(i);
        dumpModule[i] = new DumpModule<DumpBuffer>(str, MAX_LOGGER_DUMP_SIZE / AVG_LINE, true);
    }

    logger = std::make_shared<spdlog::logger>("pos_logger", begin(sinks), end(sinks));
    logger->flush_on(spdlog::level::debug);
    SetLevel(preferences.LogLevel());
}

Logger::~Logger(void)
{
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(ModuleInDebugLogDump::MAX_SIZE); i++)
    {
        delete dumpModule[i];
    }
}

int
Logger::SetLevel(string level)
{
    return preferences.SetLogLevel(logger, level);
}

string
Logger::GetLevel()
{
    return preferences.LogLevel();
}

Reporter::Reporter(void)
{
    if (DirExists(REPORT_PATH) == false)
    {
        MakeDir(REPORT_PATH);
    }

    const string pattern = "[%E][%q][%l] %v"; // [Seconds since the epoch][1001][info] blah blah ~
    auto reporter_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
        REPORT_PATH + REPORT_NAME, SIZE_MB * SZ_1MB, ROTATION);
    reporter_sink->set_level(spdlog::level::info);
    reporter_sink->set_pattern(pattern);
    reporter = std::make_shared<spdlog::logger>("pos_reporter", reporter_sink);
    reporter->set_level(spdlog::level::info);
    reporter->flush_on(spdlog::level::info);
}
} // namespace pos
