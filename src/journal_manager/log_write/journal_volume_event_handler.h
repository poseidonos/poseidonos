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

#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "src/sys_event/volume_event.h"

namespace ibofos
{
class LogWriteContextFactory;
class DirtyMapManager;
class LogWriteHandler;
class JournalConfiguration;

class JournalVolumeEventHandler : public VolumeEvent
{
public:
    JournalVolumeEventHandler(void);
    virtual ~JournalVolumeEventHandler(void);

    void Init(LogWriteContextFactory* logFactory, DirtyMapManager* dirtyPages,
        LogWriteHandler* logWritter, JournalConfiguration* journalConfiguration);

    virtual bool VolumeCreated(std::string volName, int volID, uint64_t volSizeBytem, uint64_t maxiops, uint64_t maxbw);
    virtual bool VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw);
    virtual bool VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte);
    virtual bool VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw);
    virtual bool VolumeUnmounted(std::string volName, int volID);
    virtual bool VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw);
    void VolumeDetached(vector<int> volList) override;

    void VolumeDeletedLogWriteDone(int volumeId);

private:
    void _WaitForLogWriteDone(int volumeId);

    bool isInitialized;
    bool isJournalEnabled;

    JournalConfiguration* config;
    LogWriteContextFactory* logFactory;
    DirtyMapManager* dirtyPageManager;
    LogWriteHandler* logWriteHandler;

    std::mutex mutex;
    std::condition_variable cv;

    std::map<int, bool> volumeDeleteInProgress;
};

} // namespace ibofos
