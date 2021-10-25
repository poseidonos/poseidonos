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

#include "write_uncorrectable_lba_wbt_command.h"

#include <cstdint>
#include <string>

#include "src/array/device/array_device.h"
#include "src/bio/ubio.h"
#include "src/device/device_manager.h"
#include "src/include/io_error_type.h"
#include "src/io_scheduler/io_dispatcher.h"

namespace pos
{
WriteUncorrectableLbaWbtCommand::WriteUncorrectableLbaWbtCommand(void)
:   WbtCommand(WRITE_UNCORRECTABLE_LBA, "write_uncorrectable_lba")
{
}
// LCOV_EXCL_START
WriteUncorrectableLbaWbtCommand::~WriteUncorrectableLbaWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
WriteUncorrectableLbaWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int returnValue = -1;

    if (argv.contains("dev") && argv.contains("lba"))
    {
        DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
        DevName _dev(argv["dev"].get<std::string>());
        UblockSharedPtr targetDevice = deviceMgr->GetDev(_dev);
        if (nullptr != targetDevice)
        {
            uint8_t dummyBuffer[Ubio::BYTES_PER_UNIT];
            uint32_t unitCount = 1;
            UbioSmartPtr ubio(new Ubio(dummyBuffer, unitCount, 0));
            ubio->dir = UbioDir::WriteUncor;
            ubio->SetLba(std::stoull(argv["lba"].get<std::string>()));
            ubio->SetUblock(targetDevice);

            IODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
            int retValue = ioDispatcher->Submit(ubio, true);
            if (retValue >= 0 &&
                ubio->GetError() == IOErrorType::SUCCESS)
            {
                returnValue = 0;
            }
        }
    }

    return returnValue;
}

} // namespace pos
