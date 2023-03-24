#include "src/array/array.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_mgmt/numa_awared_array_creation.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/cli_server.h"
#include "src/cli/command_processor.h"
#include "src/cli/request_handler.h"
#include "src/device/device_manager.h"
#include "src/event/event_manager.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/include/array_config.h"
#include "src/include/nvmf_const.h"
#include "src/include/poseidonos_interface.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
#include "src/logger/logger.h"
#include "src/logger/preferences.h"
#include "src/master_context/version_provider.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/resource_checker/smart_collector.h"
#include "src/restore/restore_manager.h"
#include "src/sys_info/space_info.h"


grpc::Status
CommandProcessor::ExecuteSystemInfoCommand(const SystemInfoRequest* request, SystemInfoResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    reply->mutable_result()->mutable_data()->set_version(version);

    BiosInfo biosInfo = _GetBiosInfo();
    reply->mutable_result()->mutable_data()->set_biosvendor(biosInfo.vendor);
    reply->mutable_result()->mutable_data()->set_biosversion(biosInfo.version);
    reply->mutable_result()->mutable_data()->set_biosreleasedate(biosInfo.releaseDate);

    SystemInfo systemInfo = _GetSystemInfo();
    reply->mutable_result()->mutable_data()->set_systemmanufacturer(systemInfo.manufacturer);
    reply->mutable_result()->mutable_data()->set_systemproductname(systemInfo.productName);
    reply->mutable_result()->mutable_data()->set_systemserialnumber(systemInfo.serialNumber);
    reply->mutable_result()->mutable_data()->set_systemuuid(systemInfo.uuid);

    BaseboardInfo baseboardInfo = _GetBaseboardInfo();
    reply->mutable_result()->mutable_data()->set_baseboardmanufacturer(baseboardInfo.manufacturer);
    reply->mutable_result()->mutable_data()->set_baseboardproductname(baseboardInfo.productName);
    reply->mutable_result()->mutable_data()->set_baseboardserialnumber(baseboardInfo.serialNumber);
    reply->mutable_result()->mutable_data()->set_baseboardversion(baseboardInfo.version);

    ProcessorInfo proccessorInfo = _GetProcessorInfo();
    reply->mutable_result()->mutable_data()->set_processormanufacturer(proccessorInfo.manufacturer);
    reply->mutable_result()->mutable_data()->set_processorversion(proccessorInfo.version);
    reply->mutable_result()->mutable_data()->set_processorfrequency(proccessorInfo.frequency);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());

    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteStopSystemCommand(const StopSystemRequest* request, StopSystemResponse* reply)
{
    POS_REPORT_TRACE(EID(POS_SYSTEM_STOP_EXE), "system stop CLI is executing...");
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    Status status = grpc::Status::OK;
    int eventId = 0;
    if (!_IsPosTerminating())
    {
        _SetPosTerminating(true);
        POS_REPORT_TRACE(EID(POS_TERMINATION_TRIGGERED), "Entering a termination process...");
        PoseidonosInterface* interface = PoseidonosInterface::GetInterface();
        if (interface != nullptr)
        {
            // Destroy cli server should done outside cli context
            // If cli server shutdown is called in this context, dead lock happend
            // This context waits cli server's exit, but cli server's exit waits all cli done.
            // So, we just send the signal to main() thread can waits all cli done.
            interface->TriggerTerminate();
        }
        eventId = EID(SUCCESS);
    }
    else
    {
        POS_REPORT_TRACE(EID(POS_TERMINATION_FAILED_ALREADY_IN_PROGRESS), "already in termination process");
        eventId = EID(POS_TERMINATION_FAILED_ALREADY_IN_PROGRESS);
    }
    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return status;
}

grpc::Status
CommandProcessor::ExecuteGetSystemPropertyCommand(const GetSystemPropertyRequest* request,
    GetSystemPropertyResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    qos_backend_policy backendPolicy = QosManagerSingleton::Instance()->GetBackendPolicy(BackendEvent_UserdataRebuild);
    string impact = _GetRebuildImpactString(backendPolicy.priorityImpact);
    reply->mutable_result()->mutable_data()->set_rebuild_policy(impact);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSetSystemPropertyCommand(const SetSystemPropertyRequest* request,
    SetSystemPropertyResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    int eventId = EID(SUCCESS);
    Status status = grpc::Status::OK;

    qos_backend_policy newBackendPolicy;

    std::string level = request->param().level();
    IODispatcherSubmission* ioDispatcherSubmission = IODispatcherSubmissionSingleton::Instance();
    if (level.compare("high") == 0)
    {
        newBackendPolicy.priorityImpact = PRIORITY_HIGH;
        ioDispatcherSubmission->SetRebuildImpact(RebuildImpact::High);
    }
    else if (level.compare("medium") == 0)
    {
        newBackendPolicy.priorityImpact = PRIORITY_MEDIUM;
        ioDispatcherSubmission->SetRebuildImpact(RebuildImpact::Medium);
    }
    else if (level.compare("low") == 0)
    {
        newBackendPolicy.priorityImpact = PRIORITY_LOW;
        ioDispatcherSubmission->SetRebuildImpact(RebuildImpact::Low);
    }
    else
    {
        eventId = EID(CLI_SET_SYSTEM_PROPERTY_LEVEL_NOT_SUPPORTED);
        POS_TRACE_WARN(EID(CLI_SET_SYSTEM_PROPERTY_LEVEL_NOT_SUPPORTED), "priority_level:{}", level);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return status;
    }

    newBackendPolicy.policyChange = true;
    int retVal = QosManagerSingleton::Instance()->UpdateBackendPolicy(BackendEvent_UserdataRebuild, newBackendPolicy);
    if (retVal != SUCCESS)
    {
        eventId = EID(CLI_SET_SYSTEM_PROPERTY_FAILURE);
        POS_TRACE_WARN(eventId, "update_backend_policy_return_value:{}", retVal);
        status = grpc::Status::OK;
    }

    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return status;
}
