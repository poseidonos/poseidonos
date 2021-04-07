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

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "rebuild_progress.h"
#include "src/include/address_type.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"

using namespace std;

namespace ibofos
{
class StripeLocker;
class PartitionPhysicalSize;

using Restorer = function<int(UbioSmartPtr)>;
using F2PTranslator = function<PhysicalBlkAddr(const FtBlkAddr&)>;
using RebuildCompleteHandler = function<void()>;

enum class RebuildState
{
    NO_TARGET,
    READY,
    REBUILDING,
    PASS,
    CANCELLED,
    FAIL,
};

class RebuildContext
{
public:
    RebuildContext()
    {
    }
    virtual ~RebuildContext()
    {
    }
    string id;
    Restorer restore;
    F2PTranslator translate;
    const PartitionPhysicalSize* size = nullptr;
    uint32_t faultIdx = 0;
    atomic<uint32_t> taskCnt;
    RebuildProgress* prog = nullptr;
    RebuildCompleteHandler completeHandler;
    StripeLocker* locker = nullptr;
    RebuildState result = RebuildState::READY;
};
} // namespace ibofos
