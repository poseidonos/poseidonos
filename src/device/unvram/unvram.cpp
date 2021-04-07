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

#include "unvram.h"

#include <errno.h>
#include <fcntl.h>
#include <numa.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <utility>

#include "src/array/device/array_device.h"
#include "src/device/event_framework_api.h"
#include "src/device/ioat_api.h"
#include "src/include/ibof_event_id.hpp"
#include "src/io/general_io/affinity_manager.h"
#include "src/io/general_io/ubio.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/scheduler/callback.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"
#include "unvram_drv.h"
#include "uram_device_context.h"
#include "unvram_restore_completion.h"

namespace ibofos
{
thread_local uint32_t UnvramBdev::requestCount;

UnvramBdev::UnvramBdev(std::string name,
    uint64_t size,
    UnvramDrv* driverToUse)
: UBlockDevice(name, size, driverToUse),
  ioatReactorCount(0),
  reactorCount(0)
{
    property.type = DeviceType::NVRAM;
    property.mn = name;
    property.sn = name;

    for (uint32_t i = 0; i < MAX_THREAD_COUNT; i++)
    {
        ioatReactor[i] = UINT32_MAX;
    }
    _GetIoatCntInNuma();
}

UnvramBdev::~UnvramBdev(void)
{
}

void
UnvramBdev::_GetIoatCntInNuma(void)
{
    ioatInNuma0 = 0;
    ioatInNuma1 = 0;

    useIoat = IoatApi::IsIoatEnable();
    if (useIoat == true)
    {
        int32_t numaCnt = numa_num_configured_nodes();
        if (numaCnt > MAX_NUMA_COUNT)
        {
            useIoat = false;
            IBOF_TRACE_WARN(IBOF_EVENT_ID::IOAT_CONFIG_INVALID,
                "max numa count is over 2. disable awaring numa");
            return;
        }
        ConfigManager& configManager = *ConfigManagerSingleton::Instance();
        int ret = configManager.GetValue("ioat", "ioat_cnt_numa0", &ioatInNuma0,
            CONFIG_TYPE_UINT32);
        bool numa0Config = (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS));

        ret = configManager.GetValue("ioat", "ioat_cnt_numa1", &ioatInNuma1,
            CONFIG_TYPE_UINT32);
        bool numa1Config = (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS));

        if (numa0Config == false || numa1Config == false || ioatInNuma0 == 0 || ioatInNuma1 == 0)
        {
            useIoat = false;
            IBOF_TRACE_WARN(IBOF_EVENT_ID::IOAT_CONFIG_INVALID,
                "IOAT HW count for each Numa from config is not valid");
            return;
        }
    }
}

bool
UnvramBdev::_RecoverBackup(DeviceContext* deviceContext)
{
    bool restoreSuccessful = true;

    const char* backupFileName = "/etc/uram_backup/uram_backup.bin";
    const uint32_t bytesPerHugepage = 2 * SZ_1MB;
    int fd = -1;

    try
    {
        fd = open(backupFileName, O_RDONLY);
        if (0 > fd)
        {
            if (errno == ENOENT)
            {
                IBOF_EVENT_ID eventId =
                    IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_NOT_EXISTS;
                EventLevel eventLevel = EventLevel::INFO;
                throw std::make_pair(eventId, eventLevel);
            }
            else
            {
                IBOF_EVENT_ID eventId =
                    IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_OPEN_FAILED;
                EventLevel eventLevel = EventLevel::WARNING;
                throw std::make_pair(eventId, eventLevel);
            }
        }

        struct stat fileStat;
        int rc = fstat(fd, &fileStat);
        if (0 > rc)
        {
            IBOF_EVENT_ID eventId =
                IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_STAT_FAILED;
            EventLevel eventLevel = EventLevel::WARNING;
            throw std::make_pair(eventId, eventLevel);
        }

        uint64_t pageCountBackup = fileStat.st_size / bytesPerHugepage;
        uint32_t unitsPerHugepage = bytesPerHugepage / Ubio::BYTES_PER_UNIT;

        for (uint64_t pageIndex = 0; pageIndex < pageCountBackup; pageIndex++)
        {
            UbioSmartPtr ubio(new Ubio(nullptr,
                DivideUp(bytesPerHugepage, Ubio::BYTES_PER_UNIT)));
            ubio->dir = UbioDir::Write;
            ArrayDevice* dev = new ArrayDevice(this, ArrayDeviceState::NORMAL);
            PhysicalBlkAddr pba = {.dev = dev,
                .lba = pageIndex * unitsPerHugepage};
            ubio->SetPba(pba);
            CallbackSmartPtr callback(new UnvramRestoreCompletion(ubio));
            ubio->SetCallback(callback);

            rc = read(fd, ubio->GetBuffer(), ubio->GetSize());
            if (bytesPerHugepage == rc)
            {
                UnvramRestoreCompletion::IncreasePendingUbio();
                SubmitAsyncIO(ubio);
            }
            else
            {
                delete dev;

                IBOF_EVENT_ID eventId =
                    IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_READ_FAILED;
                std::string additionalMessage(
                    "Read page # " + std::to_string(pageIndex) + " failed");
                throw std::make_pair(eventId, additionalMessage);
            }
        }
    }
    catch (std::pair<IBOF_EVENT_ID, EventLevel> eventWithLevel)
    {
        IBOF_EVENT_ID eventId = eventWithLevel.first;
        EventLevel eventLevel = eventWithLevel.second;
        IbofEventId::Print(eventId, eventLevel);
        if (eventId != IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_NOT_EXISTS)
        {
            restoreSuccessful = false;
        }
    }
    catch (std::pair<IBOF_EVENT_ID, std::string> eventWithMessage)
    {
        IBOF_EVENT_ID eventId = eventWithMessage.first;
        std::string& additionalMessage = eventWithMessage.second;
        IbofEventId::Print(eventId, EventLevel::WARNING, additionalMessage);
        restoreSuccessful = false;
    }

    UnvramRestoreCompletion::WaitPendingUbioZero();

    if (fd >= 0)
    {
        close(fd);
        unlink(backupFileName);
    }
    return restoreSuccessful;
}

