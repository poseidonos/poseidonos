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
#include <atomic>
#include <string>
#include <vector>

#include "spdk/pos.h"
#include "src/include/nvmf_const.h"
#include "src/network/nvmf.h"
#include "src/network/nvmf_target.h"
#include "src/spdk_wrapper/event_framework_api.h"

using namespace std;
namespace pos
{
struct volumeListInfo
{
    string arrayName;
    string subnqn;
    vector<int> vols;
};

class NvmfVolumePos
{
public:
    explicit NvmfVolumePos(unvmf_io_handler ioHandler);
    NvmfVolumePos(unvmf_io_handler ioHandler, EventFrameworkApi* eventFrameworkApi, SpdkCaller* spdkCaller, NvmfTarget* nvmfTarget);
    virtual ~NvmfVolumePos(void);

    virtual bool VolumeCreated(struct pos_volume_info* info, uint64_t time = NS_CREATE_TIMEOUT);
    virtual bool VolumeLoaded(struct pos_volume_info* info);
    virtual bool VolumeDeleted(struct pos_volume_info* info, uint64_t time = NS_DELETE_TIMEOUT);
    virtual void VolumeMounted(struct pos_volume_info* info);
    virtual bool VolumeUnmounted(struct pos_volume_info* info, uint64_t time = NS_DETACH_TIMEOUT);
    virtual void VolumeUpdated(struct pos_volume_info* info);
    virtual bool VolumeDetached(vector<int>& volList, string arrayName, uint64_t time = NS_DETACH_TIMEOUT);

protected:
    static atomic<uint32_t> volumeDetachedCnt;
    unvmf_io_handler ioHandler;
    EventFrameworkApi* eventFrameworkApi;
    SpdkCaller* spdkCaller;
    static NvmfTarget* target;
    uint32_t VOLUME_UNMOUNT_CNT = 1;

    static void _VolumeDetachHandler(void* arg1, void* arg2);
    static void _VolumeCreateHandler(void* arg1, void* arg2);
    static void _VolumeMountHandler(void* arg1, void* arg2);
    static void _VolumeUnmountHandler(void* arg1, void* arg2);
    static void _VolumeDeleteHandler(void* arg1, void* arg2);
    static void _VolumeUpdateHandler(void* arg1, void* arg2);
    static void _NamespaceDetachedHandler(void* cbArg, int status);
    static void _NamespaceDetachedAllHandler(void* cbArg, int status);

    bool _WaitVolumeCreated(uint32_t volId, string arrayName, uint64_t time);
    bool _WaitVolumeDeleted(uint32_t volId, string arrayName, uint64_t time);
    bool _WaitVolumeDetached(uint32_t volCnt, uint64_t time);
private:
    static atomic<bool> detachFailed;
};

} // namespace pos
