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

#include "src/mapper_service/mapper_service.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
MapperService::MapperService(void)
{
    iVSAMaps.fill(nullptr);
    iStripeMaps.fill(nullptr);
    iReverseMaps.fill(nullptr);
    iMapFlushes.fill(nullptr);
    iMapperWbts.fill(nullptr);
}

void
MapperService::RegisterMapper(std::string arrayName, int arrayId,
    IVSAMap* iVSAMap, IStripeMap* iStripeMap, IReverseMap* iReverseMap,
    IMapFlush* iMapFlush, IMapperWbt* iMapperWbt)
{
    if (arrayNameToId.find(arrayName) == arrayNameToId.end())
    {
        arrayNameToId.emplace(arrayName, arrayId);

        iVSAMaps[arrayId] = iVSAMap;
        iStripeMaps[arrayId] = iStripeMap;
        iReverseMaps[arrayId] = iReverseMap;
        iMapFlushes[arrayId] = iMapFlush;
        iMapperWbts[arrayId] = iMapperWbt;
    }
    else
    {
        POS_TRACE_ERROR(EID(MAPPER_ALREADY_EXIST), "Mapper for array {} is already registered", arrayName);
    }
}

void
MapperService::UnregisterMapper(std::string arrayName)
{
    if (arrayNameToId.find(arrayName) != arrayNameToId.end())
    {
        int arrayId = arrayNameToId[arrayName];
        arrayNameToId.erase(arrayName);

        iVSAMaps[arrayId] = nullptr;
        iStripeMaps[arrayId] = nullptr;
        iReverseMaps[arrayId] = nullptr;
        iMapFlushes[arrayId] = nullptr;
        iMapperWbts[arrayId] = nullptr;
    }
    else
    {
        POS_TRACE_INFO(EID(MAPPER_ALREADY_EXIST), "Mapper for array {} already unregistered", arrayName);
    }
}

IVSAMap*
MapperService::GetIVSAMap(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iVSAMaps[arrayId->second];
    }
}

IStripeMap*
MapperService::GetIStripeMap(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iStripeMaps[arrayId->second];
    }
}

IReverseMap*
MapperService::GetIReverseMap(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iReverseMaps[arrayId->second];
    }
}

IMapFlush*
MapperService::GetIMapFlush(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iMapFlushes[arrayId->second];
    }
}

IMapperWbt*
MapperService::GetIMapperWbt(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return iMapperWbts[arrayId->second];
    }
}

IVSAMap*
MapperService::GetIVSAMap(int arrayId)
{
    return iVSAMaps[arrayId];
}

IStripeMap*
MapperService::GetIStripeMap(int arrayId)
{
    return iStripeMaps[arrayId];
}

IReverseMap*
MapperService::GetIReverseMap(int arrayId)
{
    return iReverseMaps[arrayId];
}

IMapFlush*
MapperService::GetIMapFlush(int arrayId)
{
    return iMapFlushes[arrayId];
}

IMapperWbt*
MapperService::GetIMapperWbt(int arrayId)
{
    return iMapperWbts[arrayId];
}

} // namespace pos
