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

#include <air/Air.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "src/array/array.h"
#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_server.h"
#include "src/cli/grpc_cli_server.h"
#include "src/cpu_affinity/affinity_manager.h"
#include "src/cpu_affinity/affinity_viewer.h"
#include "src/signal_handler/deadlock_checker.h"
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
#include "src/lib/signal_mask.h"
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
#include "src/restore/restore_manager.h"
#include "src/signal_handler/signal_handler.h"
#include "src/signal_handler/user_signal_interface.h"
#include "src/spdk_wrapper/accel_engine_api.h"
#include "src/spdk_wrapper/spdk.h"
#include "src/telemetry/telemetry_air/telemetry_air_delegator.h"
#include "src/telemetry/telemetry_client/easy_telemetry_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/trace/otlp_factory.h"
#include "src/trace/trace_exporter.h"

namespace pos
{
PoseidonosInterface* PoseidonosInterface::instance;

int
Poseidonos::Init(int argc, char** argv)
{
    PoseidonosInterface::instance = this;
    POS_REPORT_TRACE(EID(POS_TRACE_INIT_START), "POS Initialize Sequence Start, total remaining steps: {}", total_init_seq_cnt);
    int ret = _LoadConfiguration();
    if (ret == 0)
    {
        POS_REPORT_TRACE(EID(POS_CONFIG_LOADED), "POS Initialize Sequence Loaded Config: {}", ConfigManagerSingleton::Instance()->RawData());
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_VERSION), "POS Initialize Sequence In Progress({}/{}): POS Version...", curr_init_seq_num, total_init_seq_cnt);
        _LoadVersion();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_SPDK), "POS Initialize Sequence In Progress({}/{}): SPDK...", curr_init_seq_num, total_init_seq_cnt);
        _InitSpdk(argc, argv);
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_SIG_HANDLER), "POS Initialize Sequence In Progress({}/{}): Signal Handler...", curr_init_seq_num, total_init_seq_cnt);
        // InitSpdk has signal handling initialize,
        // To give signal configuration from poseidonos,
        // InitSignalHandler should be done after InitSpdk.
        _InitSignalHandler();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_CPU_AFFINITY), "POS Initialize Sequence In Progress({}/{}): Affinity...", curr_init_seq_num, total_init_seq_cnt);
        _InitAffinity();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_THREAD_MODEL), "POS Initialize Sequence In Progress({}/{}): Thread Model...", curr_init_seq_num, total_init_seq_cnt);
        _SetupThreadModel();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_QOS_POLICY), "POS Initialize Sequence In Progress({}/{}): Perf Impact...", curr_init_seq_num, total_init_seq_cnt);
        _SetPerfImpact();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_DEBUG_MODULES), "POS Initialize Sequence In Progress({}/{}): Debug Info...", curr_init_seq_num, total_init_seq_cnt);
        _InitDebugInfo();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_AIR), "POS Initialize Sequence In Progress({}/{}): AIR...", curr_init_seq_num, total_init_seq_cnt);
        _InitAIR();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_IO_MODULES), "POS Initialize Sequence In Progress({}/{}): IO Interface...", curr_init_seq_num, total_init_seq_cnt);
        _InitIOInterface();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_MEMORY_CHECKER), "POS Initialize Sequence In Progress({}/{}): Memory Checker...", curr_init_seq_num, total_init_seq_cnt);
        _InitMemoryChecker();
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_RESOURCE_CHECKER), "POS Initialize Sequence In Progress({}/{}): Resource Checker...", curr_init_seq_num, total_init_seq_cnt);
        _InitResourceChecker();
#ifdef IBOF_CONFIG_REPLICATOR
        _InitReplicatorManager();
        POS_TRACE_INFO((EID(HA_DEBUG_MSG)), "ReplicatorManager is compiled with POS");
#else
        POS_TRACE_INFO((EID(HA_DEBUG_MSG)), "ReplicatorManager is excluded from POS. Skip initializing Replicator Manager");
