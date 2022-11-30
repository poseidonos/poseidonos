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

#include <bitset>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/array_models/interface/i_mount_sequence.h"
#include "src/include/address_type.h"
#include "src/lib/singleton.h"
#include "src/sys_event/volume_event.h"
#include "src/volume/volume_list.h"
#include "src/bio/volume_io.h"

namespace pos
{
class RBAStateService;
class VolumeEventPublisher;

enum class RBAOwnerType
{
    NoOwner,
    HOST,
    GC
};

class RBAStateManager : public VolumeEvent, public IMountSequence
{
public:
    explicit RBAStateManager(std::string arrayName, int arrayID);
    virtual ~RBAStateManager();

    virtual int Init(void) override;
    virtual void Dispose(void) override;
    virtual void Shutdown(void) override;
    virtual void Flush(void) override;

    virtual void CreateRBAState(uint32_t volumeID, uint64_t totalRBACount);
    virtual void DeleteRBAState(uint32_t volumeID);
    virtual VolumeIo::RbaList::iterator AcquireOwnershipRbaList(uint32_t volumeId,
        const VolumeIo::RbaList& uniqueRbaList, VolumeIo::RbaList::iterator startIter,
        uint32_t& acquiredCnt);
    virtual void ReleaseOwnershipRbaList(uint32_t volumeId,
        const VolumeIo::RbaList& uniqueRbaList);
    virtual bool BulkAcquireOwnership(uint32_t volumeID,
        BlkAddr startRba,
        uint32_t count);
    virtual void BulkReleaseOwnership(uint32_t volumeID,
        BlkAddr startRba,
        uint32_t count);
    virtual RBAOwnerType GetOwner(uint32_t volumeID, RbaAndSize rbaAndSize);
    int VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    int VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    int VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) override;

private:
    class RBAState
    {
    public:
        RBAState(void);
        bool AcquireOwnership(RBAOwnerType owner);
        void ReleaseOwnership(void);
        RBAOwnerType GetOwner(void) { return owner; }

    private:
        std::atomic_flag ownered;
        RBAOwnerType owner = RBAOwnerType::NoOwner;
    };

    class RBAStatesInVolume
    {
    public:
        RBAStatesInVolume(void);
        bool AcquireOwnership(BlkAddr startRba, uint32_t cnt, RBAOwnerType owner);
        void ReleaseOwnership(BlkAddr startRba, uint32_t cnt);
        RBAOwnerType GetOwner(BlkAddr rba);
        void SetSize(uint64_t newSize);

    private:
        bool _IsAccessibleRba(BlkAddr endRba);
        RBAState* rbaStates;
        uint64_t size;
    };
    using RBAStatesInArray = std::array<RBAStatesInVolume, MAX_VOLUME_COUNT>;

    RBAStatesInArray rbaStatesInArray;

    bool _AcquireOwnership(uint32_t volumeID, BlkAddr startRba, uint32_t count, RBAOwnerType owner = RBAOwnerType::HOST);
    void _ReleaseOwnership(uint32_t volumeID, BlkAddr startRba, uint32_t count);
};

} // namespace pos
