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

#include "translate_device_lba_wbt_command.h"

#include <cstdlib>
#include <fstream>
#include <string>

#include "src/array/device/array_device.h"
#include "src/array/service/array_service_layer.h"
#include "src/array/service/io_translator/i_io_translator.h"
#include "src/array_mgmt/array_manager.h"
#include "src/device/base/ublock_device.h"
#include "src/include/address_type.h"
#include "src/include/partition_type.h"

namespace pos
{
TranslateDeviceLbaWbtCommand::TranslateDeviceLbaWbtCommand(void)
: WbtCommand(TRANSLATE_DEVICE_LBA, "translate_device_lba")
{
}
// LCOV_EXCL_START
TranslateDeviceLbaWbtCommand::~TranslateDeviceLbaWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
TranslateDeviceLbaWbtCommand::Execute(Args& argv, JsonElement& elem)
{
    int ret = -1;
    std::string coutfile = "output.txt";
    std::ofstream out(coutfile.c_str(), std::ofstream::app);
    string arrayName;
    unsigned int arrayIndex;

    if (!argv.contains("lsid") || !argv.contains("offset") || !argv.contains("array"))
    {
        out << "invalid parameter" << endl;
        out.close();
        return ret;
    }

    if (argv.contains("array"))
    {
        arrayName = argv["array"].get<std::string>();
        ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
        if (info == nullptr)
        {
            out << "wrong array name";
            out.close();
            return ret;
        }
    }

    arrayIndex = ArrayMgr()->GetInfo(arrayName)->arrayInfo->GetIndex();
    LogicalBlkAddr lsa =
        {
            .stripeId = static_cast<uint32_t>(strtoul(argv["lsid"].get<std::string>().c_str(), nullptr, 0)),
            .offset = strtoul(argv["offset"].get<std::string>().c_str(), nullptr, 0),
        };

    out << "logical stripe : " << lsa.stripeId << std::endl;
    out << "logical offset : " << lsa.offset << std::endl;

    PhysicalBlkAddr pba;
    list<PhysicalEntry> physicalEntries;
    LogicalEntry logicalEntry{
        .addr = lsa,
        .blkCnt = 1
    };
    IIOTranslator* trans = ArrayService::Instance()->Getter()->GetTranslator();
    //ret = trans->Translate(arrayIndex, USER_DATA, pba, lsa);
    ret = trans->Translate(arrayIndex, USER_DATA, physicalEntries, logicalEntry);
    pba = physicalEntries.front().addr;
    if (ret != 0 || pba.arrayDev == nullptr)
    {
        out << "translation failed" << std::endl;
    }
    else
    {
        UblockSharedPtr uBlock = pba.arrayDev->GetUblock();
        string deviceName = uBlock->GetName();
        uint64_t lba = pba.lba;

        out << "device name : " << deviceName << std::endl;
        out << "lba : " << lba << std::endl;
        out << std::endl;
    }
    out.close();

    return ret;
}

} // namespace pos
