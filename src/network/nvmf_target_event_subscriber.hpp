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
#include <string>
#include <vector>

#include "src/network/nvmf_volume.hpp"
#include "src/sys_event/volume_event.h"
using namespace std;

class NvmfVolume;

namespace pos
{
class NvmfTargetEventSubscriber : public VolumeEvent
{
public:
    NvmfTargetEventSubscriber(NvmfVolume* vol, std::string arrayName);
    ~NvmfTargetEventSubscriber(void);

    bool VolumeCreated(string volName, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, string arrayName) override;
    bool VolumeDeleted(string volName, int volID, uint64_t volSizeByte, string arrayName) override;
    bool VolumeMounted(string volName, string subnqn, int volID, uint64_t volSizeByte, uint64_t maxiops, uint64_t maxbw, string arrayName) override;
    bool VolumeUnmounted(string volName, int volID, string arrayName) override;
    bool VolumeLoaded(string volName, int id, uint64_t totalSize, uint64_t maxiops, uint64_t maxbw, string arrayName) override;
    bool VolumeUpdated(string volName, int volID, uint64_t maxiops, uint64_t maxbw, string arrayName) override;
    void VolumeDetached(vector<int> volList, string arrayName) override;

    void CopyVolumeInfo(char* destInfo, const char* srcInfo, int len);

private:
    NvmfVolume* volume;
    std::string arrayName;
    const uint32_t MIB_IN_BYTE = 1024 * 1024;
    const uint32_t KIOPS = 1000;
};

} // namespace pos
