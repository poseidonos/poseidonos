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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <yaml-cpp/yaml.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

#include "preferences.h"
#include "spdlog/spdlog.h"
#include "src/cli/cli_event_code.h"
#include "src/dump/dump_module.h"
#include "src/dump/dump_module.hpp"
#include "src/event/event_manager.h"
#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/lib/signal_mask.h"
#include "src/lib/singleton.h"

#define POS_EVENT_FILE_PATH "/etc/pos/pos_event.yaml"

using namespace std;

namespace pos
{
enum class ModuleInDebugLogDump
{
    IO_FLUSH,
    IO_GENERAL,
    CALLBACK_TIMEOUT,
    FLUSH_CMD,
    JOURNAL,
    META,
    MAX_SIZE,
};

string
BuildPattern(bool logJson);

class Logger
{
public:
    // LCOV_EXCL_START
    Logger();
    ~Logger();

    template<typename... Args>
    void
    IboflogWithDump(ModuleInDebugLogDump module,
        spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int id, spdlog::string_view_t fmt, const Args&... args)
    {
#ifndef POS_UT_SUPPRESS_LOGMSG
        if (ShouldLog(lvl, id))
        {
            uint32_t moduleId = static_cast<uint32_t>(module);
            fmt::memory_buffer buf;
            fmt::format_to(buf, fmt, args...);
            DumpModule<DumpBuffer>* dumpModulePtr = dumpModule[moduleId];
            DumpBuffer dumpBuffer(buf.data(), buf.size(), dumpModulePtr);
            dumpModulePtr->AddDump(dumpBuffer, 0);
        }
#endif
    }

    template<typename... Args>
    void
    Poslog(spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int eventId, spdlog::string_view_t fmt, const Args&... args)
    {
#ifndef POS_UT_SUPPRESS_LOGMSG
        if (ShouldLog(lvl, eventId))
        {
            auto event_info = eventManager.GetEventInfo();
            auto it = event_info->find(eventId);
            try
            {
                if (it == event_info->end())
                {
                    // TODO (mj): currently, we print raw message
                    // when there is no information about the event in PosEventInfo.
                    // A method is required to enforce to add event information to
                    // PoSEventInfo.(e.g., invoking a compile error if eventId does not
                    // match with PosEventInfo)
                    logger->iboflog_sink(loc, lvl, eventId,
                        fmt::format(
                            preferences.IsStrLoggingEnabled() ? "\"event_name:\":\"\",\"message\":\"{}\",\"cause\":\"\",\"solution\":\"\",\"variables\":\"\"" : "\tN/A - {} (cause: N/A, solution: N/A, variables: N/A)",
                            fmt),
                        args...);
                }
                else
                {
                    logger->iboflog_sink(loc, lvl, eventId,
                        fmt::format(
                            preferences.IsStrLoggingEnabled() ? "\"event_name:\":\"{}\",\"message\":\"{}\",\"cause\":\"{}\",\"solution\":\"{}\",\"variables\":\"{}\"" : "\t{} - {} (cause: {}, solution: {}, variables: {})",
                            it->second.GetEventName(), it->second.GetMessage(),
                            it->second.GetCause(), it->second.GetSolution(),
                            fmt),
                        args...);
                }
            }
            catch (const std::exception& e)
            {
                try
                {
                    logger->iboflog_sink(loc, lvl, eventId, fmt::format("\"exception\":\"{}\",\"message\":\"{}\"", e.what(), fmt), args...);
                }
                catch (const std::exception& e)
                {
                    logger->iboflog_sink(loc, lvl, eventId, "expection has occured while parsing the event log: " + std::string(e.what()));
                }
            }
        }
#endif
    }

    void ApplyPreference(void);

    int SetLevel(string lvl);
    string GetLevel(void);

    int SetStrLogging(bool input); // Note (mj): need ApplyPreference to apply the JSON setting.
    bool IsStrLoggingEnabled(void)
    {
        return preferences.IsStrLoggingEnabled();
    }

    int
    ApplyFilter()
    {
        return preferences.ApplyFilter();
    }

    pos_logger::Preferences
    GetPreferenceInstance()
    {
        return preferences;
    }

    JsonElement
    GetPreference()
    {
        return preferences.ToJson();
    }

    string
    GetLogDir()
    {
        return preferences.LogDir();
    }

    bool ShouldLog(spdlog::level::level_enum lvl, int id)
    {
        return preferences.ShouldLog(lvl, id);
    }

private:
    const uint32_t MAX_LOGGER_DUMP_SIZE = 1 * 1024 * 1024;
    const uint32_t AVG_LINE = 80;
    DumpModule<DumpBuffer>* dumpModule[static_cast<uint32_t>(ModuleInDebugLogDump::MAX_SIZE)];
    shared_ptr<spdlog::logger> logger;
    pos_logger::Preferences preferences;
    // LCOV_EXCL_STOP
};

using LoggerSingleton = pos::Singleton<Logger>;

class Reporter
{
public:
    Reporter();
    template<typename... Args>
    void
    Poslog(spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int id, spdlog::string_view_t fmt, const Args&... args)
    {
#ifndef POS_UT_SUPPRESS_LOGMSG
        reporter->iboflog_sink(loc, lvl, id, fmt, args...);
#endif
    }

private:
    shared_ptr<spdlog::logger> reporter;
    pos_logger::Preferences preferences;
    const uint32_t SIZE_MB = 50;
    const uint32_t ROTATION = 20;
    const string REPORT_PATH = "/var/log/pos/";
    const string REPORT_NAME = "report.log";
};

using ReporterSingleton = pos::Singleton<Reporter>;
} // namespace pos

