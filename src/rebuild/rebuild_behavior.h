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

#include <string>

#include "src/array/rebuild/rebuild_context.h"
#include "src/logger/logger.h"
#include "src/resource_manager/memory_manager.h"

namespace pos
{
class BufferPool;
class RebuildBehavior
{
public:
    RebuildBehavior(unique_ptr<RebuildContext> ctx,
        MemoryManager* mm = MemoryManagerSingleton::Instance());
    virtual ~RebuildBehavior(void);
    virtual void StopRebuilding(void);
    virtual RebuildContext* GetContext(void);
    virtual bool Init(void) = 0;
    virtual bool Read(void) = 0;
    virtual bool Write(uint32_t targetId, UbioSmartPtr ubio) = 0;
    virtual bool Complete(uint32_t targetId, UbioSmartPtr ubio) = 0;
    virtual void UpdateProgress(uint32_t val) = 0;

protected:
    bool _InitBuffers(void);
    bool _InitRecoverBuffers(string owner);
    bool _InitRebuildReadBuffers(string owner, int totalChunksToRead);
    int _GetTotalReadChunksForRecovery(void);
    virtual string _GetClassName(void) = 0;
    bool isInitialized = false;

    unique_ptr<RebuildContext> ctx = nullptr;
    MemoryManager* mm = nullptr;
    BufferPool* recoverBuffers = nullptr;
    BufferPool* rebuildReadBuffers = nullptr;
    static const int INIT_REBUILD_BUFFER_MAX_RETRY = 100;
    int initBufferRetryCnt = 0;
};
} // namespace pos
