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

#include "debug_info_maker.h"
#include "debug_info_maker.hpp"
#include "debug_info_queue.h"
#include "debug_info_queue.hpp"
#include <cassert>
#include <map>
#include <mutex>
#include <string>

namespace pos
{

std::map<std::string, DebugInfoInstance*> debugInfo;
std::mutex DebugInfoInstance::registeringMutex;
Document debugInfoDoc(kObjectType);
Document::AllocatorType& debugInfoAllocator = debugInfoDoc.GetAllocator();


DebugInfoInstance::DebugInfoInstance(void)
{
    instanceOkay = DebugInfoOkay::PASS;
    summaryOkay = DebugInfoOkay::PASS;
}

DebugInfoInstance::DebugInfoInstance(const DebugInfoInstance& debugInfoInstance)
: instanceOkay(debugInfoInstance.instanceOkay),
summaryOkay(debugInfoInstance.summaryOkay)
{
}


DebugInfoInstance::~DebugInfoInstance(void)
{
}

void
DebugInfoInstance::RegisterDebugInfoInstance(std::string str)
{
    std::lock_guard<std::mutex> lock(registeringMutex);
    debugInfo[str] = this;
}

void
DebugInfoInstance::DeRegisterDebugInfoInstance(std::string str)
{
    std::lock_guard<std::mutex> lock(registeringMutex);
    if (debugInfo.find(str) != debugInfo.end())
    {
        debugInfo.erase(str);
    }
}

DebugInfoOkay
DebugInfoInstance::IsOkay(void)
{
    return DebugInfoOkay::PASS;
}

}