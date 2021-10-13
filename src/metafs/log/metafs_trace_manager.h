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

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

#include "metafs_spinlock.h"

namespace pos
{
class MetaFsTraceManager
{
public:
    void
    PrintArgs(void)
    {
    }
    template<typename T, typename... Args>
    void
    PrintArgs(const T& msg, const Args&... args)
    {
        std::cout << msg;
        PrintArgs(args...);
    }
    void
    PrintLineInfo(const char* fileName, const char* funcName, const int line)
    {
        std::cout << "[thread id=" << std::this_thread::get_id() << "] @" << fileName << ":" << funcName << ": Line:" << line << "> ";
    }
    template<typename... Args>
    void
    PrintBottomHalf(const char* fileName, const char* funcName, const int line, const Args&... args)
    {
        PrintLineInfo(fileName, funcName, line);
        PrintArgs(args...);
        std::cout << "\033[0m" << std::endl;
    }
    template<typename... Args>
    const std::string
    PrintCurrTime(void)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
    template<typename... Args>
    void
    Info(const char* fileName, const char* funcName, const int line, const Args&... args)
    {
        SPIN_LOCK_GUARD_IN_SCOPE(logSerialize);
        std::cout << PrintCurrTime() << " [Info]: ";
        PrintBottomHalf(fileName, funcName, line, args...);
    }
    template<typename... Args>
    void
    Debug(const char* fileName, const char* funcName, const int line, const Args&... args)
    {
        SPIN_LOCK_GUARD_IN_SCOPE(logSerialize);
        std::cout << "\033[1;32m" << PrintCurrTime() << " [Debug]: ";
        PrintBottomHalf(fileName, funcName, line, args...);
    }
    template<typename... Args>
    void
    Err(const char* fileName, const char* funcName, const int line, const Args&... args)
    {
        SPIN_LOCK_GUARD_IN_SCOPE(logSerialize);
        std::cout << "\033[1;33m" << PrintCurrTime() << " [Error]: ";
        PrintBottomHalf(fileName, funcName, line, args...);
    }
    template<typename... Args>
    void
    Fatal(const char* fileName, const char* funcName, const int line, const Args&... args)
    {
        SPIN_LOCK_GUARD_IN_SCOPE(logSerialize);
        std::cout << "\033[1;31m" << PrintCurrTime() << " [Fatal]: ";
        PrintBottomHalf(fileName, funcName, line, args...);
    }

private:
    MetaFsSpinLock logSerialize;
};
} // namespace pos
