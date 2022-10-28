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

#include "poseidonos.h"

#include <iostream>
#include <air/Air.h>

#include <string>
#include <vector>
#include <sstream>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_server.h"
#include "src/cli/grpc_cli_server.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/cpu_affinity/affinity_viewer.h"
#include "src/device/device_manager.h"
#include "src/event_scheduler/event.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/event_scheduler/io_completer.h"
#include "src/event_scheduler/io_timeout_checker.h"
#include "src/include/pos_event_id.h"
#include "src/io/frontend_io/flush_command_manager.h"
#include "src/io/frontend_io/unvmf_io_handler.h"
#include "src/io/general_io/io_recovery_event_factory.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/io_scheduler/io_dispatcher.h"
#include "src/io_submit_interface/i_io_submit_handler.h"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"
#include "src/master_context/version_provider.h"
#include "src/metafs/include/metafs_service.h"
#include "src/network/nvmf_target.h"
#include "src/network/transport_configuration.h"
#include "src/pos_replicator/grpc_publisher.h"
#include "src/pos_replicator/grpc_subscriber.h"
#include "src/pos_replicator/posreplicator_manager.h"
#include "src/qos/qos_manager.h"
#include "src/resource_checker/resource_checker.h"
#include "src/resource_checker/smart_collector.h"
#include "src/signal_handler/signal_handler.h"
#include "src/signal_handler/user_signal_interface.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/spdk_wrapper/spdk.h"
#include "src/telemetry/telemetry_air/telemetry_air_delegator.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/resource_checker/resource_checker.h"
#include "src/resource_checker/smart_collector.h"
#include "src/trace/trace_exporter.h"
#include "src/trace/otlp_factory.h"

