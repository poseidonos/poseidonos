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
#include <vector>

#include "spdk/pos.h"
#include "src/array_models/interface/i_mount_sequence.h"
#include "src/network/nvmf_volume_pos.h"
#include "src/sys_event/volume_event.h"
#include "src/sys_event/volume_event_publisher.h"
using namespace std;

class NvmfVolumePos;

namespace pos
{
class Nvmf : public VolumeEvent, public IMountSequence
{
public:
    Nvmf(std::string arrayName, int arrayId);
    Nvmf(std::string arrayName, int arrayId, VolumeEventPublisher* volumeEventPublisher, NvmfVolumePos* inputNvmfVolume);
    virtual ~Nvmf(void);

    int Init(void) override;
    void Dispose(void) override;
    void Shutdown(void) override;
    void Flush(void) override;
    void SetuNVMfIOHandler(unvmf_io_handler handler);

    virtual int VolumeCreated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeDeleted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    int VolumeMounted(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeUnmounted(VolumeEventBase* volEventBase, VolumeArrayInfo* volArrayInfo) override;
    int VolumeLoaded(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeUpdated(VolumeEventBase* volEventBase, VolumeEventPerf* volEventPerf, VolumeArrayInfo* volArrayInfo) override;
    int VolumeDetached(vector<int> volList, VolumeArrayInfo* volArrayInfo) override;

private:
    NvmfVolumePos* volume;
    VolumeEventPublisher* volumeEventPublisher;
    const uint32_t MIB_IN_BYTE = 1024 * 1024;
    const uint32_t KIOPS = 1000;
    unvmf_io_handler ioHandler = {nullptr, nullptr};

    void _CopyVolumeInfo(char* destInfo, const char* srcInfo, int len);
    void _CopyVolumeEventBase(pos_volume_info* vInfo, VolumeEventBase* volEventBase);
    void _CopyVolumeEventPerf(pos_volume_info* vInfo, VolumeEventPerf* volEventPerf);
    void _CopyVolumeArrayInfo(pos_volume_info* vInfo, VolumeArrayInfo* volArrayInfo);
};

} // namespace pos