#endif
        curr_init_seq_num++;
        POS_REPORT_TRACE(EID(POS_INITIALIZING_EXPORTER), "POS Initialize Sequence In Progress({}/{}): Trace Exporter...", curr_init_seq_num, total_init_seq_cnt);
        _InitTraceExporter(argv[0], pos::ConfigManagerSingleton::Instance(), pos::VersionProviderSingleton::Instance(), pos::TraceExporterSingleton::Instance(new OtlpFactory()));
    }
    else
    {
        POS_REPORT_TRACE(EID(POS_TRACE_INIT_FAIL), "POS Initialize Sequence Failed, LoadConfig Failed, {}", ConfigManagerSingleton::Instance()->RawData());
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

#ifdef IBOF_CONFIG_REPLICATOR
void
Poseidonos::_InitReplicatorManager(void)
{
    ConfigManager* configManager = ConfigManagerSingleton::Instance();
    std::string module("replicator");
    PosReplicatorManager* posReplicatorManager = PosReplicatorManagerSingleton::Instance();
    posReplicatorManager->Init(new GrpcPublisher(nullptr, configManager), new GrpcSubscriber(posReplicatorManager, configManager), configManager);
    POS_TRACE_WARN(EID(HA_DEBUG_MSG), "POS Replicator is disabled. Skip initializing ReplicatorManager");
}
#endif

void
Poseidonos::Run(void)
{
    _RestoreState();
    curr_init_seq_num++;
    POS_REPORT_TRACE(EID(POS_INITIALIZING_CLI_SERVER), "POS Initialize Sequence In Progress({}/{}): CLI Server...", curr_init_seq_num, total_init_seq_cnt);
    _RunCLIService();

    POS_REPORT_TRACE(EID(POS_TRACE_INIT_SUCCESS), "POS Initialize Sequence Success!");
    {
        std::unique_lock<std::mutex> lock(systemStopMutex);
        // Signal masking should be unmasked within systemStopMutex
        // to avoid signal processs happend right before systemStopWait happend.
        SignalMask::RestoreSignal(&oldSet);
        systemStopWait.wait(lock);
    }
    // Stop CLI Service if CLI server is still alive (For SystemStop command).
    _StopCLIService();
    IArrayMgmt* array = ArrayMgr();
    array->UnmountAllArrayAndStop();
}

void
Poseidonos::TriggerTerminate(void)
{
    systemStopWait.notify_all();
}
void
Poseidonos::Terminate(void)
{
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_START), "POS Terminate Sequence Start, total remaining steps: {}", total_term_seq_cnt);

    curr_term_seq_num++;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_PHASE_1), "POS Terminate Sequence In Progress({}/{}): Phase 1(Memory Checker & Singleton Group 1)...", curr_term_seq_num, total_term_seq_cnt);
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

    curr_term_seq_num++;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_PHASE_2), "POS Terminate Sequence In Progress({}/{}): Phase 2(Singleton Group 2)...", curr_term_seq_num, total_term_seq_cnt);
    delete singletonInfo;
    IOSubmitHandler* submitHandler = static_cast<IOSubmitHandler*>(IIOSubmitHandler::GetInstance());
    delete submitHandler;
    if (ioRecoveryEventFactory != nullptr)
    {
        delete ioRecoveryEventFactory;
    }
    ArrayManagerSingleton::ResetInstance();
    AccelEngineApi::Finalize();
    DeadLockCheckerSingleton::ResetInstance();
    SpdkCallerSingleton::Instance()->SpdkBdevPosUnRegisterPoller(UNVMfCompleteHandler);
    EventFrameworkApiSingleton::ResetInstance();
    SpdkSingleton::ResetInstance();
    IoTimeoutCheckerSingleton::ResetInstance();
    curr_term_seq_num++;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_PHASE_3), "POS Terminate Sequence In Progress({}/{}): Phase 3(AIR)...", curr_term_seq_num, total_term_seq_cnt);
    air_deactivate();
    POS_TRACE_INFO(EID(AIR_DEACTIVATE_SUCCEED), "");
    air_finalize();
    POS_TRACE_INFO(EID(AIR_FINALIZE_SUCCEED), "");
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

    curr_term_seq_num++;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_PHASE_4), "POS Terminate Sequence In Progress({}/{}): Phase 4(Signal Handler)...", curr_term_seq_num, total_term_seq_cnt);
    if (nullptr != signalHandler)
    {
        signalHandler->Deregister();
        UserSignalInterface::Enable(false);
    }
    SignalHandlerSingleton::ResetInstance();

    curr_term_seq_num++;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_PHASE_5), "POS Terminate Sequence In Progress({}/{}): Phase 5(Resource Checker)...", curr_term_seq_num, total_term_seq_cnt);
    ResourceCheckerSingleton::ResetInstance();

    curr_term_seq_num++;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_PHASE_6), "POS Terminate Sequence In Progress({}/{}): Phase 6(Singleton Group 3 & CLI Server)...", curr_term_seq_num, total_term_seq_cnt);
    SmartCollectorSingleton::ResetInstance();
    TraceExporterSingleton::ResetInstance();
    ConfigManagerSingleton::ResetInstance();
    VersionProviderSingleton::ResetInstance();
    delete GrpcCliServerThread;
    POS_REPORT_TRACE(EID(POS_TRACE_TERMINATE_SUCCESS), "POS Terminate Sequence Success!");
    PoseidonosInterface::instance = nullptr;
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
    POS_TRACE_INFO(EID(AIR_TARGET_CORE), "AIR Target Core: {}", std::to_string(air_target_core).c_str());
    air_initialize(air_target_core);
    POS_TRACE_INFO(EID(AIR_INITIALIZE_SUCCEED), "");
    air_activate();
    POS_TRACE_INFO(EID(AIR_ACTIVATE_SUCCEED), "");
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
        POS_TRACE_INFO(EID(AIR_REGISTER_EVENT_SUCCEED), "");
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
    singletonInfo = new SingletonInfo();
    singletonInfoUpdater = singletonInfo;
    singletonInfo->Update();
    int ret;
    ret = system("mkdir -p /etc/pos/core");
    if (ret != 0)
    {
        POS_TRACE_WARN(EID(DEBUG_CORE_DUMP_SETTING_FAILED), "Core directory will not be created");
        return;
    }
    ret = system("echo /etc/pos/core/%E.core > /proc/sys/kernel/core_pattern");
    if (ret != 0)
    {
        POS_TRACE_WARN(EID(DEBUG_CORE_DUMP_SETTING_FAILED), "Core pattern is not set properly");
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
    else
    {
        POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "DebugInfo: Failed to get a value of callback_timeout_sec from config.");
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
    else
    {
        POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "SignalHandler: Failed to get a value of user_signal_ignore_timeout_sec from config.");
    }
    // It should be released when CLI can be serviced.
    SignalMask::MaskQuitSignal(&oldSet);
}

