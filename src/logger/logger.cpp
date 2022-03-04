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
#include "src/master_context/instance_id_provider.h"
#include "src/master_context/version_provider.h"

namespace pos
{


string
BuildPattern(bool isStrLoggingEnabled)
{
    std::string version = VersionProviderSingleton::Instance()->GetVersion();
    id_t instanceId = InstanceIdProviderSingleton::Instance()->GetInstanceId();
    std::string pattern = "";

    // Structured Log Format
    const std::string strLogPattern =
        {std::string("{\"datetime\":\"%Y-%m-%d %H:%M:%S.%F\"") +
            ",\"process_id\":%P,\"thread_id\":%t" +
            ",\"pos_id\":" + std::to_string(instanceId) +
            ",\"event_id\":%q" +
            ",\"level\":\"%^%l%$\",\"description\":{%v}" +
            ",\"source\":\"%s\",\"line\":\"%#\",\"function\":\"%!\"" +
            ",\"pos_version\":" + "\"" + version + "\"},"};

    // Plain-text log entry format
    // [datetime][process_id][thread_id][pos_id][event_id]][level]
    // message, cause, source: file:line function_name(), pos_version: version
    const std::string plainTextpattern =
        std::string("[%Y-%m-%d %H:%M:%S.%F][%P][%t]") +
        "[" + std::to_string(instanceId) + "]" +
         "[%q][%=7l] %v, source: %@ %!(), pos_version: " + version;

    pattern = isStrLoggingEnabled ? strLogPattern : plainTextpattern;

    return pattern;
}

Logger::Logger(void)
{
    if (DirExists(preferences.LogDir()) == false)
    {
        MakeDir(preferences.LogDir());
    }

    std::vector<spdlog::sink_ptr> sinks;
    auto console_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto minor_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
        preferences.MinorLogFilePath(), preferences.LogFileSize() * SZ_1MB, preferences.LogRotation());
    auto major_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
        preferences.MajorLogFilePath(), preferences.LogFileSize() * SZ_1MB, preferences.LogRotation());

    console_sink->set_level(spdlog::level::trace);
    minor_sink->set_level(spdlog::level::debug);
    major_sink->set_level(spdlog::level::warn);

    // Console log is always displayed in plain text form
    console_sink->set_pattern(BuildPattern(false));
    // Minor and major logs will be displayed
    // according to preference (plain text or json)
    minor_sink->set_pattern(BuildPattern(preferences.IsStrLoggingEnabled()));
    major_sink->set_pattern(BuildPattern(preferences.IsStrLoggingEnabled()));

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

void
Logger::ApplyPreference(void)
{
    if (DirExists(preferences.LogDir()) == false)
    {
        MakeDir(preferences.LogDir());
    }

    string pattern = BuildPattern(preferences.IsStrLoggingEnabled());
    logger->sinks()[1]->set_pattern(pattern); // Index 1: minor sink
    logger->sinks()[2]->set_pattern(pattern); // Index 2: major sink
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

int
Logger::SetStrLogging(bool input)
{
    return preferences.SetStrLogging(input);
}

Reporter::Reporter(void)
{
    if (DirExists(REPORT_PATH) == false)
    {
        MakeDir(REPORT_PATH);
    }

    const string pattern = BuildPattern(preferences.IsStrLoggingEnabled());
    auto reporter_sink = make_shared<spdlog::sinks::rotating_file_sink_mt>(
        REPORT_PATH + REPORT_NAME, SIZE_MB * SZ_1MB, ROTATION);
    reporter_sink->set_level(spdlog::level::info);
    reporter_sink->set_pattern(pattern);
    reporter = std::make_shared<spdlog::logger>("pos_reporter", reporter_sink);
    reporter->set_level(spdlog::level::info);
    reporter->flush_on(spdlog::level::info);
}
} // namespace pos
