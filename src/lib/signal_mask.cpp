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

#include "src/lib/signal_mask.h"

namespace pos
{

std::mutex SignalMask::signalMutex;

void
SignalMask::MaskSignal(int AllowedSignalNo, sigset_t* oldset)
{
    std::lock_guard<std::mutex> lock(signalMutex);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGSEGV);
    sigaddset(&set, SIGABRT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    sigdelset(&set, AllowedSignalNo);
    pthread_sigmask(SIG_BLOCK, &set, oldset);
}


void
SignalMask::MaskQuitSignal(sigset_t* oldset)
{
    std::lock_guard<std::mutex> lock(signalMutex);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &set, oldset);
}

void
SignalMask::RestoreSignal(sigset_t* oldset)
{
    std::lock_guard<std::mutex> lock(signalMutex);
    pthread_sigmask(SIG_SETMASK, oldset, nullptr);
}

void
SignalMask::MaskSignal(sigset_t* oldset)
{
    std::lock_guard<std::mutex> lock(signalMutex);
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGSEGV);
    sigaddset(&set, SIGABRT);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &set, oldset);
}

} // namespace pos

