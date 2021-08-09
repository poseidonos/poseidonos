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

#include "dump_disk_layout_wbt_command.h"

#include <fstream>
#include <string>

#include "src/array/service/array_service_layer.h"
#include "src/array/service/io_translator/i_io_translator.h"
#include "src/array_mgmt/array_manager.h"
#include "src/include/address_type.h"
#include "src/include/array_config.h"
#include "src/include/partition_type.h"

namespace pos
{
DumpDiskLayoutWbtCommand::DumpDiskLayoutWbtCommand(void)
: WbtCommand(DUMP_DISK_LAYOUT, "dump_disk_layout")
{
}

DumpDiskLayoutWbtCommand::~DumpDiskLayoutWbtCommand(void)
{
}

int
DumpDiskLayoutWbtCommand::Execute(Args& argv, JsonElement& elem)
{
    std::string coutfile = "output.txt";
    std::ofstream out(coutfile.c_str(), std::ofstream::app);

    IIOTranslator* trans = ArrayService::Instance()->Getter()->GetTranslator();
    if (!argv.contains("name"))
    {
        out << "invalid parameter" << endl;
        out.close();
        return -1;
    }
    string arrayName = argv["name"].get<std::string>();
    IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    if (nullptr == info)
    {
        out << "there is no array with name " + arrayName << endl;
        out.close();
        return -1;
    }

    LogicalBlkAddr lsa = {.stripeId = 0, .offset = 0};

    PhysicalBlkAddr pba;
    trans->Translate(arrayName, META_SSD, pba, lsa);
    out << "meta ssd start lba : " << pba.lba << std::endl;

    const PartitionLogicalSize* logicalSize = info->GetSizeInfo(META_SSD);
    uint64_t blk = (uint64_t)logicalSize->totalStripes *
        logicalSize->blksPerChunk * ArrayConfig::SECTORS_PER_BLOCK;
    out << "meta ssd end lba : " << pba.lba + blk - 1 << std::endl;

    trans->Translate(arrayName, USER_DATA, pba, lsa);
    out << "user data start lba : " << pba.lba << std::endl;

    logicalSize = info->GetSizeInfo(USER_DATA);
    blk = (uint64_t)logicalSize->totalStripes * logicalSize->blksPerChunk * ArrayConfig::SECTORS_PER_BLOCK;
    out << "user data end lba : " << pba.lba + blk - 1 << std::endl;

    if (0 /*useNvm*/)
    {
        trans->Translate(arrayName, WRITE_BUFFER, pba, lsa);
        out << "write buffer start lba : " << pba.lba << std::endl;

        logicalSize = info->GetSizeInfo(WRITE_BUFFER);
        blk = (uint64_t)logicalSize->totalStripes * logicalSize->blksPerChunk * ArrayConfig::SECTORS_PER_BLOCK;
        out << "write buffer end lba : " << pba.lba + blk - 1 << std::endl;
    }

    out << std::endl;
    out.close();

    return 0;
}

} // namespace pos
