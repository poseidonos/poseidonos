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
CommandProcessor::ExecuteSetLogLevelCommand(const SetLogLevelRequest* request, SetLogLevelResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string level = (request->param()).level();

    if (level == "")
    {
        int eventId = EID(CLI_SET_LOG_LEVEL_FAILURE_LEVEL_NOT_SPECIFIED);
        POS_TRACE_INFO(eventId, "");
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    int ret = logger()->SetLevel(level);

    POS_TRACE_INFO(ret, "");
    _SetEventStatus(ret, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSetLogPreferenceCommand(const SetLogPreferenceRequest* request, SetLogPreferenceResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string structuredLogging = (request->param()).structuredlogging();

    if (structuredLogging == "")
    {
        int eventId = EID(CLI_SET_LOG_PREFERENCE_FAILURE_STR_LOG_NOT_SPECIFIED);
        POS_TRACE_INFO(eventId, "");
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    bool _structuredLogging = (strcasecmp("true", structuredLogging.c_str()) == 0);
    int ret = logger()->SetStrLogging(_structuredLogging);
    logger()->ApplyPreference();

    POS_TRACE_INFO(ret, "");
    _SetEventStatus(ret, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteLoggerInfoCommand(const LoggerInfoRequest* request, LoggerInfoResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    pos_logger::Preferences pref = logger()->GetPreferenceInstance();

    reply->mutable_result()->mutable_data()->set_minorlogpath(pref.MinorLogFilePath());
    reply->mutable_result()->mutable_data()->set_majorlogpath(pref.MajorLogFilePath());
    reply->mutable_result()->mutable_data()->set_logfilesizeinmb(to_string(pref.LogFileSize()));
    reply->mutable_result()->mutable_data()->set_logfilerotationcount(pref.LogRotation());
    reply->mutable_result()->mutable_data()->set_minallowableloglevel(pref.LogLevel());
    reply->mutable_result()->mutable_data()->set_filterenabled(pref.IsFiltered());
    reply->mutable_result()->mutable_data()->set_filterincluded(pref.IncludeRule());
    reply->mutable_result()->mutable_data()->set_filterexcluded(pref.ExcludeRule());
    reply->mutable_result()->mutable_data()->set_structuredlogging(pref.IsStrLoggingEnabled());

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteGetLogLevelCommand(const GetLogLevelRequest* request, GetLogLevelResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    reply->mutable_result()->mutable_data()->set_level(logger()->GetLevel());

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteApplyLogFilterCommand(const ApplyLogFilterRequest* request, ApplyLogFilterResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    int ret = logger()->ApplyFilter();

    POS_TRACE_INFO(ret, "");
    _SetEventStatus(ret, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}
