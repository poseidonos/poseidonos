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
CommandProcessor::ExecuteStartTelemetryCommand(const StartTelemetryRequest* request,
    StartTelemetryResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    TelemetryClient* tc = TelemetryClientSingleton::Instance();
    bool result = tc->StartAllPublisher();
    if (!result)
    {
        _SetEventStatus(EID(TELEMETRY_START_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status(StatusCode::FAILED_PRECONDITION, "");
    }

    TelemetryConfig* config = TelemetryConfigSingleton::Instance();
    if (!config->GetClient().UpdateConfig(TelemetryConfigType::Client, "enabled", true, true))
    {
        _SetEventStatus(EID(TELEMETRY_START_FAILURE_CONFIG_ERROR), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status(StatusCode::FAILED_PRECONDITION, "");
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());

    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteStopTelemetryCommand(const StopTelemetryRequest* request,
    StopTelemetryResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    TelemetryClient* tc = TelemetryClientSingleton::Instance();
    bool result = tc->StopAllPublisher();
    if (!result)
    {
        _SetEventStatus(EID(TELEMETRY_STOP_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status(StatusCode::FAILED_PRECONDITION, "");
    }

    TelemetryConfig* config = TelemetryConfigSingleton::Instance();
    if (!config->GetClient().UpdateConfig(TelemetryConfigType::Client, "enabled", true, true))
    {
        _SetEventStatus(EID(TELEMETRY_STOP_FAILURE_CONFIG_ERROR), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status(StatusCode::FAILED_PRECONDITION, "");
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());

    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSetTelemetryPropertyCommand(const SetTelemetryPropertyRequest* request,
    SetTelemetryPropertyResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    TelemetryClient* tc = TelemetryClientSingleton::Instance();

    std::string publicationListPath = (request->param()).publicationlistpath();

    if (_IsValidFile(publicationListPath))
    {
        tc->LoadPublicationList(publicationListPath);
    }
    else
    {
        _SetEventStatus(EID(TELEMETRY_SET_PROPERTY_FAILURE_INVALID_FILE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteGetTelemetryPropertyCommand(const GetTelemetryPropertyRequest* request, GetTelemetryPropertyResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    TelemetryClient* tc = TelemetryClientSingleton::Instance();

    bool isRunning = tc->IsRunning();
    std::string publicationListPath = tc->GetPublicationList();

    reply->mutable_result()->mutable_data()->set_status(isRunning);
    reply->mutable_result()->mutable_data()->set_publicationlistpath(publicationListPath);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());

    return grpc::Status::OK;
}
