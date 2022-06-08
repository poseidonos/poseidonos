
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/cli/command_processor.h"

#include "src/array/array.h"
#include "src/array_mgmt/numa_awared_array_creation.h"
#include "src/array_mgmt/array_manager.h"
#include "src/mbr/mbr_info.h"
#include "src/sys_info/space_info.h"
#include "src/cli/request_handler.h"
#include "src/cli/cli_server.h"
#include "src/logger/logger.h"
#include "src/mbr/mbr_info.h"
#include "src/master_context/version_provider.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_manager.h"
#include "src/network/nvmf_target.h"
#include "src/qos/qos_manager.h"

CommandProcessor::CommandProcessor(void)
{
    google::protobuf::util::JsonPrintOptions options;
    options.always_print_primitive_fields = true;
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

    if (param.isspare() == false)
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
    if (ret == 0)
    {
        int eventId = EID(CLI_ADD_DEVICE_SUCCESS);
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
CommandProcessor::ExecuteAutocreateArrayCommand(const AutocreateArrayRequest* request, AutocreateArrayResponse* reply)
{
    vector<string> buffers;
    string arrayName = (request->param()).name();
    
    string dataFt = (request->param()).raidtype();
    if (dataFt == "")
    {
        dataFt = "RAID5";
    }

    string metaFt = "RAID10";
    if (dataFt == "RAID0" || dataFt == "NONE")
    {
        metaFt = dataFt;
    }

    for (const grpc_cli::DeviceNameList& buffer : (request->param()).buffer())
    {
        buffers.push_back(buffer.devicename());
    }

    int dataCnt = 0;
    dataCnt = (request->param()).numdata();
    
    int spareCnt = 0;
    spareCnt = (request->param()).numspare();

    NumaAwaredArrayCreation creationDelegate(buffers, dataCnt, spareCnt, DeviceManagerSingleton::Instance());
    NumaAwaredArrayCreationResult res = creationDelegate.GetResult();

    if (res.code != 0)
    {
        _SetEventStatus(res.code, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IArrayMgmt* array = ArrayMgr();
    // TODO(SRM): interactive cli to select from multiple-options.
    int ret = array->Create(arrayName, res.options.front().devs, metaFt, dataFt);

    if (0 != ret)
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        QosManagerSingleton::Instance()->UpdateArrayMap(arrayName);
        int event = EID(CLI_AUTOCREATE_ARRAY_SUCCESS);
        POS_TRACE_INFO(event, "");
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}

grpc::Status
CommandProcessor::ExecuteCreateArrayCommand(const CreateArrayRequest* request, CreateArrayResponse* reply)
{
    DeviceSet<string> nameSet;
    string arrayName = (request->param()).name();
    
    string dataFt = (request->param()).raidtype();
    if (dataFt == "")
    {
        dataFt = "RAID5";
    }

    string metaFt = "RAID10";
    if (dataFt == "RAID0" || dataFt == "NONE")
    {
        metaFt = dataFt;
    }

    for (const grpc_cli::DeviceNameList& buffer : (request->param()).buffer())
    {
        if (buffer.devicename() != "")
        {
            nameSet.nvm.push_back(buffer.devicename());
        }
    }

    for (const grpc_cli::DeviceNameList& data : (request->param()).data())
    {
        if (data.devicename() != "")
        {
            nameSet.data.push_back(data.devicename());
        }
    }
    
    for (const grpc_cli::DeviceNameList& spare : (request->param()).spare())
    {
        if (spare.devicename() != "")
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
CommandProcessor::ExecuteListArrayCommand(const ListArrayRequest* request, ListArrayResponse* reply)
{
    std::vector<ArrayBootRecord> abrList;
    int result = ArrayManagerSingleton::Instance()->GetAbrList(abrList);

    if (result != 0)
    {
        if (result == EID(MBR_DATA_NOT_FOUND))
        {
            result = EID(CLI_LIST_ARRAY_NO_ARRAY_EXISTS);
            _SetEventStatus(result, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        else
        {
            int result = EID(CLI_LIST_ARRAY_FAILURE);
            POS_TRACE_WARN(result, "");
            _SetEventStatus(result, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }

    if (abrList.empty())
    {
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        JsonArray jsonArrayList("arrayList");
        for (const auto& abr : abrList)
        {
            string arrayName(abr.arrayName);
            string createDatetime(abr.createDatetime);
            string updateDatetime(abr.updateDatetime);
            string arrayStatus(DEFAULT_ARRAY_STATUS);

            ComponentsInfo* CompInfo = ArrayMgr()->GetInfo(arrayName);
            if (CompInfo == nullptr)
            {
                POS_TRACE_ERROR(EID(ARRAY_MGR_DEBUG_MSG),
                    "Failed to list array"
                    " because of failing to get componentsInfo"
                    " with given array name. ArrayName: {}", arrayName);
                _SetEventStatus(EID(ARRAY_MGR_DEBUG_MSG), reply->mutable_result()->mutable_status());
                _SetPosInfo(reply->mutable_info());
                continue;
            }

            IArrayInfo* info = CompInfo->arrayInfo;
            grpc_cli::Array* array =
                reply->mutable_result()->mutable_data()->add_arraylist();
            if (info == nullptr)
            {
                arrayStatus = "Fault";
                array->set_index(ARRAY_ERROR_INDEX);
            }
            else
            {
                if (info->GetState() >= ArrayStateEnum::NORMAL)
                {
                    arrayStatus = "Mounted";
                }
                array->set_index(info->GetIndex());
                array->set_dataraid(info->GetDataRaidType());
                array->set_writethroughenabled(info->IsWriteThroughEnabled());   
            }

            array->set_name(arrayName);
            array->set_status(arrayStatus);
            array->set_createdatetime(createDatetime);
            array->set_updatedatetime(updateDatetime);
            array->set_capacity(SpaceInfo::SystemCapacity(arrayName));
            array->set_used(SpaceInfo::Used(arrayName));
        }
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteArrayInfoCommand(const ArrayInfoRequest* request, ArrayInfoResponse* reply)
{
    string arrayName = (request->param()).name();

    if (arrayName == "")
    {   
        int event = EID(CLI_ARRAY_INFO_NO_ARRAY_NAME);
        POS_TRACE_WARN(event, "");
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IArrayInfo* arrayInfo = info->arrayInfo;
    pos::IGCInfo* gc = info->gcInfo;

    string state = arrayInfo->GetStateCtx()->ToStateType().ToString();
    string situ = arrayInfo->GetStateCtx()->GetSituation().ToString();

    grpc_cli::Array* array = reply->mutable_result()->mutable_data();
    array->set_index(arrayInfo->GetIndex());
    array->set_unique_id(arrayInfo->GetUniqueId());
    array->set_name(arrayName);
    array->set_state(state);
    array->set_situation(situ);
    array->set_createdatetime(arrayInfo->GetCreateDatetime());
    array->set_updatedatetime(arrayInfo->GetUpdateDatetime());
    array->set_rebuildingprogress(to_string(arrayInfo->GetRebuildingProgress()));
    array->set_capacity(SpaceInfo::SystemCapacity(arrayName));
    array->set_used(SpaceInfo::Used(arrayName));
    array->set_metaraid(arrayInfo->GetMetaRaidType());
    array->set_dataraid(arrayInfo->GetDataRaidType());
    array->set_writethroughenabled(arrayInfo->IsWriteThroughEnabled());

    if (arrayInfo->GetState() >= ArrayStateEnum::NORMAL)
    {
        array->set_gcmode(_GetGCMode(gc, arrayName));
    }

    DeviceSet<string> nameSet = arrayInfo->GetDevNames();

    if (nameSet.nvm.size() == 0 && nameSet.data.size() == 0)
    {
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    for (string name : nameSet.nvm)
    {
        grpc_cli::Device* device = array->add_devicelist();
        device->set_type("BUFFER");
        device->set_name(name);
    }
    for (string name : nameSet.data)
    {
        grpc_cli::Device* device = array->add_devicelist();
        device->set_type("DATA");
        device->set_name(name);
    }
    for (string name : nameSet.spares)
    {
        grpc_cli::Device* device = array->add_devicelist();
        device->set_type("SPARE");
        device->set_name(name);
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

std::string
CommandProcessor::_GetGCMode(pos::IGCInfo* gc, std::string arrayName)
{
    if (arrayName == "")
    {
        return "N/A";
    }

    int isEnabled = gc->IsEnabled();
    if (0 != isEnabled)
    {
        return "N/A";
    }

    IContextManager* iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(arrayName);
    GcCtx* gcCtx = iContextManager->GetGcCtx();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        return "N/A";
    }

    GcMode gcMode = gcCtx->GetCurrentGcMode();

    std::string strGCMode;

    if (gcMode == GcMode::MODE_URGENT_GC)
    {
        strGCMode = "urgent";
    }
    else if (gcMode == GcMode::MODE_NORMAL_GC)
    {
        strGCMode = "normal";
    }
    else
    {
        strGCMode = "none";
    }

    return strGCMode;
}