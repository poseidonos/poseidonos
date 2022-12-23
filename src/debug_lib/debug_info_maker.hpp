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
#include <cassert>
#include <unistd.h>

#include "debug_info_maker.h"
#include "debug_info_queue.h"
namespace pos
{

template<typename T>
DebugInfoMaker<T>::DebugInfoMaker(void)
{
    run = true;
    debugInfoThread = new std::thread(&DebugInfoMaker<T>::_DebugInfoThread, this);
}

template<typename T>
DebugInfoMaker<T>::~DebugInfoMaker(void)
{
    run = false;
    if (nullptr != debugInfoThread)
    {
        debugInfoThread->join();
    }
    delete debugInfoThread;
}

template<typename T>
void
DebugInfoMaker<T>::RegisterDebugInfoMaker(T* obj, DebugInfoQueue<T>* queue)
{
    debugInfoObject = obj;
    debugInfoQueue = queue;
    AddDebugInfo();
}

template<typename T>
void
DebugInfoMaker<T>::AddDebugInfo(uint64_t userSpecific)
{
    MakeDebugInfo(*debugInfoObject);
    debugInfoQueue->AddDebugInfo(*debugInfoObject, userSpecific);
}

template<typename T>
void
DebugInfoMaker<T>::SetTimer(uint64_t inputTimerUsec)
{
    timerUsec = inputTimerUsec;
}

template<typename T>
void
DebugInfoMaker<T>::_DebugInfoThread(void)
{
    cpu_set_t cpuSet = AffinityManagerSingleton::Instance()->GetCpuSet(CoreType::GENERAL_USAGE);
    sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
    while(run)
    {
        AddDebugInfo(TIMER_TRIGGERED);
        usleep(timerUsec);
    }
}

}
