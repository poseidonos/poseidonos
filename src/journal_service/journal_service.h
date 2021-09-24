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

#include <array>
#include <string>
#include <unordered_map>

#include "src/include/array_mgmt_policy.h"
#include "src/journal_service/i_journal_manager.h"
#include "src/journal_service/i_journal_status_provider.h"
#include "src/journal_service/i_journal_writer.h"
#include "src/lib/singleton.h"

namespace pos
{
class JournalService
{
    friend class Singleton<JournalService>;

public:
    virtual bool IsEnabled(std::string arrayName);
    virtual bool IsEnabled(int arrayId);

    virtual void Register(std::string arrayName, int arrayId,
        IJournalManager* journal, IJournalWriter* writer,
        IJournalStatusProvider* provider);
    virtual void Unregister(std::string arrayName);

    virtual IJournalWriter* GetWriter(std::string arrayName);
    IJournalStatusProvider* GetStatusProvider(std::string arrayName);

    virtual IJournalWriter* GetWriter(int arrayId);
    IJournalStatusProvider* GetStatusProvider(int arrayId);

protected:
    JournalService(void);
    virtual ~JournalService(void);

private:
    std::array<IJournalManager*, ArrayMgmtPolicy::MAX_ARRAY_CNT> journalManagers;
    std::array<IJournalWriter*, ArrayMgmtPolicy::MAX_ARRAY_CNT> journalWriters;
    std::array<IJournalStatusProvider*, ArrayMgmtPolicy::MAX_ARRAY_CNT> statusProviders;

    std::unordered_map<std::string, int> arrayNameToId;
};

using JournalServiceSingleton = Singleton<JournalService>;

} // namespace pos
