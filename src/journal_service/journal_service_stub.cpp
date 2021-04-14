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
#include "journal_writer_stub.h"
#include "volume_event_handler_stub.h"
#include "journal_status_provider_stub.h"

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
    return false;
}

void
JournalService::Register(std::string arrayName, IJournalWriter* writer)
{
}

void
JournalService::Register(std::string arrayName, IVolumeEventHandler* writer)
{
}

void
JournalService::Register(std::string arrayName, IJournalStatusProvider* writer)
{
}

void
JournalService::Unregister(std::string arrayName)
{
}

IJournalWriter*
JournalService::GetWriter(std::string arrayName)
{
    // Note caller should delete the stub
    JournalWriterStub* stub = new JournalWriterStub();
    return stub;
}

IVolumeEventHandler*
JournalService::GetVolumeEventHandler(std::string arrayName)
{
    // Note caller should delete the stub
    VolumeEventHandlerStub* stub = new VolumeEventHandlerStub();
    return stub;
}

IJournalStatusProvider*
JournalService::GetStatusProvider(std::string arrayName)
{
    // Note caller should delete the stub
    JournalStatusProviderStub* stub = new JournalStatusProviderStub();
    return stub;
}

} // namespace pos