bool
UnvramBdev::_WrapupOpenDeviceSpecific(DeviceContext* deviceContext)
{
    bool restoreSuccessful = false;

    // Reactor cannot handle Async operation for Unvram in current implementation.
    // ioat poll cannot be called in Empty(), so, we restore the contents by IO worker.
    if (!EventFrameworkApi::IsReactorNow())
    {
        restoreSuccessful = _RecoverBackup(deviceContext);
    }
    _SetIoatReactor();

    return restoreSuccessful;
}

DeviceContext*
UnvramBdev::_AllocateDeviceContext(void)
{
    DeviceContext* deviceContext = new UramDeviceContext(GetName());
    return deviceContext;
}

void
UnvramBdev::_ReleaseDeviceContext(DeviceContext* deviceContextToRelease)
{
    if (nullptr != deviceContextToRelease)
    {
        UramDeviceContext* deviceContext =
            static_cast<UramDeviceContext*>(deviceContextToRelease);
        delete deviceContext;
    }
}

void
UnvramBdev::_SetIoatReactor(void)
{
    DeviceContext* devCtx = _GetDeviceContext();
    UramDeviceContext* uramDevCtx = static_cast<UramDeviceContext*>(devCtx);
    if (uramDevCtx->reactorCore != EventFrameworkApi::INVALID_CORE)
    {
        if (uramDevCtx->bdev_desc != nullptr)
        {
            ioatReactor[ioatReactorCount++] = uramDevCtx->reactorCore;
        }
        reactorCount++;
    }
    uint32_t totalReactor = AffinityManagerSingleton::Instance()->GetCoreCount(CoreType::REACTOR);
    if (useIoat == true && reactorCount == totalReactor)
    {
        uint32_t totalIoat = ioatInNuma0 + ioatInNuma1;
        if (totalIoat > ioatReactorCount)
        {
            useIoat = false;
            IBOF_TRACE_WARN(IBOF_EVENT_ID::IOAT_CONFIG_INVALID,
                "Number of IOAT HWs exceeds max count");
        }
    }
}

int
UnvramBdev::SubmitAsyncIO(UbioSmartPtr ubio)
{
    UramDeviceContext* devCtx =
        static_cast<UramDeviceContext*>(_GetDeviceContext());
    if (devCtx->bdev_desc == nullptr)
    {
        uint32_t ioatReactorIndex;
        if (useIoat == true && devCtx->reactorCore == EventFrameworkApi::INVALID_CORE)
        {
            ioatReactorIndex = requestCount % ioatInNuma1 + ioatInNuma0;
        }
        else
        {
            ioatReactorIndex = requestCount % ioatReactorCount;
        }
        uint32_t core = ioatReactor[ioatReactorIndex];
        requestCount++;

        UbioSmartPtr* ubioArgument = new UbioSmartPtr(ubio);
        EventFrameworkApi::SendSpdkEvent(
            core, _RequestAsyncIo, this, ubioArgument);
        return 1;
    }

    return UBlockDevice::SubmitAsyncIO(ubio);
}

void
UnvramBdev::_RequestAsyncIo(void* arg1, void* arg2)
{
    UnvramBdev* dev = static_cast<UnvramBdev*>(arg1);
    UbioSmartPtr ubio = *static_cast<UbioSmartPtr*>(arg2);

    dev->SubmitAsyncIO(ubio);
    delete static_cast<UbioSmartPtr*>(arg2);
}

} // namespace ibofos
