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

#include "src/lib/singleton.h"
#include "src/mapper/i_map_flush.h"
#include "src/mapper/i_mapper_wbt.h"
#include "src/mapper/i_reversemap.h"
#include "src/mapper/i_stripemap.h"
#include "src/mapper/i_vsamap.h"
#include "src/mapper_service/mapper_interface_container.h"

#include <string>

namespace pos
{

class MapperService
{
    friend class Singleton<MapperService>;

public:
    MapperService(void) = default;
    virtual ~MapperService(void) = default;

    void RegisterMapper(std::string arrayName, IVSAMap* iVSAMap);
    void RegisterMapper(std::string arrayName, IStripeMap* iStripeMap);
    void RegisterMapper(std::string arrayName, IReverseMap* iReverseMap);
    void RegisterMapper(std::string arrayName, IMapFlush* iMapFlush);
    void RegisterMapper(std::string arrayName, IMapperWbt* iMapperWbt);
    void UnregisterMapper(std::string arrayName);

    virtual IVSAMap* GetIVSAMap(std::string arrayName);
    IStripeMap* GetIStripeMap(std::string arrayName);
    IReverseMap* GetIReverseMap(std::string arrayName);
    IMapFlush* GetIMapFlush(std::string arrayName);
    IMapperWbt* GetIMapperWbt(std::string arrayName);

private:
    MapperInterfaceContainer<IVSAMap> iVSAMaps;
    MapperInterfaceContainer<IStripeMap> iStripeMaps;
    MapperInterfaceContainer<IReverseMap> iReverseMaps;
    MapperInterfaceContainer<IMapFlush> iMapFlushes;
    MapperInterfaceContainer<IMapperWbt> iMapperWbts;
};

using MapperServiceSingleton = Singleton<MapperService>;

} // namespace pos
