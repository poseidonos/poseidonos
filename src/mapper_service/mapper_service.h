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

#include <array>
#include <string>
#include <unordered_map>

#include "src/include/array_mgmt_policy.h"
#include "src/lib/singleton.h"
#include "src/mapper/i_map_flush.h"
#include "src/mapper/i_mapper_wbt.h"
#include "src/mapper/i_reversemap.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"

namespace pos
{

class MapperService
{
    friend class Singleton<MapperService>;

public:
    MapperService(void);
    virtual ~MapperService(void) = default;

    void RegisterMapper(std::string arrayName, int arrayId,
        IVSAMap* iVSAMap, IStripeMap* iStripeMap, IReverseMap* iReverseMap,
        IMapFlush* iMapFlush, IMapperWbt* iMapperWbt);
    void UnregisterMapper(std::string arrayName);

    virtual IVSAMap* GetIVSAMap(std::string arrayName);
    IStripeMap* GetIStripeMap(std::string arrayName);
    IReverseMap* GetIReverseMap(std::string arrayName);
    IMapFlush* GetIMapFlush(std::string arrayName);
    IMapperWbt* GetIMapperWbt(std::string arrayName);

    virtual IVSAMap* GetIVSAMap(int arrayId);
    IStripeMap* GetIStripeMap(int arrayId);
    IReverseMap* GetIReverseMap(int arrayId);
    IMapFlush* GetIMapFlush(int arrayId);
    IMapperWbt* GetIMapperWbt(int arrayId);

private:
    std::unordered_map<std::string, int> arrayNameToId;

    std::array<IVSAMap*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iVSAMaps;
    std::array<IStripeMap*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iStripeMaps;
    std::array<IReverseMap*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iReverseMaps;
    std::array<IMapFlush*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iMapFlushes;
    std::array<IMapperWbt*, ArrayMgmtPolicy::MAX_ARRAY_CNT> iMapperWbts;
};

using MapperServiceSingleton = Singleton<MapperService>;

} // namespace pos
