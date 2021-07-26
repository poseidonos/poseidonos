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

#include <bitset>
#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/include/address_type.h"
#include "src/lib/singleton.h"
#include "src/sys_event/volume_event.h"
#include "src/volume/volume_list.h"
#include "src/bio/volume_io.h"

namespace pos
{
class RBAStateService;
class VolumeEventPublisher;

class RBAStateManager : public VolumeEvent
{
public:
    explicit RBAStateManager(std::string arrayName, int arrayID);
    virtual ~RBAStateManager();

    virtual void CreateRBAState(uint32_t volumeID, uint64_t totalRBACount);
    virtual void DeleteRBAState(uint32_t volumeID);
    virtual bool AcquireOwnershipRbaList(uint32_t volumeId,
        const VolumeIo::RbaList& sectorRbaList);
    virtual void ReleaseOwnershipRbaList(uint32_t volumeId,
        const VolumeIo::RbaList& sectorRbaList);
    virtual bool BulkAcquireOwnership(uint32_t volumeID,
        BlkAddr startRba,
        uint32_t count);
    virtual void BulkReleaseOwnership(uint32_t volumeID,
        BlkAddr startRba,
        uint32_t count);

    bool VolumeCreated(std::string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    bool VolumeDeleted(std::string volName, int volID, uint64_t volSizeByte, std::string arrayName, int arrayID) override;
    bool VolumeMounted(std::string volName, std::string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    bool VolumeUnmounted(std::string volName, int volID, std::string arrayName, int arrayID) override;
    bool VolumeLoaded(std::string name, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    bool VolumeUpdated(std::string volName, int volID, uint64_t maxiops, uint64_t maxbw, std::string arrayName, int arrayID) override;
    void VolumeDetached(vector<int> volList, std::string arrayName, int arrayID) override;

    std::string GetArrayName(void);

private:
    class RBAState
    {
    public:
        RBAState(void);
        bool AcquireOwnership(void);
        void ReleaseOwnership(void);

    private:
        std::atomic_flag ownered;
    };

    class RBAStatesInVolume
    {
    public:
        RBAStatesInVolume(void);
        bool AcquireOwnership(BlkAddr startRba, uint32_t cnt);
        void ReleaseOwnership(BlkAddr startRba, uint32_t cnt);
        void SetSize(uint64_t newSize);

    private:
        bool _IsAccessibleRba(BlkAddr endRba);
        RBAState* rbaStates;
        uint64_t size;
    };
    using RBAStatesInArray = std::array<RBAStatesInVolume, MAX_VOLUME_COUNT>;

    RBAStatesInArray rbaStatesInArray;

    bool _AcquireOwnership(uint32_t volumeID, BlkAddr startRba, uint32_t count);
    void _ReleaseOwnership(uint32_t volumeID, BlkAddr startRba, uint32_t count);
};

} // namespace pos
