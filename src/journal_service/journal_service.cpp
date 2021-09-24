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

#include "journal_service.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
JournalService::JournalService(void)
{
    journalManagers.fill(nullptr);
    journalWriters.fill(nullptr);
    statusProviders.fill(nullptr);
}

JournalService::~JournalService(void)
{
}

bool
JournalService::IsEnabled(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return false;
    }
    else
    {
        return journalManagers[arrayId->second]->IsEnabled();
    }
}

void
JournalService::Register(std::string arrayName, int arrayId,
    IJournalManager* journal, IJournalWriter* writer,
    IJournalStatusProvider* provider)
{
    if (arrayNameToId.find(arrayName) == arrayNameToId.end())
    {
        arrayNameToId.emplace(arrayName, arrayId);

        journalManagers[arrayId] = journal;
        journalWriters[arrayId] = writer;
        statusProviders[arrayId] = provider;
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::JOURNAL_ALREADY_EXIST,
            "Journal service for array {} is already registered", arrayName);
    }
}

void
JournalService::Unregister(std::string arrayName)
{
    if (arrayNameToId.find(arrayName) != arrayNameToId.end())
    {
        int arrayId = arrayNameToId[arrayName];
        arrayNameToId.erase(arrayName);

        journalManagers[arrayId] = nullptr;
        journalWriters[arrayId] = nullptr;
        statusProviders[arrayId] = nullptr;
    }
    else
    {
        POS_TRACE_INFO((int)POS_EVENT_ID::JOURNAL_ALREADY_EXIST,
            "Journal service for array {} already unregistered", arrayName);
    }
}

IJournalWriter*
JournalService::GetWriter(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return journalWriters[arrayId->second];
    }
}

IJournalStatusProvider*
JournalService::GetStatusProvider(std::string arrayName)
{
    auto arrayId = arrayNameToId.find(arrayName);
    if (arrayId == arrayNameToId.end())
    {
        return nullptr;
    }
    else
    {
        return statusProviders[arrayId->second];
    }
}

bool
JournalService::IsEnabled(int arrayId)
{
    if (journalWriters[arrayId] == nullptr)
    {
        return false;
    }
    else
    {
        return journalManagers[arrayId]->IsEnabled();
    }
}

IJournalWriter*
JournalService::GetWriter(int arrayId)
{
    return journalWriters[arrayId];
}

IJournalStatusProvider*
JournalService::GetStatusProvider(int arrayId)
{
    return statusProviders[arrayId];
}
} // namespace pos