inline pos::Logger*
logger()
{
    return pos::LoggerSingleton::Instance();
}

inline pos::Reporter*
reporter()
{
    return pos::ReporterSingleton::Instance();
}

#define POS_TRACE_DEBUG_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_INFO_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_TRACE_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::trace, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_WARN_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_ERROR_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_CRITICAL_IN_MEMORY(dumpmodule, eventid, ...) \
    logger()->IboflogWithDump(dumpmodule, spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_DEBUG(eventid, ...) \
    logger()->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_INFO(eventid, ...) \
    logger()->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_TRACE(eventid, ...) \
    logger()->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::trace, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_WARN(eventid, ...) \
    logger()->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_ERROR(eventid, ...) \
    logger()->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__)

#define POS_TRACE_CRITICAL(eventid, ...) \
    logger()->Poslog(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__)

#define POS_REPORT_TRACE(eventid, ...)                                                                        \
    {                                                                                                         \
        logger()->Poslog(spdlog::source_loc{}, spdlog::level::trace, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                            \
            ->Poslog(spdlog::source_loc{}, spdlog::level::trace, static_cast<int>(eventid), __VA_ARGS__);     \
    }

#define POS_REPORT_WARN(eventid, ...)                                                                        \
    {                                                                                                        \
        logger()->Poslog(spdlog::source_loc{}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                           \
            ->Poslog(spdlog::source_loc{}, spdlog::level::warn, static_cast<int>(eventid), __VA_ARGS__);     \
    }

#define POS_REPORT_ERROR(eventid, ...)                                                                      \
    {                                                                                                       \
        logger()->Poslog(spdlog::source_loc{}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                          \
            ->Poslog(spdlog::source_loc{}, spdlog::level::err, static_cast<int>(eventid), __VA_ARGS__);     \
    }

#define POS_REPORT_CRITICAL(eventid, ...)                                                                        \
    {                                                                                                            \
        logger()->Poslog(spdlog::source_loc{}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__); \
        reporter()                                                                                               \
            ->Poslog(spdlog::source_loc{}, spdlog::level::critical, static_cast<int>(eventid), __VA_ARGS__);     \
    }

namespace pos
{
template<typename StateT>
class ChangeLogger
{
public:
    ChangeLogger(void)
    : ChangeLogger(logger(), StateT())
    {
    }
    ChangeLogger(Logger* logger, StateT initState)
    : logger_(logger),
      prev_(initState),
      count_(0),
      logging_(false)
    {
    }
    virtual ~ChangeLogger(void)
    {
    }
    virtual void LoggingStateChangeConditionally(spdlog::source_loc loc, spdlog::level::level_enum lvl,
        int id, StateT currentState, std::string msg)
    {
        logging_ = false;
        if (prev_ == currentState)
        {
            count_++;
            return;
        }

        if (logger_)
        {
            logger_->Poslog(loc, lvl, id, msg + " current_state:{}, prev_state:{}, prev_state_repeat_count:{} times", prev_, currentState, count_);
            logging_ = true;
        }
        count_ = 0;
        prev_ = currentState;
    }
    virtual uint64_t GetCount(void)
    {
        return count_;
    }
    virtual bool IsLoggingStateChanged(void)
    {
        return logging_;
    }

private:
    pos::Logger* logger_;
    StateT prev_;
    std::atomic<uint64_t> count_;
    bool logging_;
};
} // namespace pos

#define POS_TRACE_DEBUG_CONDITIONALLY(instance, id, currentState, msg) \
    (instance)->LoggingStateChangeConditionally(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::debug, id, currentState, msg)
#define POS_TRACE_WARN_CONDITIONALLY(instance, id, currentState, msg) \
    (instance)->LoggingStateChangeConditionally(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::warn, id, currentState, msg)
#define POS_TRACE_TRACE_CONDITIONALLY(instance, id, currentState, msg) \
    (instance)->LoggingStateChangeConditionally(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::trace, id, currentState, msg)
#define POS_TRACE_INFO_CONDITIONALLY(instance, id, currentState, msg) \
    (instance)->LoggingStateChangeConditionally(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::info, id, currentState, msg)
#define POS_TRACE_ERROR_CONDITIONALLY(instance, id, currentState, msg) \
    (instance)->LoggingStateChangeConditionally(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::err, id, currentState, msg)
#define POS_TRACE_CRITICAL_CONDITIONALLY(instance, id, currentState, msg) \
    (instance)->LoggingStateChangeConditionally(spdlog::source_loc{__FILE__, __LINE__, __FUNCTION__}, spdlog::level::critical, id, currentState, msg)

#endif // LOGGER_H_
