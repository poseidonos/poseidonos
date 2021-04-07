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

#include "ibofos.h"

#include <string>

#include "Air.h"
#include "src/cli/cli_server.h"
#include "src/device/device_manager.h"
#include "src/device/spdk/spdk.hpp"
#include "src/include/ibof_event_id.h"
#include "src/io/general_io/affinity_manager.h"
#include "src/io/general_io/affinity_viewer.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/master_context/version_provider.h"
#include "src/network/nvmf_target_event_subscriber.hpp"
#include "src/scheduler/event.h"
#include "src/scheduler/scheduler_api.h"
#if defined QOS_ENABLED_BE
#include "src/qos/qos_manager.h"
#endif

namespace ibofos
{
void
Ibofos::Init(int argc, char** argv)
{
    _LoadConfiguration();
    _LoadVersion();
#if defined QOS_ENABLED_BE
    _SetPerfImpact();
#endif
    _InitSpdk(argc, argv);
    _InitAffinity();
    _InitDebugInfo();
    _InitMemoryChecker();
}

void
Ibofos::Run()
{
    _RunCLIService();
    ibofos_cli::Wait();
}

void
Ibofos::Terminate()
{
    MemoryChecker::Enable(false);
    DeviceManagerSingleton::ResetInstance();
    delete debugInfo;

    AIR_DEACTIVATE();
    AIR_FINALIZE();
}

void
Ibofos::_InitAffinity()
{
    ibofos::AffinityViewer::Print();
    ibofos::AffinityManagerSingleton::Instance()->SetGeneralAffinitySelf();

    cpu_set_t general_core = ibofos::AffinityManagerSingleton::Instance()
                                 ->GetCpuSet(ibofos::CoreType::GENERAL_USAGE);
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    for (long i = 0; i < nproc; i++)
    {
        if (1 == CPU_ISSET(i, &general_core))
        {
            std::cout << "CPU ID: " << i << "       Usage: AIR\n";
            AIR_INITIALIZE((unsigned int)i);
            AIR_ACTIVATE();
            break;
        }
    }
}

void
Ibofos::_InitSpdk(int argc, char** argv)
{
    ibofos::SpdkSingleton::Instance()->Init(argc, argv);
}

void
Ibofos::_InitDebugInfo()
{
    debugInfo = new DebugInfo();
    debugInfo->UpdateModulePointer();
}

void
Ibofos::_InitMemoryChecker(void)
{
    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("debug");
    bool enabled = false;
    int ret = configManager.GetValue(module, "memory_checker", &enabled,
        CONFIG_TYPE_BOOL);
    if (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS))
    {
        MemoryChecker::Enable(enabled);
    }
    else
    {
        MemoryChecker::Enable(false);
    }
}

void
Ibofos::_LoadConfiguration()
{
    ConfigManagerSingleton::Instance()->ReadFile();
}

void
Ibofos::_LoadVersion()
{
    std::string version =
        ibofos::VersionProviderSingleton::Instance()->GetVersion();
    IBOF_TRACE_INFO(static_cast<uint32_t>(IBOF_EVENT_ID::SYSTEM_VERSION),
        "POS Version {}", version.c_str());
}

#if defined QOS_ENABLED_BE
void
Ibofos::_SetPerfImpact()
{
    {
        std::string impact = "";
        std::string event = "gc";
        int ret = ConfigManagerSingleton::Instance()->GetValue("perf_impact", event,
            &impact, ConfigType::CONFIG_TYPE_STRING);
        if (ret == (int)IBOF_EVENT_ID::SUCCESS)
        {
            QosManagerSingleton::Instance()->SetEventPolicy(event, impact);
        }
    }

    {
        std::string impact = "";
        std::string event = "rebuild";
        int ret = ConfigManagerSingleton::Instance()->GetValue("perf_impact", event,
            &impact, ConfigType::CONFIG_TYPE_STRING);
        if (ret == (int)IBOF_EVENT_ID::SUCCESS)
        {
            QosManagerSingleton::Instance()->SetEventPolicy(event, impact);
        }
    }
}
#endif

void
Ibofos::_RunCLIService()
{
    ibofos_cli::CLIServerMain();
}

} // namespace ibofos
