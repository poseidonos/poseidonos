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
CommandProcessor::ExecuteAddSpareCommand(const AddSpareRequest* request, AddSpareResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    grpc_cli::AddSpareRequest_Param param = request->param();

    string arrayName = param.array();

    if (param.spare_size() == 0)
    {
        int eventId = EID(CLI_ADD_DEVICE_FAILURE_NO_DEVICE_SPECIFIED);
        POS_TRACE_WARN(eventId, "");
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    string devName = param.spare().at(0).devicename();
    IArrayMgmt* array = ArrayMgr();
    int ret = array->AddDevice(arrayName, devName);
    if (ret == EID(SUCCESS))
    {
        int eventId = EID(SUCCESS);
        POS_TRACE_INFO(eventId, "device_name:{}, array_name:{}", devName, arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}

grpc::Status
CommandProcessor::ExecuteRemoveSpareCommand(const RemoveSpareRequest* request, RemoveSpareResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string arrayName = (request->param()).array();
    string devName = (request->param()).spare().at(0).devicename();

    if (devName == "")
    {
        int eventId = EID(CLI_ADD_DEVICE_FAILURE_NO_DEVICE_SPECIFIED);
        POS_TRACE_WARN(eventId, "");
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IArrayMgmt* array = ArrayMgr();
    int ret = array->RemoveDevice(arrayName, devName);
    if (ret == 0)
    {
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}

grpc::Status
CommandProcessor::ExecuteReplaceArrayDeviceCommand(const ReplaceArrayDeviceRequest* request, ReplaceArrayDeviceResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string arrayName = (request->param()).array();
    string devName = (request->param()).device();

    if (devName == "")
    {
        int eventId = EID(CLI_ADD_DEVICE_FAILURE_NO_DEVICE_SPECIFIED);
        POS_TRACE_WARN(eventId, "");
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IArrayMgmt* array = ArrayMgr();
    int ret = array->ReplaceDevice(arrayName, devName);
    if (ret == 0)
    {
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}
