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

#include "src/meta_service/meta_service.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
MetaService::MetaService(void)
{
    metaUpdaters.fill(nullptr);
    journalStatusProviders.fill(nullptr);
}

MetaService::~MetaService(void)
{
}

void
MetaService::Register(std::string arrayName, int arrayId, IMetaUpdater* mapUpdater, IJournalStatusProvider* journalStatus)
{
    if (arrayNameToId.find(arrayName) == arrayNameToId.end())
    {
        arrayNameToId.emplace(arrayName, arrayId);

        metaUpdaters[arrayId] = mapUpdater;
        journalStatusProviders[arrayId] = journalStatus;
    }
    else
    {
        POS_TRACE_INFO(EID(META_ALREADY_REGISTERED),
            "Meta service for array {} is already registered", arrayName);
    }
}

void
MetaService::Unregister(std::string arrayName)
{
    if (arrayNameToId.find(arrayName) != arrayNameToId.end())
    {
        int arrayId = arrayNameToId[arrayName];
        arrayNameToId.erase(arrayName);

        metaUpdaters[arrayId] = nullptr;
        journalStatusProviders[arrayId] = nullptr;
    }
    else
    {
        POS_TRACE_INFO(EID(META_ALREADY_REGISTERED),
            "Meta service for array {} already unregistered", arrayName);
    }
}

IMetaUpdater*
MetaService::GetMetaUpdater(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return metaUpdaters[arrayId->second];
    }
}

IMetaUpdater*
MetaService::GetMetaUpdater(int arrayId)
{
    return metaUpdaters[arrayId];
}

IJournalStatusProvider*
MetaService::GetJournalStatusProvider(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return journalStatusProviders[arrayId->second];
    }
}

IJournalStatusProvider*
MetaService::GetJournalStatusProvider(int arrayId)
{
    return journalStatusProviders[arrayId];
}

} // namespace pos
