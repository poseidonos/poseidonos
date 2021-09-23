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

#include "src/network/nvmf_target.h"
#include "src/network/nvmf_volume_pos.h"
namespace pos
{
class NvmfVolumePosSpy : public NvmfVolumePos
{
public:
    using NvmfVolumePos::NvmfVolumePos;
    NvmfVolumePosSpy(unvmf_io_handler ioHandler, EventFrameworkApi* eventFrameworkApi, SpdkCaller* spdkCaller, NvmfTarget* mockTarget)
    : NvmfVolumePos(ioHandler, eventFrameworkApi, spdkCaller, mockTarget)
    {
    }

    virtual ~NvmfVolumePosSpy(void) = default;

    void
    VolumeCreateHandler(void* arg1, void* arg2)
    {
        NvmfVolumePos::_VolumeCreateHandler(arg1, arg2);
    }

    void
    VolumeMountHandler(void* arg1, void* arg2)
    {
        NvmfVolumePos::_VolumeMountHandler(arg1, arg2);
    }

    void
    VolumeUnmountHandler(void* arg1, void* arg2)
    {
        NvmfVolumePos::_VolumeUnmountHandler(arg1, arg2);
    }

    void
    VolumeDeleteHandler(void* arg1, void* arg2)
    {
        NvmfVolumePos::_VolumeDeleteHandler(arg1, arg2);
    }

    void
    VolumeUpdateHandler(void* arg1, void* arg2)
    {
        NvmfVolumePos::_VolumeUpdateHandler(arg1, arg2);
    }

    void
    VolumeDetachHandler(void* arg1, void* arg2)
    {
        NvmfVolumePos::_VolumeDetachHandler(arg1, arg2);
    }

    void
    NamespaceDetachedHandler(void* cbArg, int status)
    {
        NvmfVolumePos::_NamespaceDetachedHandler(cbArg, status);
    }

    void
    NamespaceDetachedAllHandler(void* cbArg, int status)
    {
        NvmfVolumePos::_NamespaceDetachedAllHandler(cbArg, status);
    }

    void
    SetVolumeDetachedCount(uint32_t count)
    {
        volumeDetachedCnt = count;
    }
};
} // namespace pos
