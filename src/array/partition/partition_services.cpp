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

#include "partition_services.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
void PartitionServices::AddTranslator(PartitionType type, ITranslator* trans)
{
    if (translator.find(type) == translator.end())
    {
        POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG),
            "PartitionServices::AddTranslator, type:{}, trans:{}, size:{}",
            type, (void*)(trans), translator.size());
        translator.emplace(type, trans);
    }
}

void PartitionServices::AddRecover(PartitionType type, IRecover* recov)
{
    if (recover.find(type) == recover.end())
    {
        POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG),
            "PartitionServices::AddRebuildTarget, type:{}, recov:{}",
            type, (void*)(recov));
        recover.emplace(type, recov);
    }
}

void PartitionServices::AddRebuildTarget(RebuildTarget* tgt)
{
    POS_TRACE_INFO(EID(MOUNT_ARRAY_DEBUG_MSG),
            "PartitionServices::AddRebuildTarget, tgt:{}",
             (void*)(tgt));
    rebuildTargets.push_back(tgt);
}

void PartitionServices::Clear(void)
{
    POS_TRACE_INFO(EID(UNMOUNT_ARRAY_DEBUG_MSG), "PartitionServices::Clear");
    translator.clear();
    recover.clear();
    rebuildTargets.clear();
}

list<RebuildTarget*> PartitionServices::GetRebuildTargets(void)
{
    return rebuildTargets;
}

map<PartitionType, ITranslator*> PartitionServices::GetTranslator(void)
{
    return translator;
}

map<PartitionType, IRecover*> PartitionServices::GetRecover(void)
{
    return recover;
}
} // namespace pos
