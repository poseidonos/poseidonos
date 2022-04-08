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

#include <list>
#include <mutex>

#include "src/bio/flush_io.h"
#include "src/io/frontend_io/flush_configuration.h"
#include "src/lib/singleton.h"
#include "src/volume/volume_base.h"

namespace pos
{
class FlushCmdManager
{
public:
    FlushCmdManager(EventScheduler* eventScheduler = nullptr);
    virtual ~FlushCmdManager(void);
    virtual bool IsFlushEnabled(void);
    virtual bool CanFlushMeta(FlushIoSmartPtr flushIo);
    virtual void FinishMetaFlush(void);
    virtual void UpdateVSANewEntries(uint32_t volId, int arrayId);
    virtual bool IsInternalFlushEnabled(void);
    virtual int GetInternalFlushThreshold(void);
    virtual bool TrySetFlushInProgress(uint32_t volId);
    virtual void ResetFlushInProgress(uint32_t volId, bool isBackendFlush);

private:
    std::mutex metaFlushLock;
    std::mutex createAndExecFlushLock;
    std::atomic<bool> flushInProgress[MAX_VOLUME_COUNT];
    std::list<FlushIoSmartPtr> flushEvents;
    bool backendFlushInProgress[MAX_VOLUME_COUNT];
    bool metaFlushInProgress;
    FlushConfiguration config;
    EventScheduler* eventScheduler;
};

using FlushCmdManagerSingleton = Singleton<FlushCmdManager>;

} // namespace pos
