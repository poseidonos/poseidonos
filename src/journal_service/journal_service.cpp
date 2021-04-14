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
}

JournalService::~JournalService(void)
{
}

bool
JournalService::IsEnabled(std::string arrayName)
{
    IJournalWriter* journal = journalWriters.Find(arrayName);
    if (journal == nullptr)
    {
        return false;
    }
    else
    {
        return journal->IsEnabled();
    }
}

void
JournalService::Register(std::string arrayName, IJournalWriter* writer)
{
    journalWriters.Register(arrayName, writer);
}

void
JournalService::Register(std::string arrayName, IVolumeEventHandler* handler)
{
    volEventHandlers.Register(arrayName, handler);
}

void
JournalService::Register(std::string arrayName, IJournalStatusProvider* provider)
{
    statusProvider.Register(arrayName, provider);
}

void
JournalService::Unregister(std::string arrayName)
{
    journalWriters.Unregister(arrayName);
    volEventHandlers.Unregister(arrayName);
    statusProvider.Unregister(arrayName);
}

IJournalWriter*
JournalService::GetWriter(std::string arrayName)
{
    return journalWriters.Find(arrayName);
}

IVolumeEventHandler*
JournalService::GetVolumeEventHandler(std::string arrayName)
{
    return volEventHandlers.Find(arrayName);
}

IJournalStatusProvider*
JournalService::GetStatusProvider(std::string arrayName)
{
    return statusProvider.Find(arrayName);
}

} // namespace pos
