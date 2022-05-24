
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/cli/command_processor.h"

#include "src/array_mgmt/array_manager.h"
#include "src/cli/request_handler.h"
#include "src/cli/cli_server.h"
#include "src/logger/logger.h"
#include "src/mbr/mbr_info.h"
#include "src/master_context/version_provider.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_manager.h"

CommandProcessor::CommandProcessor(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
CommandProcessor::~CommandProcessor(void)
{
}
// LCOV_EXCL_STOP

grpc::Status
CommandProcessor::ExecuteSystemInfoCommand(const SystemInfoRequest* request, SystemInfoResponse* reply)
{
    logger()->SetCommand(request->command());
    
    reply->set_command(request->command());
    reply->set_rid(request->rid());
    
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    reply->mutable_result()->mutable_data()->set_version(version);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    
    logger()->ResetCommand();

    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSystemStopCommand(const SystemStopRequest* request, SystemStopResponse* reply)
{
    logger()->SetCommand(request->command());

    reply->set_command(request->command());
    reply->set_rid(request->rid());
    
    int ret = 0;
    std::vector<ArrayBootRecord> abrList;
    ret = ArrayManagerSingleton::Instance()->GetAbrList(abrList);

    if (ret == 0)
    {
        if (!abrList.empty())
        {
            int eventId = EID(MBR_ABR_LIST_SUCCESS);
            POS_TRACE_DEBUG(eventId, "Found {} arrays from abr list", abrList.size());
            for (const auto& abr : abrList)
            {
                ComponentsInfo* CompInfo = ArrayMgr()->GetInfo(abr.arrayName);

                if (CompInfo == nullptr || CompInfo->arrayInfo == nullptr)
                {
                    continue;
                }
                IArrayInfo* arrayInfo = CompInfo->arrayInfo;

                if (arrayInfo->GetState() >= ArrayStateEnum::TRY_MOUNT)
                {
                    eventId = EID(STOP_POS_REJECTED_DUE_TO_MOUNTED_ARRAY_EXISTS);
                    POS_TRACE_ERROR(eventId,
                        "array:{}, state:{}",
                        abr.arrayName, arrayInfo->GetState().ToString());

                    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
                    _SetPosInfo(reply->mutable_info());
                    
                    return grpc::Status::OK;
                }
            }
        }
    }

    if (!_IsPosTerminating())
    {
        _SetPosTerminating(true);
        pos_cli::Exit(); // ToDo (mj): gRPC CLI server temporarily uses pos_cli::Exit()
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
    }
    else
    {
        _SetEventStatus(EID(POS_STOP_REJECTED_BEING_TERMINATED), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
    }

    logger()->ResetCommand();

    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteGetSystemPropertyCommand(const GetSystemPropertyRequest* request,
    GetSystemPropertyResponse* reply)
{
    logger()->SetCommand(request->command());

    reply->set_command(request->command());
    reply->set_rid(request->rid());
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    reply->mutable_info()->set_version(version);

    qos_backend_policy backendPolicy = QosManagerSingleton::Instance()->GetBackendPolicy(BackendEvent_UserdataRebuild);
    string impact = _GetRebuildImpactString(backendPolicy.priorityImpact);

    reply->mutable_result()->mutable_data()->set_rebuild_policy(impact);
    reply->mutable_result()->mutable_status()->set_code(EID(SUCCESS));
    reply->mutable_result()->mutable_status()->set_event_name("SUCCESS");

    logger()->ResetCommand();
    
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSetSystemPropertyCommand(const SetSystemPropertyRequest* request,
    SetSystemPropertyResponse* reply)
{
    logger()->SetCommand(request->command());

    reply->set_command(request->command());
    reply->set_rid(request->rid());
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    reply->mutable_info()->set_version(version);

    qos_backend_policy newBackendPolicy;

    if (request->param().level().compare("highest") == 0)
        {
            newBackendPolicy.priorityImpact = PRIORITY_HIGHEST;
        }
        else if (request->param().level().compare("medium") == 0)
        {
            newBackendPolicy.priorityImpact = PRIORITY_MEDIUM;
        }
        else if (request->param().level().compare("lowest") == 0)
        {
            newBackendPolicy.priorityImpact = PRIORITY_LOWEST;
        }
        else
        {
            _SetEventStatus(EID(CLI_SET_SYSTEM_PROPERTY_LEVEL_NOT_SUPPORTED), reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }

        newBackendPolicy.policyChange = true;
        int retVal = QosManagerSingleton::Instance()->UpdateBackendPolicy(BackendEvent_UserdataRebuild, newBackendPolicy);
        if (retVal != SUCCESS)
        {
            _SetEventStatus(EID(CLI_SET_SYSTEM_PROPERTY_LEVEL_NOT_SUPPORTED), reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }

        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());

        logger()->ResetCommand();
        
        return grpc::Status::OK;
}

std::string
CommandProcessor::_GetRebuildImpactString(uint8_t impact)
{
    switch (impact)
    {
        case PRIORITY_HIGHEST:
            return "highest";

        case PRIORITY_MEDIUM:
            return "medium";

        case PRIORITY_LOWEST:
            return "lowest";

        default:
            return "unknown";
    }
}

void
CommandProcessor::_SetEventStatus(int eventId, grpc_cli::Status *status)
{
    std::unordered_map<int, PosEventInfoEntry*>::const_iterator it =
        PosEventInfo.find(eventId);
        
    if (it == PosEventInfo.end())
    {
        status->set_code(eventId);
        status->set_event_name("");
        status->set_description("");
        status->set_cause("");
        status->set_solution("");
    }
    else
    {
        PosEventInfoEntry* entry = it->second;
        status->set_code(eventId);
        status->set_event_name(entry->GetEventName());
        status->set_description(entry->GetMessage());
        status->set_cause(entry->GetCause());
        status->set_solution(entry->GetSolution());
    }
}

void
CommandProcessor::_SetPosInfo(grpc_cli::PosInfo *posInfo)
{
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    posInfo->set_version(version);
}
