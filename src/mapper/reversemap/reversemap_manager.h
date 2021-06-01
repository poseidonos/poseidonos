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

#include "src/array_models/interface/i_array_info.h"
#include "src/mapper/i_reversemap.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper/address/mapper_address_info.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/meta_file_intf/meta_file_include.h"

namespace pos
{

class ReverseMapManager : public IReverseMap
{
public:
    ReverseMapManager(IVSAMap* ivsaMap, IStripeMap* istripeMap, IArrayInfo* iarrayInfo);
    virtual ~ReverseMapManager(void);

    void Init(MapperAddressInfo& info);
    void SetDoC(IArrayInfo* iarrayInfo);
    void Close(void);

    ReverseMapPack* GetReverseMapPack(StripeId wbLsid) override;
    ReverseMapPack* AllocReverseMapPack(bool gcDest) override;

    uint64_t GetReverseMapPerStripeFileSize(void);
    uint64_t GetWholeReverseMapFileSize(void);
    int LoadWholeReverseMap(char* pBuffer);
    int StoreWholeReverseMap(char* pBuffer);

private:
    int _SetPageSize(StorageOpt storageOpt = StorageOpt::DEFAULT);
    int _SetNumMpages(void);

    uint64_t mpageSize;          // Optimal page size for each FS (MFS, legacy)
    uint64_t numMpagesPerStripe; // It depends on block count per a stripe
    uint64_t fileSizePerStripe;
    uint64_t fileSizeWholeRevermap;

    ReverseMapPack* revMapPacks;
    MetaFileIntf* revMapWholefile;

    IVSAMap* iVSAMap;
    IStripeMap* iStripeMap;
    IArrayInfo* iArrayInfo;
};

} // namespace pos
