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

#include "src/signal_handler/signal_handler.h"

#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>
#include <dirent.h>
#include <execinfo.h>
#include <sched.h>
#include <signal.h>
#include <sys/syscall.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "src/cli/cli_server.h"
#include "src/include/poseidonos_interface.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

#define gettid() syscall(SYS_gettid)
#define tgkill(tgid, tid, sig) syscall(SYS_tgkill, tgid, tid, sig)

namespace pos
{
SignalHandler::SignalHandler(void)
{
    char btLogPath[] = "/var/log/pos/pos_backtrace.log";
    pendingThreads = 0;
    btLogFilePtr = fopen(btLogPath, "a");
}

SignalHandler::~SignalHandler(void)
{
    pendingThreads = 0;
    if (btLogFilePtr != nullptr)
    {
        fclose(btLogFilePtr);
    }
}

void
SignalHandler::Register(void)
{
    signal(SIGINT, SignalHandler::INTHandler);
    signal(SIGSEGV, SignalHandler::ExceptionHandler);
    signal(SIGABRT, SignalHandler::ExceptionHandler);
    signal(SIGTERM, SignalHandler::ExceptionHandler);
    signal(SIGQUIT, SignalHandler::ExceptionHandler);
    signal(SIGUSR1, SignalHandler::ExceptionHandler);
}

void
SignalHandler::Deregister(void)
{
    // We do not rollback INTHandler.
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
}

void
SignalHandler::INTHandler(int sig)
{
    signal(sig, SIG_IGN);
    std::cout << "You cannot close FA server abruptly. Close it via client. "
                 "Ex: ./poseidonos-cli system stop\n";
}

void
SignalHandler::ExceptionHandler(int sig)
{
    SignalHandlerSingleton::Instance()->_ExceptionHandler(sig);
}

void
SignalHandler::_Log(std::string logMsg, bool printTimeStamp)
{
    if (btLogFilePtr != nullptr)
    {
        if (printTimeStamp)
        {
            // Get the current time
            time_t curTime = time(NULL);

            // Convert the current time
            struct tm local;

            localtime_r(&curTime, &local);
            if (btLogFilePtr)
            {
                fprintf(btLogFilePtr, "[%04d-%02d-%02d]%02d:%02d:%02d  ",
                    local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                    local.tm_hour, local.tm_min, local.tm_sec);
            }

            printf("[%04d-%02d-%02d]%02d:%02d:%02d  ",
                local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                local.tm_hour, local.tm_min, local.tm_sec);
        }
        if (btLogFilePtr)
        {
            fprintf(btLogFilePtr, "%s\n", logMsg.c_str());
        }
        printf("%s\n", logMsg.c_str());
    }
}

void
SignalHandler::_ShutdownProcess(void)
{
    PoseidonosInterface* interface = PoseidonosInterface::GetInterface();
    if (interface != nullptr)
    {
        interface->TriggerTerminate();
    }
}

void
SignalHandler::_ExceptionHandler(int sig)
{
    switch(sig)
    {
        case SIGTERM:
        case SIGQUIT:
        {
            sigset_t oldset;
            // This mask will not restored, this is irreversible.
            SignalMask::MaskQuitSignal(&oldset);
            std::lock_guard<std::mutex> lock(signalMutex);
            _Log("Quit Signal Handling!");
            _ShutdownProcess();
            break;
        }
        case SIGUSR1:
        {
            sigset_t oldset;
            SignalMask::MaskSignal(&oldset);
            {
                std::lock_guard<std::mutex> lock(signalMutex);
                _Log("May be stuck on the below backtrace ");
                _BacktraceAndInvokeNextThread(sig);
            }
            SignalMask::RestoreSignal(&oldset);
            break;
        }
        default:
        {
            sigset_t oldset;
            SignalMask::MaskSignal(&oldset);
            {
                std::lock_guard<std::mutex> lock(signalMutex);
                _Log("Signal Handling! num : " + std::to_string(sig));
                _BacktraceAndInvokeNextThread(sig);
                signal(sig, SIG_DFL);
                raise(sig);
            }
            SignalMask::RestoreSignal(&oldset);
        }
    }
}

void
SignalHandler::_GetThreadIdList(void)
{
    threadList.clear();
    DIR* dir;
    struct dirent* ent;
    if ((dir = opendir("/proc/self/task/")) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            // pid will not have "0", so we can distinguish error and proper pid.
            long tid = atol(ent->d_name);
            if (tid != ATOI_ERR && gettid() != tid)
            {
                threadList.push_back(tid);
                pendingThreads++;
            }
        }
        closedir(dir);
    }
    else
    {
        /* could not open directory */
        _Log("Cannot found directory!");
        return;
    }
}

void
SignalHandler::_BacktraceAndInvokeNextThread(int sig)
{
    _Backtrace();
    if (threadList.empty() == false)
    {
        long tid = threadList.front();
        threadList.pop_front();

        int ret = tgkill(getpid(), tid, sig);
        if (ret < 0)
        {
            _Log("signal is failed\n");
        }
        pendingThreads--;
    }
}

// This stacktrace mechanism will be supported from c++23
// So, we just use glibc implmentation.
void
SignalHandler::_Backtrace(void)
{
    const boost::stacktrace::stacktrace current_stacktrace;

    std::ostringstream oss;
    oss << "Stack trace:\n" << current_stacktrace;
    _Log(oss.str(), false);
    _Log("", false);
    fflush(btLogFilePtr);
}
} // namespace pos
