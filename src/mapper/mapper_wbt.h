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

#include "src/mapper/i_mapper_wbt.h"
#include "src/mapper/address/mapper_address_info.h"

#include "src/mapper/reversemap/reversemap_manager.h"
#include "src/mapper/stripemap/stripemap_manager.h"
#include "src/mapper/vsamap/vsamap_manager.h"

#include <string>

namespace pos
{
class MapperWbt : public IMapperWbt
{
public:
    MapperWbt(void) = default;
    MapperWbt(MapperAddressInfo* addrInfo_, VSAMapManager* vsaMapMgr, StripeMapManager* stripeMapMgr, ReverseMapManager* revMapMgr);
    virtual ~MapperWbt(void);

    int GetMapLayout(std::string fname) override;
    int ReadVsaMap(int volId, std::string fname) override;
    int ReadVsaMapEntry(int volId, BlkAddr rba, std::string fname) override;
    int WriteVsaMap(int volId, std::string fname) override;
    int WriteVsaMapEntry(int volId, BlkAddr rba, VirtualBlkAddr vsa) override;
    int ReadStripeMap(std::string fname) override;
    int ReadStripeMapEntry(StripeId vsid, std::string fname) override;
    int WriteStripeMap(std::string fname) override;
    int WriteStripeMapEntry(StripeId vsid, StripeLoc loc, StripeId lsid) override;
    int ReadReverseMap(StripeId vsid, std::string fname) override;
    int ReadWholeReverseMap(std::string fname) override;
    int ReadReverseMapEntry(StripeId vsid, BlkOffset offset, std::string fname) override;
    int WriteReverseMap(StripeId vsid, std::string fname) override;
    int WriteWholeReverseMap(std::string fname) override;
    int WriteReverseMapEntry(StripeId vsid, BlkOffset offset, BlkAddr rba, uint32_t volumeId) override;

private:
    VSAMapContent* _GetFirstValidVolume(void);
    int _LoadReverseMapVsidFromMFS(StripeId vsid);
    int _StoreReverseMapToMFS(StripeId vsid);

    MapperAddressInfo* addrInfo;
    VSAMapManager* vsaMapManager;
    StripeMapManager* stripeMapManager;
    ReverseMapManager* reverseMapManager;
};

} // namespace pos
