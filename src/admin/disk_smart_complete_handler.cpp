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

#include "disk_smart_complete_handler.h"

#include "src/admin/admin_command_complete_handler.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/spdk_event_scheduler.h"
namespace pos
{
DiskSmartCompleteHandler::DiskSmartCompleteHandler(struct spdk_nvme_health_information_page* resultPage, uint32_t volId, uint32_t arrayId,
        uint32_t originCore, pos_io* io, CallbackSmartPtr callback, SmartLogMgr* smartLogMgr)
: Callback(false, CallbackType_DiskSmartCompleteHandler),
  smartLogMgr(smartLogMgr),
  resultPage(resultPage),
  volId(volId),
  arrayId(arrayId),
  originCore(originCore),
  io(io),
  callback(callback)
{
}
DiskSmartCompleteHandler::~DiskSmartCompleteHandler(void)
{
}
void
DiskSmartCompleteHandler::_AddComponentTemperature(void)
{
    // test cpu temperature
    int index = 0;
    ComponentManager* componentMgr = new ComponentManager();
    uint64_t cpuTemperature = componentMgr->FindCpuTemperature();
    // since the spdk parser converts temperature from kelvin to celsius and cpu temperature is reported in celsius.
    cpuTemperature = cpuTemperature + KELVIN_TO_CELSIUS;
    delete componentMgr;
    resultPage->temperature = (resultPage->temperature + cpuTemperature) / NO_OF_COMPONENTS;
    // check for temperature sensor with no value(not filled) and fill cpu temperature in that
    for (index = 0; index < NO_OF_TEMP_SENSORS; index++)
    {
        if (resultPage->temp_sensor[index] == 0)
        {
            resultPage->temp_sensor[index] = cpuTemperature;
            break;
        }
    }
    return;
}
void
DiskSmartCompleteHandler::_SetValfromSmartLogMgr(void)
{
    resultPage->host_read_commands[0] = smartLogMgr->GetReadCmds(volId, arrayId);
    resultPage->host_write_commands[0] = smartLogMgr->GetWriteCmds(volId, arrayId);

    resultPage->host_read_commands[1] = 0;
    resultPage->host_write_commands[1] = 0;

    uint64_t dataBytesRead = smartLogMgr->GetReadBytes(volId, arrayId);
    if (dataBytesRead == 0)
        resultPage->data_units_read[0] = 0;
    else
        resultPage->data_units_read[0] = dataBytesRead / NVME_SPEC_BYTE_UNIT + 1;

    uint64_t dataBytesWritten = smartLogMgr->GetWriteBytes(volId, arrayId);
    if (dataBytesWritten == 0)
        resultPage->data_units_written[0] = 0;
    else
        resultPage->data_units_written[0] = dataBytesWritten / NVME_SPEC_BYTE_UNIT + 1;

    resultPage->data_units_read[1] = 0;
    resultPage->data_units_written[1] = 0;
}

bool
DiskSmartCompleteHandler::_DoSpecificJob(void)
{
    _AddComponentTemperature();
    _SetValfromSmartLogMgr();
    EventSmartPtr event(new AdminCommandCompleteHandler(io, originCore, callback));
    bool success = SpdkEventScheduler::SendSpdkEvent(originCore,
        event);
    return success;
    // return true;
}
} // namespace pos
