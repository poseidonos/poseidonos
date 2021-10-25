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

#include <unistd.h>

#include <cstring>
#include <string>

#include "src/wbt/nvme_cli_command.h"

#include "src/array/device/array_device.h"
#include "src/device/device_manager.h"
#include "src/io_scheduler/io_dispatcher.h"

namespace pos
{
const uint32_t NvmeCliCommand::BUFFER_SIZE_4KB = 4 * 1024;
const uint32_t NvmeCliCommand::SECTOR_SIZE = 512;

NvmeCliCommand::NvmeCliCommand(void)
: WbtCommand(NVME_CLI, "nvme_cli")
{
}
// LCOV_EXCL_START
NvmeCliCommand::~NvmeCliCommand(void)
{
}
// LCOV_EXCL_STOP
int
NvmeCliCommand::Execute(Args& argv, JsonElement& elem)
{
    int returnValue = -1;

    if (true == _VerifyCommonParameters(argv))
    {
        std::string diskName = _GetParameter(argv, "dev");
        DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
        DevName dev(diskName);
        UblockSharedPtr targetDevice = deviceMgr->GetDev(dev);

        struct spdk_nvme_ctrlr* ctrlr;
        ctrlr = deviceMgr->GetNvmeCtrlr(diskName);

        if (nullptr != targetDevice)
        {
            void* buffer = Memory<SECTOR_SIZE>::Alloc(BUFFER_SIZE_4KB / SECTOR_SIZE);
            int cmd = stoi(_GetParameter(argv, "op"));

            switch (cmd)
            {
                case spdk_nvme_admin_opcode::SPDK_NVME_OPC_GET_LOG_PAGE:
                {
                    returnValue = _NvmeGetLogPage(&targetDevice, argv, buffer);
                    break;
                }
                case spdk_nvme_admin_opcode::SPDK_NVME_OPC_IDENTIFY:
                {
                    returnValue = _NvmeIdentify(ctrlr, argv, buffer);
                    break;
                }
                case spdk_nvme_admin_opcode::SPDK_NVME_OPC_FORMAT_NVM:
                {
                    returnValue = _NvmeFormat(ctrlr, argv);
                    break;
                }
                default:
                    break;
            }

            Memory<SECTOR_SIZE>::Free(buffer);
        }
    }

    return returnValue;
}

int
NvmeCliCommand::_NvmeIdentify(spdk_nvme_ctrlr* ctrlr, Args& argv, void* buffer)
{
    int returnValue = -1;
    int cns = stoi(_GetParameter(argv, "cns"));
    int nsid = stoi(_GetParameter(argv, "nsid"));

    switch (cns)
    {
        case spdk_nvme_identify_cns::SPDK_NVME_IDENTIFY_CTRLR:
        {
            const struct spdk_nvme_ctrlr_data* cdata;
            cdata = spdk_nvme_ctrlr_get_data(ctrlr);
            memcpy(buffer, cdata, sizeof(spdk_nvme_ctrlr_data));

            returnValue = _DumpPayloadFromReadData(argv,
                buffer, sizeof(spdk_nvme_ctrlr_data));

            break;
        }
        case spdk_nvme_identify_cns::SPDK_NVME_IDENTIFY_NS:
        {
            const struct spdk_nvme_ns_data* nsdata;
            struct spdk_nvme_ns* ns;

            ns = spdk_nvme_ctrlr_get_ns(ctrlr, nsid);
            nsdata = spdk_nvme_ns_get_data(ns);

            memcpy(buffer, nsdata, sizeof(spdk_nvme_ns_data));

            returnValue = _DumpPayloadFromReadData(argv, buffer, sizeof(spdk_nvme_ns_data));

            break;
        }
        default:
            break;
    }

    return returnValue;
}

int
NvmeCliCommand::_NvmeFormat(spdk_nvme_ctrlr* ctrlr, Args& argv)
{
    int ret = 0;
    int nsid = stoi(_GetParameter(argv, "nsid"));
    struct spdk_nvme_format format = {};

    format.lbaf = stoi(_GetParameter(argv, "lbaf"));
    format.ms = stoi(_GetParameter(argv, "ms"));
    format.pi = stoi(_GetParameter(argv, "pi"));
    format.pil = stoi(_GetParameter(argv, "pil"));
    format.ses = stoi(_GetParameter(argv, "ses"));

    ret = spdk_nvme_ctrlr_format(ctrlr, nsid, &format);

    return ret;
}

int
NvmeCliCommand::_NvmeGetLogPage(UblockSharedPtr* targetDevice, Args& argv, void* buffer)
{
    uint32_t unitCount = BUFFER_SIZE_4KB / Ubio::BYTES_PER_UNIT;
    int returnValue = -1;
    std::string arrayName = _GetParameter(argv, "array");

    UbioSmartPtr ubio(new Ubio(buffer, unitCount, 0));

    struct spdk_nvme_cmd cmd;
    {
        memset(&cmd, 0, sizeof(cmd));
        cmd.opc = stoi(_GetParameter(argv, "op"));
        cmd.nsid = stoi(_GetParameter(argv, "nsid"));
        cmd.cdw10 = stoi(_GetParameter(argv, "cns"));
    }

    std::memcpy(buffer, &cmd, sizeof(cmd));

    ubio->dir = UbioDir::NvmeCli;
    ubio->SetLba(0);
    ubio->SetUblock(*targetDevice);

    IODispatcher* ioDispatcher =
        IODispatcherSingleton::Instance();
    int retValue = ioDispatcher->Submit(ubio, true);
    if (retValue >= 0 &&
        ubio->GetError() == IOErrorType::SUCCESS)
    {
        returnValue = _DumpPayloadFromReadData(argv,
            ubio->GetBuffer(), ubio->GetMemSize());
    }

    return returnValue;
}

bool
NvmeCliCommand::_VerifyCommonParameters(Args& argv)
{
    bool parametersValid = false;

    std::string diskName = _GetParameter(argv, "dev");
    std::string arrayName = _GetParameter(argv, "array");
    std::string op = _GetParameter(argv, "op");

    if ((0 < diskName.length()) && (0 < arrayName.length()))
    {
        parametersValid = true;
    }

    return parametersValid;
}

int
NvmeCliCommand::_DumpPayloadFromReadData(Args& argv,
    void* buffer, uint32_t bytesToRead)
{
    int returnValue = -1;

    std::string inputFileName = _GetParameter(argv, "output");
    if (0 < inputFileName.length())
    {
        int flag = O_CREAT | O_WRONLY | O_TRUNC;
        int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        int fd = open(inputFileName.c_str(), flag, mode);
        if (2 < fd)
        {
            int bytesWritten = write(fd, buffer, bytesToRead);
            if (0 < bytesWritten)
            {
                if (static_cast<uint32_t>(bytesWritten) < bytesToRead)
                {
                    uint8_t* bufferToFill = static_cast<uint8_t*>(buffer);
                    uint32_t remainingBytes = bytesToRead - bytesWritten;
                    memset(&bufferToFill[bytesWritten], 0xFD, remainingBytes);
                }

                returnValue = 0;
            }
        }
        if (fd != -1)
        {
            close(fd);
        }
    }

    return returnValue;
}

} // namespace pos
