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

#include "src/array_models/interface/i_array_info.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/state/interface/i_state_control.h"

namespace pos
{
class Mapper;
class Allocator;
class JournalManager;
class MetaFsFileControlApi;

class MetaUpdater;
class SegmentContextUpdater;
class MetaEventFactory;
class MetaVolumeEventHandler;
class MetaService;

class Metadata : public IMountSequence
{
public:
    Metadata(IArrayInfo* info, IStateControl* state);
    Metadata(IArrayInfo* info, Mapper* mapper, Allocator* allocator, JournalManager* jouranl, MetaFsFileControlApi* metaFsCtrl, MetaService* service);
    virtual ~Metadata(void);

    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;

    virtual bool NeedRebuildAgain(void);
    virtual int PrepareRebuild(void);
    virtual void StopRebuilding(void);

private:
    IArrayInfo* arrayInfo;
    Mapper* mapper;
    Allocator* allocator;
    JournalManager* journal;
    MetaFsFileControlApi* metaFsCtrl;
    MetaVolumeEventHandler* volumeEventHandler;
    MetaService* metaService;

    MetaUpdater* metaUpdater;
    SegmentContextUpdater* segmentContextUpdater;
    MetaEventFactory* metaEventFactory;
};
} // namespace pos
