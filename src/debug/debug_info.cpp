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

#include "src/debug/debug_info.h"

#include <vector>

#include "src/allocator/allocator.h"
#include "src/array/array.h"
#include "src/device/device_manager.h"
#include "src/device/spdk/spdk.hpp"
#include "src/device/unvme/unvme_drv.h"
#include "src/device/unvram/unvram_drv.h"
#include "src/dump/dump_manager.h"
#include "src/gc/garbage_collector.h"
#include "src/io/frontend_io/aio.h"
#include "src/io/frontend_io/flush_command_manager.h"
#include "src/io/general_io/affinity_manager.h"
#include "src/io/general_io/command_timeout_handler.h"
#include "src/io/general_io/rba_state_manager.h"
#include "src/journal_manager/journal_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/master_context/config_manager.h"
#include "src/master_context/mbr_manager.h"
#include "src/master_context/version_provider.h"
#include "src/metafs/mvm/meta_vol_mgr.h"
#include "src/qos/qos_manager.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/event_scheduler.h"
#include "src/scheduler/io_dispatcher.h"
#include "src/state/state_manager.h"
#include "src/state/state_policy.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/volume/volume_manager.h"

namespace ibofos
{

DebugInfo *debugInfo;

DebugInfo::DebugInfo(void)
:   affinityManager(nullptr),
    allocator(nullptr),
    array(nullptr),
    commandTimeoutHandler(nullptr),
    configManager(nullptr),
    deviceManager(nullptr),
    dumpManager(nullptr),
    eventScheduler(nullptr),
#if defined NVMe_FLUSH_HANDLING
    flushCmdManager(nullptr),
#endif
    garbageCollector(nullptr),
    ioDispatcher(nullptr),
    journalManager(nullptr),
    logger(nullptr),
    mapper(nullptr),
    mbrManager(nullptr),
    metaVolManager(nullptr),
    qosManager(nullptr),
    rbaStateManager(nullptr),
    reporter(nullptr),
    spdk(nullptr),
    stateManager(nullptr),
    statePolicy(nullptr),
    unvmeDrv(nullptr),
    unvramDrv(nullptr),
    versionProvider(nullptr),
    volumeEventPublisher(nullptr),
    volumeManager(nullptr)
{
}

void
DebugInfo::UpdateModulePointer(void)
{
    affinityManager = AffinityManagerSingleton::Instance();
    deviceManager = DeviceManagerSingleton::Instance();
    reporter = ReporterSingleton::Instance();
    logger = LoggerSingleton::Instance();
    dumpManager = DumpManagerSingleton::Instance();
    volumeManager = VolumeManagerSingleton::Instance();
    mapper = MapperSingleton::Instance();
    stateManager = StateManagerSingleton::Instance();
    statePolicy = StatePolicySingleton::Instance();
    array = ArraySingleton::Instance();
    garbageCollector = GarbageCollectorSingleton::Instance();
    allocator = AllocatorSingleton::Instance();
    mbrManager = MbrManagerSingleton::Instance();
    unvramDrv = UnvramDrvSingleton::Instance();
    unvmeDrv = UnvmeDrvSingleton::Instance();
    journalManager = JournalManagerSingleton::Instance();
    configManager = ConfigManagerSingleton::Instance();
    spdk = SpdkSingleton::Instance();
    rbaStateManager = RbaStateManagerSingleton::Instance();
    volumeEventPublisher = VolumeEventPublisherSingleton::Instance();
    eventScheduler = EventArgument::GetEventScheduler();
    ioDispatcher = EventArgument::GetIODispatcher();
    qosManager = QosManagerSingleton::Instance();
#if defined NVMe_FLUSH_HANDLING
    flushCmdManager = FlushCmdManagerSingleton::Instance();
#endif
    versionProvider = VersionProviderSingleton::Instance();
    commandTimeoutHandler = CommandTimeoutHandlerSingleton::Instance();

    metaVolManager = &metaVolMgr;
}
} // namespace ibofos
