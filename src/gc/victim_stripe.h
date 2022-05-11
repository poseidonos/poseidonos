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
#include <list>

#include "src/array_models/interface/i_array_info.h"
#include "src/mapper/i_reversemap.h"
#include "src/mapper/reversemap/reverse_map.h"
#include "src/volume/i_volume_io_manager.h"

using namespace std;

namespace pos
{
struct BlkInfo
{
    BlkAddr rba;
    uint32_t volID;
    VirtualBlkAddr vsa;
};

class ReverseMapPack;
class IVSAMap;
class IStripeMap;
class IVolumeInfoManager;

class VictimStripe
{
public:
    explicit VictimStripe(IArrayInfo* array);
    VictimStripe(IArrayInfo* array,
                IReverseMap* inputRevMap,
                IVSAMap* inputIVSAMap,
                IStripeMap* inputIStripeMap,
                IVolumeIoManager* inputVolumeManager);

    virtual ~VictimStripe(void);
    virtual void Load(StripeId _lsid, CallbackSmartPtr callback);

    virtual list<BlkInfo>&
    GetBlkInfoList(uint32_t index)
    {
        return validBlkInfos[index];
    };

    virtual uint32_t
    GetBlkInfoListSize(void)
    {
        return validBlkInfos.size();
    };

    virtual bool LoadValidBlock(void);

private:
    void _InitValue(StripeId _lsid);
    void _LoadReverseMap(CallbackSmartPtr callback);

    StripeId myLsid;
    vector<list<BlkInfo>> validBlkInfos;
    list<BlkInfo> blkInfoList;

    uint32_t dataBlks;
    uint32_t chunkIndex;
    uint32_t blockOffset;
    uint32_t validBlockCnt;
    bool isLoaded;

    IArrayInfo* array;
    IReverseMap* iReverseMap;
    IVSAMap* iVSAMap;
    IStripeMap* iStripeMap;
    IVolumeIoManager* volumeManager;
    ReverseMapPack* revMapPack;
};

} // namespace pos
