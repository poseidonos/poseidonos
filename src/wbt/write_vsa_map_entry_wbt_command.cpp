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

#include "src/array_mgmt/array_manager.h"
#include "src/include/address_type.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/volume/volume_service.h"
#include "src/wbt/write_vsa_map_entry_wbt_command.h"

#include <string>

namespace pos
{
WriteVsaMapEntryWbtCommand::WriteVsaMapEntryWbtCommand(void)
:   WbtCommand(WRITE_VSAMAP_ENTRY, "write_vsamap_entry")
{
}
// LCOV_EXCL_START
WriteVsaMapEntryWbtCommand::~WriteVsaMapEntryWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
WriteVsaMapEntryWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int res = -1;
    std::string arrayName = _GetParameter(argv, "array");
    if (0 == arrayName.length())
    {
        return res;
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        return res;
    }

    IVolumeInfoManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    int volId = volMgr->GetVolumeID(argv["name"].get<std::string>());

    if (volId < 0)
    {
        res = volId;
    }
    else
    {
        BlkAddr rba = static_cast<BlkAddr>(std::stoull(argv["rba"].get<std::string>()));
        VirtualBlkAddr vsa =
            {
                .stripeId = static_cast<StripeId>(std::stoul(argv["vsid"].get<std::string>())),
                .offset = static_cast<BlkOffset>(std::stoull(argv["offset"].get<std::string>()))};

        IMapperWbt* iMapperWbt = MapperServiceSingleton::Instance()->GetIMapperWbt(arrayName);
        res = iMapperWbt->WriteVsaMapEntry(volId, rba, vsa);
    }

    return res;
}

} // namespace pos
