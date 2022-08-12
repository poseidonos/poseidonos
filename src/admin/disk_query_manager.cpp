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
#include "disk_query_manager.h"

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "spdk/nvme.h"
#include "spdk/nvme_spec.h"
#include "spdk/pos.h"
#include "src/admin/disk_smart_complete_handler.h"
#include "src/admin/smart_log_mgr.h"
#include "src/admin/smart_log_update_request.h"
#include "src/array/device/i_array_device_manager.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_models/dto/device_set.h"
#include "src/device/device_manager.h"
#include "src/include/pos_event_id.hpp"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/logger/logger.h"
namespace pos
{
GetLogPageContext::GetLogPageContext(void* data, uint16_t lid)
: payload(data),
  lid(lid)
{
}

DiskQueryManager::DiskQueryManager(struct spdk_nvme_cmd* cmd, struct spdk_nvme_health_information_page* resultPage, pos_io* io,
    uint32_t originCore, CallbackSmartPtr callback, IArrayInfo* info, IDevInfo* devInfo,
    IIODispatcher* dispatcher, IArrayDevMgr* arrayDevMgr, SmartLogMgr* smartLogMgr)
: cmd(cmd),
  resultPage(resultPage),
  io(io),
  originCore(originCore),
  cb(callback),
  arrayInfo(info),
  devInfo(devInfo),
  dispatcher(dispatcher),
  arrayDevMgr(arrayDevMgr),
  smartLogMgr(smartLogMgr)
{
}

bool
DiskQueryManager::SendSmartCommandtoDisk(void)
{
    vector<UblockSharedPtr> devices;
    DeviceSet<string> nameSet = arrayInfo->GetDevNames();
    for (string deviceName : nameSet.data)
    {
        DevName name(deviceName);
        UblockSharedPtr uBlock = devInfo->GetDev(name);
        devices.push_back(uBlock);
    }

    if (devices.size() == 0)
    {
        POS_TRACE_ERROR(EID(SMART_LOG_NO_DISK_IN_ARRAY),
            "No Device in Array");
        return true;
    }
    CallbackSmartPtr callback(new DiskSmartCompleteHandler(resultPage, io->volume_id, arrayInfo->GetIndex(), originCore, io, cb, smartLogMgr));
    callback->SetWaitingCount(devices.size());
    for (size_t i = 0; i < devices.size(); i++)
    {
        tuple<ArrayDevice*, ArrayDeviceType> devtuple = arrayDevMgr->GetDev(devices[i]);
        PhysicalBlkAddr addr;
        addr.lba = INVALID_LBA;
        addr.arrayDev = get<0>(devtuple);
        struct spdk_nvme_health_information_page* payload = new struct spdk_nvme_health_information_page();
        uint16_t lid = SPDK_NVME_LOG_HEALTH_INFORMATION;
        GetLogPageContext* smartLogPageContext = new GetLogPageContext(payload, lid);
        UbioSmartPtr ubio(new Ubio((void*)smartLogPageContext, sizeof(struct spdk_nvme_health_information_page), arrayInfo->GetIndex()));
        ubio->dir = UbioDir::GetLogPage;
        ubio->SetPba(addr);

        CallbackSmartPtr smartUpdateRequest(new SmartLogUpdateRequest(resultPage, payload, io, originCore));
        smartUpdateRequest->SetCallee(callback);
        ubio->SetCallback(smartUpdateRequest);
        dispatcher->Submit(ubio);
    }
    return true;
}

bool
DiskQueryManager::SendLogPagetoDisk(struct spdk_nvme_cmd* cmd)
{
    uint8_t lid;
    lid = cmd->cdw10 & 0xFF;
    switch (lid)
    {
        case SPDK_NVME_LOG_HEALTH_INFORMATION:
            return SendSmartCommandtoDisk();
            break;
        default:
            return false;
    }
}
bool
DiskQueryManager::Execute(void)
{
    switch (cmd->opc)
    {
        case SPDK_NVME_OPC_GET_LOG_PAGE:
            return SendLogPagetoDisk(cmd);
            break;
        default:
            return false;
    }
}

} // namespace pos
