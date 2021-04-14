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

#include <string>

#include "i_journal_writer.h"
#include "i_volume_event.h"
#include "i_journal_status_provider.h"

#include "journal_service_list.h"
#include "src/lib/singleton.h"

namespace pos
{
class JournalService
{
    friend class Singleton<JournalService>;

public:
    virtual bool IsEnabled(std::string arrayName);

    virtual void Register(std::string arrayName, IJournalWriter* writer);
    void Register(std::string arrayName, IVolumeEventHandler* handler);
    void Register(std::string arrayName, IJournalStatusProvider* provider);
    virtual void Unregister(std::string arrayName);

    virtual IJournalWriter* GetWriter(std::string arrayName);
    IVolumeEventHandler* GetVolumeEventHandler(std::string arrayName);
    IJournalStatusProvider* GetStatusProvider(std::string arrayName);

protected:
    JournalService(void);
    virtual ~JournalService(void);

private:
    JournalServiceList<IJournalWriter> journalWriters;
    JournalServiceList<IVolumeEventHandler> volEventHandlers;
    JournalServiceList<IJournalStatusProvider> statusProvider;
};

using JournalServiceSingleton = Singleton<JournalService>;

} // namespace pos
