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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <set>
#include <string>

#include "preferences.h"
#include "spdlog/spdlog.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/lib/singleton.h"

using namespace std;

namespace ibofos
{
enum class ModuleInDebugLogDump
{
    IO_FLUSH,
    IO_GENERAL,
    CALLBACK_TIMEOUT,
    FLUSH_CMD,
    MAX_SIZE,
};

class Logger
{
public:
    Logger();
    ~Logger();

    template<typename... Args>
    void
    IboflogWithDump(ModuleInDebugLogDump module,
        spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int id, spdlog::string_view_t fmt, const Args&... args)
    {
        if (_ShouldLog(lvl, id, fmt))
        {
            uint32_t moduleId = static_cast<uint32_t>(module);
            fmt::memory_buffer buf;
            fmt::format_to(buf, fmt, args...);
            DumpModule<DumpBuffer>* dumpModulePtr = dumpModule[moduleId];
            DumpBuffer dumpBuffer(buf.data(), buf.size(), dumpModulePtr);
            dumpModulePtr->AddDump(dumpBuffer, 0);
        }
    }

    template<typename... Args>
    void
    Iboflog(spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int id, spdlog::string_view_t fmt, const Args&... args)
    {
        if (_ShouldLog(lvl, id, fmt))
        {
            logger->iboflog_sink(loc, lvl, id, fmt, args...);
        }
    }
    int SetLevel(string lvl);
    string GetLevel();

    int
    ApplyFilter()
    {
        return preferences.ApplyFilter();
    }

    JsonElement
    GetPreference()
    {
        return preferences.ToJson();
    }

private:
    bool _ShouldLog(spdlog::level::level_enum lvl, int id, spdlog::string_view_t fmt);
    void _InitDeduplicator();
    const uint32_t MAX_LOGGER_DUMP_SIZE = 1 * 1024 * 1024;
    const uint32_t AVG_LINE = 80;
    DumpModule<DumpBuffer>* dumpModule[static_cast<uint32_t>(ModuleInDebugLogDump::MAX_SIZE)];
    shared_ptr<spdlog::logger> logger;
    ibofos_logger::Preferences preferences;
};

using LoggerSingleton = ibofos::Singleton<Logger>;

class Reporter
{
public:
    Reporter();
    template<typename... Args>
    void
    Iboflog(spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int id, spdlog::string_view_t fmt, const Args&... args)
    {
        reporter->iboflog_sink(loc, lvl, id, fmt, args...);
    }

private:
    shared_ptr<spdlog::logger> reporter;
    const uint32_t SIZE_MB = 50;
    const uint32_t ROTATION = 20;
    const string REPORT_PATH = "/var/log/ibofos/";
    const string REPORT_NAME = "report.log";
};

using ReporterSingleton = ibofos::Singleton<Reporter>;
} // namespace ibofos

inline ibofos::Logger*
logger()
{
    return ibofos::LoggerSingleton::Instance();
}

inline ibofos::Reporter*
reporter()
{
    return ibofos::ReporterSingleton::Instance();
}

#define IBOF_TRACE_DEBUG_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_INFO_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_WARN_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_ERROR_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_CRITICAL_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_DEBUG(eventid, ...) \
    logger()->Iboflog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_INFO(eventid, ...) \
    logger()->Iboflog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_WARN(eventid, ...) \
    logger()->Iboflog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_ERROR(eventid, ...) \
    logger()->Iboflog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_TRACE_CRITICAL(eventid, ...) \
    logger()->Iboflog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__)

#define IBOF_REPORT_TRACE(eventid, ...)                                                                        \
    {                                                                                                          \
        logger()->Iboflog(spdlog::source_loc{}, spdlog::level::trace, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                             \
            ->Iboflog(spdlog::source_loc{}, spdlog::level::trace, static_cast<int>(eventid), __VA_ARGS__);     \
    }

#define IBOF_REPORT_WARN(eventid, ...)                                                                        \
    {                                                                                                         \
        logger()->Iboflog(spdlog::source_loc{}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                            \
            ->Iboflog(spdlog::source_loc{}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__);     \
    }

#define IBOF_REPORT_ERROR(eventid, ...)                                                                      \
    {                                                                                                        \
        logger()->Iboflog(spdlog::source_loc{}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                           \
            ->Iboflog(spdlog::source_loc{}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__);     \
    }

#define IBOF_REPORT_CRITICAL(eventid, ...)                                                                        \
    {                                                                                                             \
        logger()->Iboflog(spdlog::source_loc{}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                                \
            ->Iboflog(spdlog::source_loc{}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__);     \
    }
#endif // LOGGER_H_
