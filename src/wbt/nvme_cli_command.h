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

#include "src/wbt/raw_data_wbt_command.h"
#include "spdk/nvme.h"
#include "spdk/nvme_spec.h"
#include "src/bio/ubio.h"

namespace pos
{
class NvmeCliCommand : public WbtCommand
{
public:
    NvmeCliCommand(void);
    virtual ~NvmeCliCommand(void);

    int Execute(Args& argv, JsonElement& elem) override;

private:
    static const uint32_t BUFFER_SIZE_4KB;
    static const uint32_t SECTOR_SIZE;

    bool _VerifyCommonParameters(Args& argv);
    int _DumpPayloadFromReadData(Args& argv, void* buffer, uint32_t bytesToRead);
    int _NvmeIdentify(spdk_nvme_ctrlr* ctrlr, Args& argv, void* buffer);
    int _NvmeFormat(spdk_nvme_ctrlr* ctrlr, Args& argv);
    int _NvmeGetLogPage(UblockSharedPtr* targetDevice, Args& argv, void* buffer);
};

} // namespace pos
