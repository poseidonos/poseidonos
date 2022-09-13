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

#include "nvm_partition_builder.h"
#include "src/array/partition/nvm_partition.h"
#include "src/include/pos_event_id.h"
#include "src/include/array_config.h"
#include "src/logger/logger.h"

namespace pos
{

int
NvmPartitionBuilder::Build(uint64_t startLba, Partitions& out)
{
    vector<ArrayDevice*> nvm {option.nvm};
    NvmPartition* impl = new NvmPartition(option.partitionType, nvm);
    POS_TRACE_INFO(EID(CREATE_ARRAY_DEBUG_MSG), "NvmPartitionBuilder::Build, part:{} blksPerChunk:{}",
        PARTITION_TYPE_STR[option.partitionType], option.blksPerChunk);
    int ret = impl->Create(startLba, option.blksPerChunk);
    if (ret != 0)
    {
        return ret;
    }
    out[option.partitionType] = impl;
    if (next != nullptr)
    {
        uint64_t nextLba = impl->GetLastLba() + 1;
        return next->Build(nextLba, out);
    }
    return 0;
}

} // namespace pos
