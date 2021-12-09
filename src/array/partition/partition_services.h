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

#include "i_partition_services.h"
#include "src/include/partition_type.h"
#include "src/array/service/io_translator/i_translator.h"
#include "src/array/service/io_recover/i_recover.h"
#include "src/array/rebuild/rebuild_target.h"


#include <map>
#include <list>

using namespace std;

namespace pos
{

class PartitionServices : public IPartitionServices
{
public:
    virtual ~PartitionServices(void) = default;
    virtual void AddTranslator(PartitionType type, ITranslator* trans) override;
    virtual void AddRecover(PartitionType type, IRecover* recov) override;
    virtual void AddRebuildTarget(RebuildTarget* tgt) override;
    virtual void Clear(void) override;
    virtual map<PartitionType, ITranslator*> GetTranslator(void);
    virtual map<PartitionType, IRecover*> GetRecover(void);
    virtual list<RebuildTarget*> GetRebuildTargets(void);

private:
    map<PartitionType, ITranslator*> translator;
    map<PartitionType, IRecover*> recover;
    list<RebuildTarget*> rebuildTargets;
};

} // namespace pos
