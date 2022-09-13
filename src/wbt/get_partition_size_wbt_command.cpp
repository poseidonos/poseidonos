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

#include "get_partition_size_wbt_command.h"

#include "src/array_mgmt/array_manager.h"
#include "src/include/pos_event_id.h"

#include <string>

namespace pos
{
GetPartitionSizeWbtCommand::GetPartitionSizeWbtCommand(void)
: WbtCommand(GET_PARTITION_SIZE, "get_partition_size")
{
}
// LCOV_EXCL_START
GetPartitionSizeWbtCommand::~GetPartitionSizeWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
GetPartitionSizeWbtCommand::Execute(Args& argv, JsonElement& elem)
{
    int ret = -1;
    string arrayName;
    if (argv.contains("array"))
    {
        arrayName = argv["array"].get<std::string>();
    }
    else
    {
        return ret;
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        return ret;
    }

    IArrayInfo* array = info->arrayInfo;
    string state = array->GetState().ToString();
    if (state == "NORMAL" || state == "DEGRADED" || state == "REBUILD")
    {
        JsonElement partitionElement("partition size");
        const PartitionLogicalSize* metaPartitionSize = array->GetSizeInfo(PartitionType::META_SSD);
        partitionElement.SetAttribute(JsonAttribute("meta partition totalStripes", "\"" + to_string(metaPartitionSize->totalStripes) + "\""));
        partitionElement.SetAttribute(JsonAttribute("meta partition totalSegments", "\"" + to_string(metaPartitionSize->totalSegments) + "\""));
        partitionElement.SetAttribute(JsonAttribute("meta partition stripesPerSegment", "\"" + to_string(metaPartitionSize->stripesPerSegment) + "\""));
        partitionElement.SetAttribute(JsonAttribute("meta partition minWriteBlkCnt", "\"" + to_string(metaPartitionSize->minWriteBlkCnt) + "\""));
        partitionElement.SetAttribute(JsonAttribute("meta partition chunksPerStripe", "\"" + to_string(metaPartitionSize->chunksPerStripe) + "\""));
        partitionElement.SetAttribute(JsonAttribute("meta partition blksPerStripe", "\"" + to_string(metaPartitionSize->blksPerStripe) + "\""));
        partitionElement.SetAttribute(JsonAttribute("meta partition blksPerChunk", "\"" + to_string(metaPartitionSize->blksPerChunk) + "\""));
        const PartitionLogicalSize* userPartitionSize = array->GetSizeInfo(PartitionType::USER_DATA);
        partitionElement.SetAttribute(JsonAttribute("user partition totalStripes", "\"" + to_string(userPartitionSize->totalStripes) + "\""));
        partitionElement.SetAttribute(JsonAttribute("user partition totalSegments", "\"" + to_string(userPartitionSize->totalSegments) + "\""));
        partitionElement.SetAttribute(JsonAttribute("user partition stripesPerSegment", "\"" + to_string(userPartitionSize->stripesPerSegment) + "\""));
        partitionElement.SetAttribute(JsonAttribute("user partition minWriteBlkCnt", "\"" + to_string(userPartitionSize->minWriteBlkCnt) + "\""));
        partitionElement.SetAttribute(JsonAttribute("user partition chunksPerStripe", "\"" + to_string(userPartitionSize->chunksPerStripe) + "\""));
        partitionElement.SetAttribute(JsonAttribute("user partition blksPerStripe", "\"" + to_string(userPartitionSize->blksPerStripe) + "\""));
        partitionElement.SetAttribute(JsonAttribute("user partition blksPerChunk", "\"" + to_string(userPartitionSize->blksPerChunk) + "\""));
        ret = 0;
        elem.SetElement(partitionElement);
        return ret;
    }

    return EID(WBT_CMD_NEED_ARRAY_MOUNT);
}

} // namespace pos
