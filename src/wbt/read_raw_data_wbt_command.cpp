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

#include "read_raw_data_wbt_command.h"

#include <cstring>
#include <string>
#include <unistd.h>

#include "src/array/device/array_device.h"
#include "src/bio/ubio.h"
#include "src/device/device_manager.h"
#include "src/io_scheduler/io_dispatcher.h"

namespace pos
{
ReadRawDataCommand::ReadRawDataCommand(void)
:   RawDataWbtCommand(READ_RAW_DATA, "read_raw")
{
}
// LCOV_EXCL_START
ReadRawDataCommand::~ReadRawDataCommand(void)
{
}
// LCOV_EXCL_STOP
int
ReadRawDataCommand::Execute(Args &argv, JsonElement &elem)
{
    int returnValue = -1;

    if ((true == _VerifyCommonParameters(argv)) &&
        (true == _VerifySpecificParameters(argv)))
    {
        std::string numLogicalBlockCount = _GetParameter(argv, "count");

        const uint32_t HUGE_PAGE_SIZE_IN_BYTES = 2 * 1024 * 1024;
        uint32_t unitCount = HUGE_PAGE_SIZE_IN_BYTES / Ubio::BYTES_PER_UNIT;
        if (unitCount > std::stoul(numLogicalBlockCount)) // NLB is zero based.
        {
            std::string diskName = _GetParameter(argv, "dev");
            DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
            DevName dev(diskName);

            UblockSharedPtr targetDevice = deviceMgr->GetDev(dev);
            if (nullptr != targetDevice)
            {
                std::string startOffset = _GetParameter(argv, "lba");
                unitCount = std::stoul(numLogicalBlockCount) + 1;

                UbioSmartPtr ubio(new Ubio(nullptr, unitCount, 0));
                ubio->dir = UbioDir::Read;
                ubio->SetLba(std::stoull(startOffset));
                ubio->SetUblock(targetDevice);

                IODispatcher* ioDispatcher =
                    IODispatcherSingleton::Instance();
                int retValue = ioDispatcher->Submit(ubio, true);
                if (retValue >= 0 &&
                    ubio->GetError() == IOErrorType::SUCCESS)
                {
                    returnValue = _DumpPayloadFromReadData(argv,
                        ubio->GetBuffer(), ubio->GetMemSize());
                }
            }
        }
    }

    return returnValue;
}

bool
ReadRawDataCommand::_VerifySpecificParameters(Args& argv)
{
    bool parametersValid = false;

    std::string inputFileName = _GetParameter(argv, "output");

    if (0 < inputFileName.length())
    {
        int errorCode = access(inputFileName.c_str(), F_OK);
        if (-1 == errorCode)
        {
            parametersValid = true;
        }

        errorCode = access(inputFileName.c_str(), W_OK);
        if (0 == errorCode)
        {
            parametersValid = true;
        }
    }

    return parametersValid;
}

int
ReadRawDataCommand::_DumpPayloadFromReadData(Args& argv,
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
