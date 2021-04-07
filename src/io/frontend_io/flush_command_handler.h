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

#include "src/allocator/allocator.h"
#include "src/io/frontend_io/flush_command_manager.h"
#include "src/io/general_io/io_controller.h"
#include "src/io/general_io/volume_io.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event.h"
#include "src/volume/volume_manager.h"

namespace ibofos
{
#if defined NVMe_FLUSH_HANDLING
class Allocator;
class VolumeIo;
class VolumeManager;
class IOController;
class Mapper;

class FlushCmdHandler : public Event
{
public:
    FlushCmdHandler(VolumeIoSmartPtr volumeIo);
    virtual ~FlushCmdHandler(void);
    virtual bool Execute(void);
    void MapFlushCompleted(int mapId);
    void AllocatorMetaFlushCompleted();

private:
    Allocator* allocator;
    Mapper* mapper;
    VolumeIoSmartPtr volumeIo;
    int volumeId;
    FlushCmdManager* flushCmdManager;
    bool hasLock;
    std::atomic<int> totalMapsCompleted;
    int totalMapsToFlush;
    std::atomic<bool> mapperFlushComplete;
    std::atomic<bool> allocaterFlushComplete;
    std::mutex mapCountUpdateLock;
    int numVSAMapsForFlushing;
    int numStripeMapsForFlushing;
};

class MapFlushCompleteEvent : public Event
{
public:
    MapFlushCompleteEvent(int mapId, FlushCmdHandler* flushCmdHandler);
    virtual ~MapFlushCompleteEvent(void);
    virtual bool Execute(void);

private:
    int mapId;
    FlushCmdHandler* flushCmdHandler;
};

class AllocatorFlushDoneEvent : public Event
{
public:
    AllocatorFlushDoneEvent(FlushCmdHandler* flushCmdHandler);
    virtual ~AllocatorFlushDoneEvent(void);
    virtual bool Execute(void);

private:
    FlushCmdHandler* flushCmdHandler;
};

#endif
} // namespace ibofos
