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

#include <vector>

#include "src/array_models/interface/i_array_info.h"
#include "src/journal_manager/log_write/i_journal_volume_event_handler.h"
#include "src/mapper/i_mapper_volume_event_handler.h"
#include "src/sys_event/volume_event.h"

namespace pos
{
class Allocator;

class MetaVolumeEventHandler : public VolumeEvent
{
public:
    MetaVolumeEventHandler(IArrayInfo* arrayInfo,
        IMapperVolumeEventHandler* mapper,
        Allocator* allocator,
        IJournalVolumeEventHandler* journal);
    virtual ~MetaVolumeEventHandler(void);

    virtual int VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    virtual int VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) override;

private:
    IArrayInfo* arrayInfo;

    IMapperVolumeEventHandler* mapper;
    Allocator* allocator;
    IJournalVolumeEventHandler* journal;
};

} // namespace pos
