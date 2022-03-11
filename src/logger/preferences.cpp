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

#include "preferences.h"

#include "configuration.h"
#include "src/dump/dump_shared_ptr.h"
#include "src/include/pos_event_id.h"
#include "src/event_scheduler/callback.h"

namespace pos_logger
{
Preferences::Preferences()
{
    Configuration conf;
    logfileSize = conf.LogSizePerFileInMB();
    logRotation = conf.NumOfLogFilesForRotation();
    logLevel = StringToLogLevel(conf.LogLevel());
    EnableStructuredLogging = conf.IsStrLoggingEnabled();

    ApplyFilter();
}

bool
Preferences::ShouldLog(spdlog::level::level_enum lvl, int id)
{
    if (lvl < logLevel)
    {
        return false;
    }

    return filter.ShouldLog(id);
}

int
Preferences::SetLogLevel(shared_ptr<spdlog::logger> logger, string value)
{
    logger->set_level(StringToLogLevel(value));
    logLevel = logger->level();

    if (value != "off" && logLevel == spdlog::level::off)
    {
        logger->set_level(spdlog::level::debug);
        logLevel = logger->level();
    }

    pos::DumpSharedModuleInstanceEnable::debugLevelEnable = (logLevel == SPDLOG_LEVEL_DEBUG);

    if (value == LogLevelToString(logLevel))
    {
        return EID(SUCCESS);
    }
    else
    {
        return (int)POS_EVENT_ID::LOGGER_SET_LEVEL_FAILED;
    }
}

JsonElement
Preferences::ToJson()
{
    JsonElement data("data");
    data.SetAttribute(JsonAttribute("minor_log_path", "\"" + MinorLogFilePath() + "\""));
    data.SetAttribute(JsonAttribute("major_log_path", "\"" + MajorLogFilePath() + "\""));
    data.SetAttribute(JsonAttribute("logfile_size_in_mb", logfileSize));
    data.SetAttribute(JsonAttribute("logfile_rotation_count", logRotation));
    data.SetAttribute(JsonAttribute("min_allowable_log_level", "\"" + LogLevelToString(logLevel) + "\""));
    data.SetAttribute(JsonAttribute("filter_enabled", filter.IsFiltered()));
    if (filter.IsFiltered() == true)
    {
        data.SetAttribute(JsonAttribute("filter_included", "\"" + filter.IncludeRule() + "\""));
        data.SetAttribute(JsonAttribute("filter_excluded", "\"" + filter.ExcludeRule() + "\""));
    }
    data.SetAttribute(JsonAttribute("structured_logging", EnableStructuredLogging ? "true" : "false"));

    return data;
}

int
Preferences::ApplyFilter()
{
    return filter.ApplyFilter(FilterFilePath());
}

int
Preferences::ApplyFilter(string filePath)
{
    return filter.ApplyFilter(filePath);
}

string
Preferences::LogLevelToString(spdlog::level::level_enum lvl)
{
    return LOG_LEVEL_NAME[(int)lvl];
}

spdlog::level::level_enum
Preferences::StringToLogLevel(string lvl)
{
    for (int i = 0; i < LOG_LEVEL_SIZE; i++)
    {
        if (LOG_LEVEL_NAME[i] == lvl)
        {
            return static_cast<spdlog::level::level_enum>(i);
        }
    }
    return spdlog::level::off;
}

int
Preferences::SetStrLogging(bool input)
{
    EnableStructuredLogging = input;

    return EID(SUCCESS);
}

} // namespace pos_logger
