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

#include "poseidonos.h"

#include <string>

#include "Air.h"
#include "src/cli/cli_server.h"
#include "src/device/device_manager.h"
#include "src/spdk_wrapper/spdk.hpp"
#include "src/include/pos_event_id.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/cpu_affinity/affinity_viewer.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/master_context/version_provider.h"
#include "src/network/nvmf_target_event_subscriber.h"
#include "src/event_scheduler/event.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/io/general_io/io_recovery_event_factory.h"
#include "src/event_scheduler/io_completer.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/qos/qos_manager.h"
#include "src/metafs/include/metafs_service.h"
#include "src/array_mgmt/array_manager.h"

namespace pos
{
void
Poseidonos::Init(int argc, char** argv)
{
    _LoadConfiguration();
    _LoadVersion();
    _InitSpdk(argc, argv);
    _InitAffinity();
    _SetupThreadModel();
    _SetPerfImpact();
    _InitDebugInfo();
    _InitIOInterface();
    _InitMemoryChecker();
}

void
Poseidonos::_InitIOInterface(void)
{
    IIOSubmitHandler *submitHandler = new IOSubmitHandler;
    IIOSubmitHandler::RegisterInstance(submitHandler);

    ioRecoveryEventFactory = new IoRecoveryEventFactory();
    IoCompleter::RegisterRecoveryEventFactory(ioRecoveryEventFactory);
    IODispatcher::RegisterRecoveryEventFactory(ioRecoveryEventFactory);
}

void
Poseidonos::Run(void)
{
    _RunCLIService();
    pos_cli::Wait();
}

void
Poseidonos::Terminate(void)
{
    MemoryChecker::Enable(false);
    DeviceManagerSingleton::ResetInstance();
    IODispatcherSingleton::ResetInstance();
    EventSchedulerSingleton::ResetInstance();
    delete debugInfo;
    IOSubmitHandler *submitHandler = static_cast<IOSubmitHandler *>(IIOSubmitHandler::GetInstance());
    delete submitHandler;
    if (ioRecoveryEventFactory != nullptr)
    {
        delete ioRecoveryEventFactory;
    }
    ArrayMgr::ResetInstance();
    EventFrameworkApiSingleton::ResetInstance();

    air_deactivate();
    air_finalize();
}

void
Poseidonos::_InitAffinity(void)
{
    pos::AffinityViewer::Print();
    pos::AffinityManagerSingleton::Instance()->SetGeneralAffinitySelf();

    cpu_set_t general_core = pos::AffinityManagerSingleton::Instance()
                                 ->GetCpuSet(pos::CoreType::GENERAL_USAGE);
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    for (long i = 0; i < nproc; i++)
    {
        if (1 == CPU_ISSET(i, &general_core))
        {
            std::cout << "CPU ID: " << i << "       Usage: AIR\n";
            air_initialize((unsigned int)i);
            air_activate();
            break;
        }
    }
}

void
Poseidonos::_InitSpdk(int argc, char** argv)
{
    pos::SpdkSingleton::Instance()->Init(argc, argv);
}

void
Poseidonos::_InitDebugInfo(void)
{
    debugInfo = new DebugInfo();
    debugInfoUpdater = debugInfo;
    debugInfo->Update();
}

void
Poseidonos::_SetupThreadModel(void)
{
    AffinityManager* affinityManager = pos::AffinityManagerSingleton::Instance();
    POS_TRACE_DEBUG(POS_EVENT_ID::DEVICEMGR_SETUPMODEL, "_SetupThreadModel");
    uint32_t coreCount =
        affinityManager->GetCoreCount(CoreType::EVENT_WORKER);
    uint32_t workerCount = coreCount * EVENT_THREAD_CORE_RATIO;
    cpu_set_t schedulerCPUSet =
        affinityManager->GetCpuSet(CoreType::EVENT_SCHEDULER);
    cpu_set_t workerCPUSet = affinityManager->GetCpuSet(CoreType::EVENT_WORKER);

    EventSchedulerSingleton::Instance()->Initialize(workerCount,
            schedulerCPUSet, workerCPUSet);
    IIODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
    DeviceManagerSingleton::Instance()->Initialize(ioDispatcher);

    coreCount = affinityManager->GetTotalCore();
    schedulerCPUSet = affinityManager->GetCpuSet(CoreType::META_SCHEDULER);
    workerCPUSet = affinityManager->GetCpuSet(CoreType::META_IO);
    MetaFsServiceSingleton::Instance()->Initialize(coreCount,
            schedulerCPUSet, workerCPUSet);
}

void
Poseidonos::_InitMemoryChecker(void)
{
    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("debug");
    bool enabled = false;
    int ret = configManager.GetValue(module, "memory_checker", &enabled,
        CONFIG_TYPE_BOOL);
    if (ret == static_cast<int>(POS_EVENT_ID::SUCCESS))
    {
        MemoryChecker::Enable(enabled);
    }
    else
    {
        MemoryChecker::Enable(false);
    }
}

void
Poseidonos::_LoadConfiguration(void)
{
    ConfigManagerSingleton::Instance()->ReadFile();
}

void
Poseidonos::_LoadVersion(void)
{
    std::string version =
        pos::VersionProviderSingleton::Instance()->GetVersion();
    POS_TRACE_INFO(static_cast<uint32_t>(POS_EVENT_ID::SYSTEM_VERSION),
        "POS Version {}", version.c_str());
}

void
Poseidonos::_SetPerfImpact(void)
{
    int retVal = -1;
    std::string impact = "";
    int ret = ConfigManagerSingleton::Instance()->GetValue("perf_impact", "rebuild",
            &impact, ConfigType::CONFIG_TYPE_STRING);
    if (ret == (int)POS_EVENT_ID::SUCCESS)
    {
        qos_rebuild_policy newRebuildPolicy;
        if (impact.compare("high") == 0)
        {
            newRebuildPolicy.rebuildImpact = PRIORITY_HIGH;
        }
        else if (impact.compare("low") == 0)
        {
            newRebuildPolicy.rebuildImpact = PRIORITY_LOW;
        }
        else
        {
            newRebuildPolicy.rebuildImpact = PRIORITY_LOW;
            POS_TRACE_INFO(static_cast<uint32_t>(POS_EVENT_ID::QOS_SET_EVENT_POLICY),
                    "Rebuild Perf Impact not supported, Set to default Low");
        }
        newRebuildPolicy.policyChange = true;
        retVal = QosManagerSingleton::Instance()->UpdateRebuildPolicy(newRebuildPolicy);
        if (retVal != SUCCESS)
        {
            POS_TRACE_INFO(static_cast<uint32_t>(POS_EVENT_ID::QOS_SET_EVENT_POLICY),
                    "Failed to set Rebuild Policy");
        }
    }
}

void
Poseidonos::_RunCLIService(void)
{
    pos_cli::CLIServerMain();
}

} // namespace pos