void
Poseidonos::_SetupThreadModel(void)
{
    AffinityManager* affinityManager = pos::AffinityManagerSingleton::Instance();
    SpdkCallerSingleton::Instance()->SpdkBdevPosRegisterPoller(UNVMfCompleteHandler);
    DeadLockCheckerSingleton::Instance()->RunDeadLockChecker();
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

    cpu_set_t generalCPUSet = affinityManager->GetCpuSet(CoreType::GENERAL_USAGE);
    EasyTelemetryPublisherSingleton::Instance()->Initialize(ConfigManagerSingleton::Instance(), generalCPUSet);
}

void
Poseidonos::_RestoreState(void)
{
    bool enabled = false;
    ConfigManagerSingleton::Instance()->GetValue("save_restore", "enable", &enabled, ConfigType::CONFIG_TYPE_BOOL);
    if (enabled)
    {
        if (RestoreManagerSingleton::Instance()->Restore())
        {
            cout << "The previous state has been restored.\n";
            RestoreManagerSingleton::Instance()->EnableStateSave();
            return;
        }
        else
        {
            cout << "Faild to restore previous state.\n";
        }
    }
    POS_TRACE_INFO(EID(RESTORE_MSG), "Saving state disabled.");
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
        POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "MemoryChecker: Failed to get a value of stack_trace_for_previous_owner from config.");
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
        POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "MemoryChecker: Failed to get a value of memory_checker from config.");
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
    else
    {
        POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "ResourceChecker has null instance");
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
    else
    {
        POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "Failed to read POS configuration file");
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
            POS_TRACE_INFO(EID(QOS_SET_EVENT_POLICY),
                "Rebuild Perf Impact not supported, Set to default low");
        }
        newRebuildPolicy.policyChange = true;
        retVal = QosManagerSingleton::Instance()->UpdateBackendPolicy(BackendEvent_UserdataRebuild, newRebuildPolicy);
        if (retVal != SUCCESS)
        {
            POS_TRACE_WARN(EID(POS_INIT_EXCEPTIONS), "Failed to set Rebuild Policy");
        }
    }
}

void
Poseidonos::_RunCLIService(void)
{
    std::lock_guard<std::mutex> lock(cliMutex);
    cliEnabled = true;
    pos_cli::CLIServerMain();
    GrpcCliServerThread = new std::thread(RunGrpcServer);
}

void
Poseidonos::_StopCLIService(void)
{
    std::lock_guard<std::mutex> lock(cliMutex);
    if (cliEnabled == true)
    {
        if (GrpcCliServerThread != nullptr)
        {
            ShutdownGrpcServer();
            GrpcCliServerThread->join();
        }
        pos_cli::Exit();
        cliEnabled = false;
    }
}

int
Poseidonos::_InitTraceExporter(char* procFullName,
    ConfigManager* cm,
    VersionProvider* vp,
    TraceExporter* te)
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

    while (std::getline(ss, token, '/'))
    {
        serviceName = token;
    }

    // Set service version
    std::string serviceVersion = vp->GetVersion();

    // Initialize trace exporter
    te->Init(serviceName, serviceVersion, traceEndPoint);

    if (te->IsEnabled())
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