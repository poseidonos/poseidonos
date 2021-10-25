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

#include "write_raw_data_wbt_command.h"

#include "src/array/device/array_device.h"
#include "src/bio/ubio.h"
#include "src/device/device_manager.h"
#include "src/io_scheduler/io_dispatcher.h"

#include <string>

namespace pos
{
WriteRawDataCommand::WriteRawDataCommand(void)
:   RawDataWbtCommand(WRITE_RAW_DATA, "write_raw")
{
}
// LCOV_EXCL_START
WriteRawDataCommand::~WriteRawDataCommand(void)
{
}
// LCOV_EXCL_STOP
int
WriteRawDataCommand::Execute(Args &argv, JsonElement &elem)
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
                unitCount = std::stoul(numLogicalBlockCount) + 1;

                UbioSmartPtr ubio(new Ubio(nullptr, unitCount, 0));
                ubio->dir = UbioDir::Write;

                std::string startOffset = _GetParameter(argv, "lba");
                ubio->SetLba(std::stoull(startOffset));
                ubio->SetUblock(targetDevice);

                returnValue = _FilloutPayloadToWrite(argv,
                    ubio->GetBuffer(), ubio->GetMemSize());
                if (0 == returnValue)
                {
                    IODispatcher* ioDispatcher =
                        IODispatcherSingleton::Instance();
                    int retValue = ioDispatcher->Submit(ubio, true);
                    if (retValue < 0 || ubio->GetError() != IOErrorType::SUCCESS)
                    {
                        returnValue = -1;
                    }
                }
            }
        }
    }

    return returnValue;
}

bool
WriteRawDataCommand::_VerifySpecificParameters(Args& argv)
{
    bool parametersValid = false;

    std::string pattern = _GetParameter(argv, "pattern");
    std::string inputFileName = _GetParameter(argv, "input");

    if (0 < pattern.length())
    {
        parametersValid = true;
    }

    if (0 < inputFileName.length())
    {
        int errorCode = access(inputFileName.c_str(), R_OK);
        if (0 == errorCode)
        {
            parametersValid = true;
        }
    }

    return parametersValid;
}

int
WriteRawDataCommand::_FilloutPayloadToWrite(Args& argv,
    void* buffer, uint32_t bytesToWrite)
{
    int returnValue = -1;
    int written = 0;
    std::string pattern = _GetParameter(argv, "pattern");
    if (0 < pattern.length())
    {
        uint32_t patternToInput = _ConvertHexStringToUINT32(pattern);
        written = _WritePattern(buffer, patternToInput, bytesToWrite);
        if (0 < written)
        {
            returnValue = 0;
        }
    }
    else
    {
        std::string inputFileName = _GetParameter(argv, "input");
        int fd = open(inputFileName.c_str(), O_RDONLY);
        if (2 < fd)
        {
            int bytesRead = read(fd, buffer, bytesToWrite);

            if (0 < bytesRead)
            {
                if (static_cast<uint32_t>(bytesRead) < bytesToWrite)
                {
                    uint8_t* bufferToFill = static_cast<uint8_t*>(buffer);
                    uint32_t remainingBytes = bytesToWrite - bytesRead;
                    memset(&bufferToFill[bytesRead], 0xCE, remainingBytes);
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

int
WriteRawDataCommand::_WritePattern(void* buf, uint32_t pattern, uint32_t bytesToWrite)
{
    int bytesWritten = 0;
    uint32_t* bucket = static_cast<uint32_t*>(buf);
    uint32_t bucketCount = bytesToWrite / sizeof(uint32_t);

    for (uint32_t bucketIndex = 0; bucketIndex < bucketCount; bucketIndex++)
    {
        bucket[bucketIndex] = pattern;
    }

    bytesWritten = bucketCount * sizeof(uint32_t);

    return bytesWritten;
}

} // namespace pos
