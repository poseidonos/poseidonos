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

#include "src/admin/smart_log_page_handler.h"

#include "lib/spdk/include/spdk/bdev_module.h"
#include "lib/spdk/include/spdk/nvme_spec.h"
#include "lib/spdk/lib/nvmf/nvmf_internal.h"
#include "spdk/pos.h"
#include "src/admin/admin_command_handler.h"
#include "src/admin/disk_query_manager.h"
#include "src/admin/smart_log_mgr.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
SmartLogPageHandler::SmartLogPageHandler(struct spdk_nvme_cmd* cmd, pos_io* io, void* smartLogPageData, uint32_t originCore,
    CallbackSmartPtr callback, IArrayInfo* info, IDevInfo* devInfo,
    IIODispatcher* dispatcher, IArrayDevMgr* arrayDevMgr, SmartLogMgr* smartLogMgr, EventScheduler* eventSchedulerArg)
: cmd(cmd),
  io(io),
  smartLogPageData(smartLogPageData),
  page(nullptr),
  originCore(originCore),
  callback(callback),
  arrayInfo(info),
  devInfo(devInfo),
  dispatcher(dispatcher),
  arrayDevMgr(arrayDevMgr),
  smartLogMgr(smartLogMgr),
  eventScheduler(eventSchedulerArg)
{
    if (eventScheduler == nullptr)
    {
        eventScheduler = EventSchedulerSingleton::Instance();
    }
}

bool
SmartLogPageHandler::Execute(void)
{
    if (smartLogPageData == NULL)
    {
        POS_TRACE_ERROR(EID(SMART_LOG_INVALID_BUFFER),
            "Invalid buffer in smart log page");
        return true;
    }
    struct spdk_nvme_health_information_page* smartLogPage = (struct spdk_nvme_health_information_page*)smartLogPageData;
    EventSmartPtr diskMgr(new DiskQueryManager(cmd, smartLogPage, io, originCore, callback, arrayInfo, devInfo, dispatcher, arrayDevMgr, smartLogMgr));
    bool result = diskMgr->Execute();
    if (result == false)
        eventScheduler->EnqueueEvent(diskMgr);
    return true;
}

} // namespace pos
