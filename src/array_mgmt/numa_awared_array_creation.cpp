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

#include "numa_awared_array_creation.h"
#include "src/device/device_manager.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/helper/enumerable/query.h"

namespace pos
{
NumaAwaredArrayCreation::NumaAwaredArrayCreation(vector<string> buffers, int dataCnt, int spareCnt, DeviceManager* devMgr)
{
    result.code = EID(SUCCESS);

    DevName bufferName(buffers.front());
    UblockSharedPtr buf = devMgr->GetDev(bufferName);
    if (buf == nullptr)
    {
        result.code = EID(CREATE_ARRAY_NVM_NAME_NOT_FOUND);
        POS_TRACE_WARN(result.code, "nvm_name:{}", buffers.front());
        return;
    }
    int requiredDevCnt = dataCnt + spareCnt;
    int targetNumaId = buf->GetNuma();
    auto&& devsInTargetNuma = Enumerable::Where(devMgr->GetDevs(),
        [targetNumaId](auto d) { return (d->GetClass() == DeviceClass::SYSTEM &&
                             d->GetType() == DeviceType::SSD &&
                             d->GetNuma() == targetNumaId); });
    size_t numofDevsInTargetNuma = devsInTargetNuma.size();
    if ((int)numofDevsInTargetNuma >= requiredDevCnt)
    {
        auto&& devsBySize = Enumerable::GroupBy(devsInTargetNuma,
                [](auto d) { return d->GetSize(); });
        for (auto sizeGroup : devsBySize)
        {
            uint64_t devSize = sizeGroup.first;
            auto sameSizeDevs = sizeGroup.second;
            size_t numofDevsWithSameCapacity = sameSizeDevs.size();
            POS_TRACE_DEBUG(EID(AUTO_CREATE_ARRAY_DEBUG), "Matching devices... required:{}, numa_id:{}, num_of_ssds_in_the_numa:{}, num_of_ssds_same_capa:{}, capacity:{}",
                requiredDevCnt, targetNumaId, numofDevsInTargetNuma, numofDevsWithSameCapacity, devSize);
            if ((int)numofDevsWithSameCapacity >= requiredDevCnt)
            {
                ArrayCreationOptions option;
                option.numaId = targetNumaId;
                int dCnt = dataCnt;
                int sCnt = spareCnt;
                for (UblockSharedPtr dev : sameSizeDevs)
                {
                    if (dCnt > 0)
                    {
                        option.devs.data.push_back(dev->GetName());
                        dCnt--;
                    }
                    else if (sCnt > 0)
                    {
                        option.devs.spares.push_back(dev->GetName());
                        sCnt--;
                    }
                }
                if (buffers.size() > 0)
                {
                    option.devs.nvm.push_back(buffers.front());
                }
                option.capacity = devSize * dataCnt;
                result.options.push_back(option);
                result.code = EID(SUCCESS);
                POS_TRACE_INFO(EID(AUTO_CREATE_ARRAY_DEBUG), "Device matching success, count:{}, numa_id:{}, capacity:{}",
                    requiredDevCnt, targetNumaId, devSize);
                break;
            }
            else
            {
                result.code = EID(CREATE_ARRAY_INSUFFICIENT_SAME_CAPACITY_DEVS);
            }
        }
    }
    else
    {
        result.code = EID(CREATE_ARRAY_INSUFFICIENT_SAME_NUMA_DEVS);
    }

    if (result.code != EID(SUCCESS))
    {
        POS_TRACE_WARN(result.code, "Device matching failed, required:{}, numa_id:{}, num_of_ssds_in_the_numa:{}",
            requiredDevCnt, targetNumaId, numofDevsInTargetNuma);
    }
}
} // namespace pos
