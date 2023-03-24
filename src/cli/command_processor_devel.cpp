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
CommandProcessor::ExecuteResetEventWrrCommand(const ResetEventWrrRequest* request,
    ResetEventWrrResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    for (int eventId = BackendEvent_Start; eventId != BackendEvent_Unknown; eventId++)
    {
        BackendEvent event = static_cast<BackendEvent>(eventId);
        QosManagerSingleton::Instance()->SetEventWeightWRR(event, RESET_EVENT_WRR_DEFAULT_WEIGHT);
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteResetMbrCommand(const ResetMbrRequest* request,
    ResetMbrResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    int result = ArrayManagerSingleton::Instance()->ResetPbr();

    if (result != 0)
    {
        _SetEventStatus(result, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteStopRebuildingCommand(const StopRebuildingRequest* request,
    StopRebuildingResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string arrayName = (request->param()).name();
    IArrayRebuilder* rebuilder = ArrayManagerSingleton::Instance()->arrayRebuilder;
    rebuilder->StopRebuild(arrayName, EID(REBUILD_STOPPED_USER_REQUEST));
    rebuilder->WaitRebuildDone(arrayName);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteUpdateEventWrrCommand(const UpdateEventWrrRequest* request,
    UpdateEventWrrResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    int eventId = EID(SUCCESS);
    string eventName = (request->param()).name();
    int weight = (request->param()).weight();

    if (eventName == "")
    {
        eventId = EID(CLI_UPDATE_EVENT_WRR_FAILURE_NO_EVENT_NAME);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status::OK;
    }

    if ((weight < 1) || (3 < weight))
    {
        eventId = EID(CLI_UPDATE_EVENT_WRR_FAILURE_WEIGHT_VALUE_RANGE_VIOLATION);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status::OK;
    }

    BackendEvent event = CommandProcessor::_GetEventId(eventName);
    if (event == pos::BackendEvent_Unknown)
    {
        eventId = EID(CLI_UPDATE_EVENT_WRR_FAILURE_UNKNOWN_EVENT);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return Status::OK;
    }

    QosManagerSingleton::Instance()->SetEventWeightWRR(event, weight);

    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteDumpMemorySnapshotCommand(const DumpMemorySnapshotRequest* request, DumpMemorySnapshotResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    grpc_cli::DumpMemorySnapshotRequest_Param param = request->param();
    std::string path = param.path();

    const std::string getPidCmd = "ps -a | awk \'$4==\"poseidonos\" {print $1}\'";
    std::string pid = _ExecuteLinuxCmd(getPidCmd);

    const std::string gcoreCmd = "gcore -o " + path + " " + pid;
    struct timeval begin, end;

    gettimeofday(&begin, 0);
    std::string result = _ExecuteLinuxCmd(gcoreCmd);
    gettimeofday(&end, 0);

    long seconds = end.tv_sec - begin.tv_sec;
    long microseconds = end.tv_usec - begin.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;

    POS_TRACE_INFO(EID(CLI_MEMORY_SNAPSHOT_DUMP_DONE),
        "pid:{}, command:{}, result:{}, elapsed_time:{}",
        pid, gcoreCmd, result, elapsed);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}
