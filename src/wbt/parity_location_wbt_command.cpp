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

#include "parity_location_wbt_command.h"

#include <cstdlib>
#include <fstream>
#include <string>

#include "src/array/array.h"
#include "src/array/ft/raid5.h"
#include "src/array_mgmt/array_manager.h"
#include "src/device/device_manager.h"
#include "src/array/partition/stripe_partition.h"

namespace pos
{
ParityLocationWbtCommand::ParityLocationWbtCommand(void)
:   WbtCommand(PARITY_LOCATION, "parity_location")
{
}
// LCOV_EXCL_START
ParityLocationWbtCommand::~ParityLocationWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
ParityLocationWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int ret = -1;
    std::string coutfile = "output.txt";
    std::ofstream out(coutfile.c_str(), std::ofstream::app);

    if (!argv.contains("dev") || !argv.contains("lba") || !argv.contains("array"))
    {
        out << "invalid parameter" << endl;
        out.close();
        return ret;
    }

    string devName = argv["dev"].get<std::string>();
    uint64_t lba = atoi(argv["lba"].get<std::string>().c_str());
    string arrayName = argv["array"].get<std::string>();

    ArrayComponents* compo = ArrayManagerSingleton::Instance()->_FindArray(arrayName);
    if (compo == nullptr)
    {
        out << "array does not exist" << endl;
        out.close();
        return ret;
    }
    Array* sysArray = compo->GetArray();
    if (sysArray == nullptr)
    {
        out << "array does not exist" << endl;
        out.close();
        return ret;
    }
    if (sysArray->state->IsMounted() == false)
    {
        out << "array is not mounted" << endl;
        out.close();
        return ret;
    }

    ArrayDeviceType devType;
    ArrayDevice* arrayDev = nullptr;
    DevName name(devName);
    UblockSharedPtr uBlock = DeviceManagerSingleton::Instance()->GetDev(name);
    tie(arrayDev, devType) = sysArray->devMgr_->GetDev(uBlock);
    if (arrayDev == nullptr || devType != ArrayDeviceType::DATA)
    {
        out << "device not found" << endl;
        return ret;
    }
    PhysicalBlkAddr pba = {
        .lba = lba,
        .arrayDev = arrayDev};
    StripePartition* ptn = static_cast<StripePartition*>(
        sysArray->ptnMgr->partitions[PartitionType::USER_DATA]);
    Method* method = static_cast<Method*>(ptn->GetMethod());
    if (method == nullptr)
    {
        out << "no ft method in the userdata partition" << endl;
        return ret;
    }
    FtBlkAddr fba = ptn->_Pba2Fba(pba);
    vector<uint32_t> parityIndexs = method->GetParityOffset(fba.stripeId);
    for (uint32_t parityIndex : parityIndexs)
    {
        ArrayDevice* parityDev = ptn->devs.at(parityIndex);
        if (parityDev->GetState() != ArrayDeviceState::FAULT)
        {
            out << "device name : " << parityDev->GetUblock()->GetName() << endl;
            out << "lba : " << lba << endl;
        }
    }
    out << std::endl;
    out.close();
    ret = 0;
    return ret;
}

} // namespace pos
