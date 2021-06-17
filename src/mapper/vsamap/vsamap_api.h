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

#include "src/mapper/i_vsamap.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/vsamap/i_vsamap_internal.h"
#include "src/mapper/vsamap/vsamap_content.h"
#include "src/volume/volume_list.h"

namespace pos
{

class VSAMapAPI : public IVSAMap
{
public:
    VSAMapAPI(IVSAMapInternal* vsaMapInt, MapperAddressInfo* info);
    virtual ~VSAMapAPI(void);

    void EnableVsaMapAccess(int volID);
    void DisableVsaMapAccess(int volID);
    VSAMapContent*& GetVSAMapContent(int volID);

    int GetVSAs(int volumeId, BlkAddr startRba, uint32_t numBlks, VsaArray& vsaArray) override;
    int SetVSAs(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks) override;
    VirtualBlkAddr GetVSAInternal(int volumeId, BlkAddr rba, int& caller) override;
    int SetVSAsInternal(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks) override;
    VirtualBlkAddr GetRandomVSA(BlkAddr rba);
    MpageList GetDirtyVsaMapPages(int volumeId, BlkAddr startRba, uint64_t numBlks) override;
    int64_t GetNumUsedBlocks(int volId) override;

    void EnableVsaMapInternalAccess(int volId);
    void DisableVsaMapInternalAccess(int volId);

private:
    int _UpdateVsaMap(int volumeId, BlkAddr startRba, VirtualBlks& virtualBlks);
    VirtualBlkAddr _ReadVSA(int volumeId, BlkAddr rba);

    bool _IsVSAMapLoaded(int volID);
    bool _IsVsaMapAccessible(int volID);

    VSAMapContent* vsaMaps[MAX_VOLUME_COUNT];
    std::atomic<bool> isVsaMapAccessable[MAX_VOLUME_COUNT];
    std::atomic<bool> isVsaMapInternalAccessable[MAX_VOLUME_COUNT];

    IVSAMapInternal* iVSAMapInternal;
    MapperAddressInfo* addrInfo;
};

} // namespace pos
