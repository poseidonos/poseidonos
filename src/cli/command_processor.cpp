
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/cli/command_processor.h"

#include "src/array/array.h"
#include "src/array_mgmt/array_manager.h"
#include "src/cli/request_handler.h"
#include "src/cli/cli_server.h"
#include "src/logger/logger.h"
#include "src/mbr/mbr_info.h"
#include "src/master_context/version_provider.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_manager.h"
#include "src/qos/qos_manager.h"

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
    reply->set_command(request->command());
    reply->set_rid(request->rid());
    
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    reply->mutable_result()->mutable_data()->set_version(version);

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSystemStopCommand(const SystemStopRequest* request, SystemStopResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    int eventId = EID(SUCCESS);
    Status status = grpc::Status::OK;
    
    int ret = 0;
    std::vector<ArrayBootRecord> abrList;
    ret = ArrayManagerSingleton::Instance()->GetAbrList(abrList);

    if (ret == 0)
    {
        if (!abrList.empty())
        {
            eventId = EID(MBR_ABR_LIST_SUCCESS);
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

                    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
                    _SetPosInfo(reply->mutable_info());
                    
                    return status;
                }
            }
        }
    }

    if (!_IsPosTerminating())
    {
        _SetPosTerminating(true);
        pos_cli::Exit(); // ToDo (mj): gRPC CLI server temporarily uses pos_cli::Exit()
        eventId = EID(SUCCESS);
    }
    else
    {
        eventId = EID(POS_STOP_REJECTED_BEING_TERMINATED);
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
            eventId = EID(CLI_SET_SYSTEM_PROPERTY_LEVEL_NOT_SUPPORTED);
            status = grpc::Status::OK;
        }

        newBackendPolicy.policyChange = true;
        int retVal = QosManagerSingleton::Instance()->UpdateBackendPolicy(BackendEvent_UserdataRebuild, newBackendPolicy);
        if (retVal != SUCCESS)
        {
            eventId = EID(CLI_SET_SYSTEM_PROPERTY_LEVEL_NOT_SUPPORTED);
            status = grpc::Status::OK;
        }

        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return status;
}

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
CommandProcessor::ExecuteResetEventWrrCommand(const ResetEventWrrRequest* request,
    ResetEventWrrResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    for ( int eventId = BackendEvent_Start; eventId != BackendEvent_Unknown; eventId++ )
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

    int result = ArrayManagerSingleton::Instance()->ResetMbr();

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
    rebuilder->StopRebuild(arrayName);
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
CommandProcessor::ExecuteCreateArrayCommand(const CreateArrayRequest* request, CreateArrayResponse* reply)
{
    grpc_cli::CreateArrayRequest_Param param = request->param();
    
    DeviceSet<string> nameSet;
    string arrayName = param.name();
    
    string dataFt = "RAID5";
    if (param.raidtype().empty() == false)
    {
        dataFt = param.raidtype();
    }
    
    string metaFt = "RAID10";
    if (dataFt == "RAID0" || dataFt == "NONE")
    {
        metaFt = dataFt;
    }

    if (param.buffer_size() != 0)
    {
        for (const grpc_cli::DeviceNameList& buffer : (request->param()).buffer())
        {
            nameSet.nvm.push_back(buffer.devicename());
        }
    }

    if (param.data_size() != 0)
    {
        for (const grpc_cli::DeviceNameList& data : (request->param()).data())
        {
            nameSet.data.push_back(data.devicename());
        }
    }
    
    if (param.spare_size() != 0)
    {
        for (const grpc_cli::DeviceNameList& spare : (request->param()).spare())
        {
            nameSet.spares.push_back(spare.devicename());
        }
    }

    IArrayMgmt* array = ArrayMgr();
    int ret = array->Create(arrayName, nameSet, metaFt, dataFt);
    if (0 != ret)
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}

grpc::Status
CommandProcessor::ExecuteDeleteArrayCommand(const DeleteArrayRequest* request, DeleteArrayResponse* reply)
{
    NvmfTarget* nvmfTarget = NvmfTargetSingleton::Instance();
    string arrayName = (request->param()).name();

    IArrayMgmt* array = ArrayMgr();
    int ret = array->Delete(arrayName);

    if (0 != ret)
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    bool deleteDone = nvmfTarget->DeletePosBdevAll(arrayName);

    if (false == deleteDone)
    {
        _SetEventStatus(EID(IONVMF_VOL_DELETE_TIMEOUT),
            reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteMountArrayCommand(const MountArrayRequest* request, MountArrayResponse* reply)
{
    string arrayName = (request->param()).name();
    bool isWTenabled = (request->param()).enablewritethrough();
    
    if (isWTenabled == false)
    {
        bool isWTenabledAtConfig = false;
        int ret = ConfigManagerSingleton::Instance()->GetValue("write_through", "enable",
            &isWTenabledAtConfig, ConfigType::CONFIG_TYPE_BOOL);
        if (ret == SUCCESS)
        {
            if (isWTenabledAtConfig == true)
            {
                isWTenabled = true;
                POS_TRACE_WARN(EID(MOUNT_ARRAY_DEBUG_MSG), "Write through mode is forcibly activated by config");
            }
        }
    }

    IArrayMgmt* array = ArrayMgr();
    int ret = array->Mount(arrayName, isWTenabled);
    if (0 != ret)
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    QosManagerSingleton::Instance()->UpdateArrayMap(arrayName);
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteUnmountArrayCommand(const UnmountArrayRequest* request, UnmountArrayResponse* reply)
{
    string arrayName = (request->param()).name();

    IArrayMgmt* array =  ArrayMgr();
    int ret = array->Unmount(arrayName);
    if (ret != 0)
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    QosManagerSingleton::Instance()->DeleteEntryArrayMap(arrayName);
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

pos::BackendEvent
CommandProcessor::_GetEventId(std::string eventName)
{
    map<string, pos::BackendEvent> eventDict = {
        {"flush", pos::BackendEvent_Flush},
        {"gc", pos::BackendEvent_GC},
        {"rebuild", pos::BackendEvent_UserdataRebuild},
        {"meta_rebuild", pos::BackendEvent_MetadataRebuild},
        {"metaio", pos::BackendEvent_MetaIO},
        {"fe_rebuild", pos::BackendEvent_FrontendIO}
    };
    
    auto search = eventDict.find(eventName);
    if (search != eventDict.end())
    {
        return (search->second);
    }
    return (pos::BackendEvent_Unknown);
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
    std::string eventName = "";
    std::string message = "";
    std::string cause = "";
    std::string solution = "";

    std::unordered_map<int, PosEventInfoEntry*>::const_iterator it =
        PosEventInfo.find(eventId);
       
    if (it != PosEventInfo.end())
    {
        PosEventInfoEntry* entry = it->second;
        eventName = entry->GetEventName();
        message = entry->GetMessage();
        cause = entry->GetCause();
        solution = entry->GetSolution();
    }
    
    status->set_code(eventId);
    status->set_event_name(eventName);
    status->set_description(message);
    status->set_cause(cause);
    status->set_solution(solution);
}

void
CommandProcessor::_SetPosInfo(grpc_cli::PosInfo *posInfo)
{
    std::string version = pos::VersionProviderSingleton::Instance()->GetVersion();
    posInfo->set_version(version);
}
