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

#include <string>
#include <unordered_map>

#include "src/include/array_mgmt_policy.h"
#include "src/lib/singleton.h"
#include "src/meta_service/i_meta_updater.h"
#include "src/journal_manager/i_journal_status_provider.h"

namespace pos
{
class MetaService
{
    friend class Singleton<MetaService>;

public:
    virtual void Register(std::string arrayName, int arrayId,
        IMetaUpdater* mapUpdater, IJournalStatusProvider* journalStatusProvider);
    virtual void Unregister(std::string arrayName);

    virtual IMetaUpdater* GetMetaUpdater(std::string arrayName);
    virtual IMetaUpdater* GetMetaUpdater(int arrayId);

    virtual IJournalStatusProvider* GetJournalStatusProvider(std::string arrayName);
    virtual IJournalStatusProvider* GetJournalStatusProvider(int arrayId);

protected:
    MetaService(void);
    virtual ~MetaService(void);

private:
    std::array<IMetaUpdater*, ArrayMgmtPolicy::MAX_ARRAY_CNT> metaUpdaters;
    std::array<IJournalStatusProvider*, ArrayMgmtPolicy::MAX_ARRAY_CNT> journalStatusProviders;
    std::unordered_map<std::string, int> arrayNameToId;
};

using MetaServiceSingleton = Singleton<MetaService>;
} // namespace pos