namespace pos
{
int
Poseidonos::Init(int argc, char** argv)
{
    POS_TRACE_TRACE(EID(POS_TRACE_STARTED), "");
    int ret = _LoadConfiguration();
    if (ret == 0)
    {
        _InitSignalHandler();
        _LoadVersion();
        _InitSpdk(argc, argv);
        _InitAffinity();
        _SetupThreadModel();
        _SetPerfImpact();
        _InitDebugInfo();
        _InitAIR();
        _InitIOInterface();
        _InitMemoryChecker();
        _InitResourceChecker();
#ifdef WITH_REPLICATOR
        _InitReplicatorManager();
#endif
        _InitTraceExporter(argv[0], pos::ConfigManagerSingleton::Instance(), pos::VersionProviderSingleton::Instance(), pos::TraceExporterSingleton::Instance(new OtlpFactory()));
    }
    else
    {
        POS_TRACE_TRACE(EID(POS_TRACE_INIT_FAIL), "{}", ConfigManagerSingleton::Instance()->RawData());
    }
    return ret;
}

void
Poseidonos::_InitIOInterface(void)
{
    IIOSubmitHandler* submitHandler = new IOSubmitHandler;
    IIOSubmitHandler::RegisterInstance(submitHandler);

    ioRecoveryEventFactory = new IoRecoveryEventFactory();
    IoCompleter::RegisterRecoveryEventFactory(ioRecoveryEventFactory);
    IODispatcher::RegisterRecoveryEventFactory(ioRecoveryEventFactory);
}

#ifdef WITH_REPLICATOR
void
Poseidonos::_InitReplicatorManager(void)
{
    PosReplicatorManager* posReplicatorManager = PosReplicatorManagerSingleton::Instance();
    posReplicatorManager->Init(new GrpcPublisher(nullptr, ConfigManagerSingleton::Instance()), new GrpcSubscriber(ConfigManagerSingleton::Instance()));
}
#endif

void
Poseidonos::Run(void)
{
    _RunCLIService();
    POS_TRACE_TRACE(EID(POS_TRACE_INIT_SUCCESS), "{}", ConfigManagerSingleton::Instance()->RawData());
    pos_cli::Wait();
}

void
Poseidonos::Terminate(void)
{
    POS_TRACE_TRACE(EID(POS_TRACE_TERMINATING), "");
    MemoryChecker::Enable(false);
    EventSchedulerSingleton::Instance()->SetTerminate(true);
    NvmfTargetSingleton::ResetInstance();
    DeviceManagerSingleton::ResetInstance();
    IODispatcherSingleton::ResetInstance();
    EventSchedulerSingleton::ResetInstance();
    QosManagerSingleton::Instance()->FinalizeSpdkManager();
    QosManagerSingleton::ResetInstance();
    FlushCmdManagerSingleton::ResetInstance();
    SmartLogMgrSingleton::ResetInstance();
    delete debugInfo;
    IOSubmitHandler* submitHandler = static_cast<IOSubmitHandler*>(IIOSubmitHandler::GetInstance());
    delete submitHandler;
    if (ioRecoveryEventFactory != nullptr)
    {
        delete ioRecoveryEventFactory;
    }
    ArrayManagerSingleton::ResetInstance();
    AccelEngineApi::Finalize();
    SpdkCallerSingleton::Instance()->SpdkBdevPosUnRegisterPoller(UNVMfCompleteHandler);
    EventFrameworkApiSingleton::ResetInstance();
    SpdkSingleton::ResetInstance();

    IoTimeoutCheckerSingleton::ResetInstance();

    air_deactivate();
    air_finalize();
    if (nullptr != telemetryAirDelegator)
    {
        telemetryAirDelegator->SetState(TelemetryAirDelegator::State::END);
        delete telemetryAirDelegator;
        telemetryAirDelegator = nullptr;
    }
    if (nullptr != telemtryPublisherForAir)
    {
        telemtryPublisherForAir->StopPublishing();
        TelemetryClientSingleton::Instance()->DeregisterPublisher(telemtryPublisherForAir->GetName());
        delete telemtryPublisherForAir;
        telemtryPublisherForAir = nullptr;
    }
    if (nullptr != signalHandler)
    {
        signalHandler->Deregister();
        UserSignalInterface::Enable(false);
    }
    SignalHandlerSingleton::ResetInstance();
    ResourceCheckerSingleton::ResetInstance();
    SmartCollectorSingleton::ResetInstance();

    TraceExporterSingleton::ResetInstance();
    ConfigManagerSingleton::ResetInstance();
    VersionProviderSingleton::ResetInstance();

    free(GrpcCliServerThread);

    POS_TRACE_TRACE(EID(POS_TRACE_TERMINATED), "");
}

void
Poseidonos::_InitAIR(void)
{
    cpu_set_t air_cpu_set = pos::AffinityManagerSingleton::Instance()->GetCpuSet(pos::CoreType::AIR);
    long nproc = sysconf(_SC_NPROCESSORS_ONLN);
    unsigned int air_target_core = 0;
    for (long i = 0; i < nproc; i++)
    {
        if (1 == CPU_ISSET(i, &air_cpu_set))
        {
            std::cout << "CPU ID: " << i << "       Usage: AIR\n";
            air_target_core = (unsigned int)i;
            break;
        }
    }
    air_initialize(air_target_core);
    air_activate();

    if (nullptr == telemtryPublisherForAir)
    {
        telemtryPublisherForAir = new TelemetryPublisher{"air_delegator"};
        telemtryPublisherForAir->StartPublishing();
        TelemetryClientSingleton::Instance()->RegisterPublisher(telemtryPublisherForAir);
    }
    if (nullptr == telemetryAirDelegator)
    {
        telemetryAirDelegator = new TelemetryAirDelegator{telemtryPublisherForAir};
        telemetryAirDelegator->RegisterAirEvent();
    }
}

void
Poseidonos::_InitAffinity(void)
{
    pos::AffinityViewer::Print();
    pos::AffinityManagerSingleton::Instance()->SetGeneralAffinitySelf();
}

void
Poseidonos::_InitSpdk(int argc, char** argv)
{
    pos::SpdkSingleton::Instance()->Init(argc, argv);
    TransportConfiguration transportConfig;
    transportConfig.CreateTransport();
}

void
Poseidonos::_InitDebugInfo(void)
{
    debugInfo = new DebugInfo();
    debugInfoUpdater = debugInfo;
    debugInfo->Update();
    int ret;
    ret = system("mkdir -p /etc/pos/core");
    if (ret != 0)
    {
        POS_TRACE_DEBUG(EID(DEBUG_CORE_DUMP_SETTING_FAILED), "Core directory will not be created");
        return;
    }
    ret = system("echo /etc/pos/core/%E.core > /proc/sys/kernel/core_pattern");
    if (ret != 0)
    {
        POS_TRACE_DEBUG(EID(DEBUG_CORE_DUMP_SETTING_FAILED), "Core pattern is not set properly");
        return;
    }

    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("debug");
    uint64_t timeout = false;
    ret = configManager.GetValue(module, "callback_timeout_sec", &timeout,
        CONFIG_TYPE_UINT64);
    if (ret == EID(SUCCESS))
    {
        Callback::SetTimeout(timeout);
    }
}

void
Poseidonos::_InitSignalHandler(void)
{
    signalHandler = SignalHandlerSingleton::Instance();
    signalHandler->Register();
    UserSignalInterface::Enable(true);

    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("debug");
    uint64_t timeout = false;
    int ret = configManager.GetValue(module, "user_signal_ignore_timeout_sec", &timeout,
        CONFIG_TYPE_UINT64);
    if (ret == EID(SUCCESS))
    {
        UserSignalInterface::SetTimeout(timeout);
    }
}

void
Poseidonos::_SetupThreadModel(void)
{
    AffinityManager* affinityManager = pos::AffinityManagerSingleton::Instance();
    SpdkCallerSingleton::Instance()->SpdkBdevPosRegisterPoller(UNVMfCompleteHandler);
    POS_TRACE_DEBUG(EID(DEVICEMGR_SETUPMODEL), "_SetupThreadModel");
    uint32_t coreCount =
        affinityManager->GetCoreCount(CoreType::EVENT_WORKER);
    uint32_t workerCount = coreCount * EVENT_THREAD_CORE_RATIO;
    cpu_set_t schedulerCPUSet =
        affinityManager->GetCpuSet(CoreType::EVENT_SCHEDULER);
    cpu_set_t workerCPUSet = affinityManager->GetCpuSet(CoreType::EVENT_WORKER);

    QosManagerSingleton::Instance()->InitializeSpdkManager();
    QosManagerSingleton::Instance()->Initialize();
    EventSchedulerSingleton::Instance()->Initialize(workerCount,
        schedulerCPUSet, workerCPUSet);
    IIODispatcher* ioDispatcher = IODispatcherSingleton::Instance();
    DeviceManagerSingleton::Instance()->Initialize(ioDispatcher);

    coreCount = affinityManager->GetTotalCore();
    schedulerCPUSet = affinityManager->GetCpuSet(CoreType::META_SCHEDULER);
    workerCPUSet = affinityManager->GetCpuSet(CoreType::META_IO);
    MetaFsServiceSingleton::Instance()->Initialize(coreCount,
        schedulerCPUSet, workerCPUSet);
    FlushCmdManagerSingleton::Instance();

    IoTimeoutCheckerSingleton::Instance()->Initialize();
}

void
Poseidonos::_InitMemoryChecker(void)
{
    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("debug");
    bool enabled = false;
    int ret = configManager.GetValue(module, "stack_trace_for_previous_owner", &enabled,
        CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        MemoryChecker::EnableStackTrace(enabled);
    }
    else
    {
        // default true
        MemoryChecker::EnableStackTrace(true);
    }
    ret = configManager.GetValue(module, "memory_checker", &enabled,
        CONFIG_TYPE_BOOL);
    if (ret == EID(SUCCESS))
    {
        MemoryChecker::Enable(enabled);
    }
    else
    {
        MemoryChecker::Enable(false);
    }
}

void
Poseidonos::_InitResourceChecker(void)
{
    ResourceChecker* resourceChecker = ResourceCheckerSingleton::Instance();
    if (nullptr != resourceChecker)
    {
        resourceChecker->Enable();
    }
}

int
Poseidonos::_LoadConfiguration(void)
{
    int ret = ConfigManagerSingleton::Instance()->ReadFile();
    if (ret == EID(CONFIG_FILE_READ_DONE))
    {
        return 0;
    }
    return ret;
}

void
Poseidonos::_LoadVersion(void)
{
    std::string version =
        pos::VersionProviderSingleton::Instance()->GetVersion();
    POS_TRACE_INFO(EID(SYSTEM_VERSION_LOAD_SUCCESS),
        "POS Version {}", version.c_str());
}

void
Poseidonos::_SetPerfImpact(void)
{
    int retVal = -1;
    std::string impact = "";
    int ret = ConfigManagerSingleton::Instance()->GetValue("perf_impact", "rebuild",
        &impact, ConfigType::CONFIG_TYPE_STRING);
    if (ret == EID(SUCCESS))
    {
        qos_backend_policy newRebuildPolicy;
        if (impact.compare("high") == 0)
        {
            newRebuildPolicy.priorityImpact = PRIORITY_HIGH;
        }
        else if (impact.compare("medium") == 0)
        {
            newRebuildPolicy.priorityImpact = PRIORITY_MEDIUM;
        }
        else if (impact.compare("low") == 0)
        {
            newRebuildPolicy.priorityImpact = PRIORITY_LOW;
        }
        else
        {
            newRebuildPolicy.priorityImpact = PRIORITY_LOW;
            POS_TRACE_INFO(static_cast<uint32_t>(EID(QOS_SET_EVENT_POLICY)),
                "Rebuild Perf Impact not supported, Set to default lowest");
        }
        newRebuildPolicy.policyChange = true;
        retVal = QosManagerSingleton::Instance()->UpdateBackendPolicy(BackendEvent_UserdataRebuild, newRebuildPolicy);
        if (retVal != SUCCESS)
        {
            POS_TRACE_INFO(static_cast<uint32_t>(EID(QOS_SET_EVENT_POLICY)),
                "Failed to set Rebuild Policy");
        }
    }
}

void
Poseidonos::_RunCLIService(void)
{
    pos_cli::CLIServerMain();
    GrpcCliServerThread = new std::thread(RunGrpcServer);
}

int
Poseidonos::_InitTraceExporter(char *procFullName,
    ConfigManager *cm,
    VersionProvider *vp,
    TraceExporter *te
    )
{
    bool traceEnabled = false;
    int ret = cm->GetValue("trace", "enable", &traceEnabled, ConfigType::CONFIG_TYPE_BOOL);

    if (EID(SUCCESS) != ret)
    {
        POS_TRACE_INFO(EID(TRACE_CONFIG_ERROR), "Specify whether the trace should be enabled or not in configuration");
        return EID(TRACE_CONFIG_ERROR);
    }

    if (true != traceEnabled)
    {
        POS_TRACE_INFO(EID(TRACE_NOT_ENABLED), "The trace is not enabled by configuration");
        return EID(TRACE_NOT_ENABLED);
    }
    
    std::string traceEndPoint = "";
    ret = cm->GetValue("trace", "collector_endpoint", &traceEndPoint, ConfigType::CONFIG_TYPE_STRING);

    if (EID(SUCCESS) != ret)
    {
        POS_TRACE_INFO(EID(TRACE_CONFIG_ERROR), "Trace is not enabled. Specify an endpoint of the traces in configuration");
        return EID(TRACE_CONFIG_ERROR);
    }
    
    // Set service name
    std::stringstream ss(procFullName);
    std::string token, serviceName;

    while(std::getline(ss, token, '/')) {
        serviceName = token;
    }

    // Set service version
    std::string serviceVersion = vp->GetVersion();

    // Initialize trace exporter
    te->Init(serviceName, serviceVersion, traceEndPoint);

    if(te->IsEnabled())
    {
        POS_TRACE_INFO(EID(TRACE_ENABLED), "Trace is enabled. Traces generated by {} will be exported to {}", serviceName + " " + serviceVersion, traceEndPoint);
        return EID(SUCCESS);
    }
    else
    {
        POS_TRACE_INFO(EID(TRACE_NOT_ENABLED), "Trace exporter is not enabled.");
        return EID(TRACE_NOT_ENABLED);
    }
}
} // namespace pos