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

#include <dirent.h>
#include <execinfo.h>
#include <sched.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>

#include "src/include/pos_event_id.h"
#include "src/lib/system_timeout_checker.h"
#include "src/logger/logger.h"
#define gettid() syscall(SYS_gettid)
#define tgkill(tgid, tid, sig) syscall(SYS_tgkill, tgid, tid, sig)

namespace pos
{
SignalHandler::SignalHandler(void)
{
    listUpdated = false;
    pendingThreads = 0;
}

void
SignalHandler::Register(void)
{
    signal(SIGINT, SignalHandler::INTHandler);
    signal(SIGSEGV, SignalHandler::ExceptionHandler);
    signal(SIGABRT, SignalHandler::ExceptionHandler);
    signal(SIGUSR1, SignalHandler::ExceptionHandler);
}

void
SignalHandler::Deregister(void)
{
    // We do not rollback INTHandler.
    signal(SIGSEGV, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGUSR1, SIG_DFL);
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
SignalHandler::_ExceptionHandler(int sig)
{
    const uint64_t WAIT_TIMEOUT_NS = 8000000000ULL;
    SystemTimeoutChecker* systemTimeoutChecker = nullptr;
    sigset_t oldset;
    // except sig, all other signal is masked.
    SignalMask::MaskSignal(sig, &oldset);
    if (listUpdated.exchange(true) == false)
    {
        dominantSignal = sig;
        systemTimeoutChecker = new SystemTimeoutChecker;
        systemTimeoutChecker->SetTimeout(WAIT_TIMEOUT_NS);
        _GetThreadIdList();
    }
    // if some threads is in USR1, the other thread gets SEGV
    // We trigger default signal handler immediately.
    // We just abandon backtrace to handle simply
    if (dominantSignal != sig)
    {
        if (sig == SIGSEGV || sig == SIGABRT)
        {
            signal(sig, SIG_DFL);
            raise(sig);
            // will not reach here
        }
        else
        {
            SignalMask::RestoreSignal(&oldset);
            return;
        }
    }
    _BacktraceAndInvokeNextThread(sig);
    if (systemTimeoutChecker != nullptr)
    {
        while (pendingThreads > 0 && systemTimeoutChecker->CheckTimeout() == false)
        {
            usleep(10);
        }
        switch (sig)
        {
            case SIGSEGV:
            {
                signal(SIGSEGV, SIG_DFL);
                raise(SIGSEGV);
                // will not reach here
                break;
            }
            case SIGABRT:
            {
                signal(SIGABRT, SIG_DFL);
                raise(SIGABRT);
                // will not reach here
                break;
            }
            case SIGUSR1:
            {
                break;
            }
            default:
            {
                signal(SIGABRT, SIG_DFL);
                raise(SIGABRT);
                // will not reach here
                break;
            }
        }
        pendingThreads = 0;
        delete systemTimeoutChecker;
        listUpdated = false;
    }
    SignalMask::RestoreSignal(&oldset);
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
        POS_TRACE_ERROR(POS_EVENT_ID::DEBUG_SIGNAL_HANDLING, "Cannot found directory!");
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
            POS_TRACE_ERROR(POS_EVENT_ID::DEBUG_SIGNAL_HANDLING, "signal is failed\n");
        }
        pendingThreads--;
    }
}

void
SignalHandler::_Backtrace(void)
{
    void* buffer[MAX_CALL_STACK];
    char** strings;
    int nptrs;
    nptrs = backtrace(buffer, MAX_CALL_STACK);

    strings = backtrace_symbols(buffer, nptrs);
    POS_TRACE_ERROR(POS_EVENT_ID::DEBUG_SIGNAL_HANDLING, "coreId : {}, tid : {}", sched_getcpu(), gettid());
    for (int index = 0; index < nptrs; index++)
    {
        POS_TRACE_ERROR(POS_EVENT_ID::DEBUG_SIGNAL_HANDLING, "{}", strings[index]);
    }
    POS_TRACE_ERROR(POS_EVENT_ID::DEBUG_SIGNAL_HANDLING, "================================================");
    POS_TRACE_ERROR(POS_EVENT_ID::DEBUG_SIGNAL_HANDLING, "                                                ");
}
} // namespace pos
