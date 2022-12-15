#include "src/cli/command_processor.h"

#include <spdk/nvme_spec.h>
#include <sys/time.h>

#include <string>
#include <vector>

#include "src/array/array.h"
#include "src/array_mgmt/array_manager.h"
#include "src/array_mgmt/numa_awared_array_creation.h"
#include "src/cli/cli_event_code.h"
#include "src/cli/cli_server.h"
#include "src/cli/request_handler.h"
#include "src/device/device_manager.h"
#include "src/event/event_manager.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/include/array_config.h"
#include "src/include/nvmf_const.h"
#include "src/io_scheduler/io_dispatcher_submission.h"
#include "src/logger/logger.h"
#include "src/logger/preferences.h"
#include "src/master_context/version_provider.h"
#include "src/mbr/mbr_info.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/resource_checker/smart_collector.h"
#include "src/sys_info/space_info.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_manager.h"
#include "src/volume/volume_status_property.h"

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
            eventId = EID(MBR_ABR_GET_LIST_SUCCESS);
            POS_TRACE_DEBUG(eventId, "abr_list_size:{}", abrList.size());
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
                    eventId = EID(POS_STOP_FAIULRE_MOUNTED_ARRAY_EXISTS);
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
        eventId = EID(POS_STOP_FAILURE_BEING_TERMINATED);
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
        POS_TRACE_INFO(EID(CLI_AUTOCREATE_ARRAY_SUCCESS), "");
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
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
    string targetAddress = (request->param()).targetaddress();

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

    if (targetAddress != "")
    {
        if (_IsValidIpAddress(targetAddress))
        {
            int eventId = EID(MOUNT_ARRAY_FAILURE_TARGET_ADDRESS_INVALID);
            POS_TRACE_WARN(eventId, "targetAddress:{}", targetAddress);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }

        array->SetTargetAddress(arrayName,targetAddress);
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

    IArrayMgmt* array = ArrayMgr();
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

grpc::Status
CommandProcessor::ExecuteListArrayCommand(const ListArrayRequest* request, ListArrayResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

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
                    " with given array name. ArrayName: {}",
                    arrayName);
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
                array->set_name(arrayName);
                array->set_status(arrayStatus);
                array->set_createdatetime(createDatetime);
                array->set_updatedatetime(updateDatetime);
                array->set_capacity(SpaceInfo::TotalCapacity(info->GetIndex()));
                array->set_used(SpaceInfo::Used(info->GetIndex()));
            }
        }
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteArrayInfoCommand(const ArrayInfoRequest* request, ArrayInfoResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

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
    array->set_uniqueid(arrayInfo->GetUniqueId());
    array->set_name(arrayName);
    array->set_state(state);
    array->set_situation(situ);
    array->set_createdatetime(arrayInfo->GetCreateDatetime());
    array->set_updatedatetime(arrayInfo->GetUpdateDatetime());
    array->set_rebuildingprogress(to_string(arrayInfo->GetRebuildingProgress()));
    array->set_capacity(SpaceInfo::TotalCapacity(arrayInfo->GetIndex()));
    array->set_used(SpaceInfo::Used(arrayInfo->GetIndex()));
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
CommandProcessor::ExecuteRebuildArrayCommand(const RebuildArrayRequest* request, RebuildArrayResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string arrayName = (request->param()).name();
    IArrayMgmt* array =  ArrayMgr();
    int ret = array->Rebuild(arrayName);
    _SetEventStatus(ret, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

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

grpc::Status
CommandProcessor::ExecuteCreateDeviceCommand(const CreateDeviceRequest* request, CreateDeviceResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    AffinityManager* affinityManager = AffinityManagerSingleton::Instance();
    SpdkRpcClient* spdkRpcClient = new SpdkRpcClient();

    string name, devType;
    uint32_t numBlocks;
    uint32_t blockSize;
    uint32_t numa;

    name = (request->param()).name();
    devType = (request->param()).devtype();
    numBlocks = (request->param()).numblocks();
    blockSize = (request->param()).blocksize();
    numa = (request->param()).numa();

    uint32_t numaCount = affinityManager->GetNumaCount();
    if (numa >= numaCount)
    {
        if (spdkRpcClient != nullptr)
        {
            delete spdkRpcClient;
        }
        
        int eventId = EID(CLI_CREATE_DEVICE_FAILURE_NUMA_COUNT_EQGT_TOTAL);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    // ToDo(mj): now we support to create bdev only.
    // When more device creations are supported,
    // a case switch may be needed.
    if (spdkRpcClient != nullptr)
    {
        auto ret = spdkRpcClient->BdevMallocCreate(
            name,
            numBlocks,
            blockSize,
            numa);

        delete spdkRpcClient;
        
        if (ret.first != 0)
        {
            int eventId = EID(CLI_CREATE_DEVICE_FAILURE);
            POS_TRACE_INFO(eventId, "error:{}", ret.second);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteScanDeviceCommand(const ScanDeviceRequest* request, ScanDeviceResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    list<string> failedArrayList;
    DeviceManagerSingleton::Instance()->ScanDevs();
    int result = ArrayManagerSingleton::Instance()->Load(failedArrayList);

    if (result != 0)
    {
        string failedArrayString = "";
        if (failedArrayList.empty() == false)
        {
            for (auto arrayName : failedArrayList)
            {
                failedArrayString += arrayName;
                failedArrayString += " ";
            }
        }
        else
        {
            failedArrayString += "no array found";
        }

        POS_TRACE_WARN(result, "failedArrays: " + failedArrayString);
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteListDeviceCommand(const ListDeviceRequest* request, ListDeviceResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    vector<DeviceProperty> list =
        DeviceManagerSingleton::Instance()->ListDevs();

    if (list.size() == 0)
    {
        POS_TRACE_INFO(EID(CLI_LIST_DEVICE_NO_DEVICE_FOUND), "");
        _SetEventStatus(EID(CLI_LIST_DEVICE_NO_DEVICE_FOUND), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    for (size_t i = 0; i < list.size(); i++)
    {
        grpc_cli::Device* device =
            reply->mutable_result()->mutable_data()->add_devicelist();
        device->set_name(list[i].name);
        device->set_size(list[i].size);
        device->set_modelnumber(list[i].mn);
        device->set_serialnumber(list[i].sn);
        device->set_type(list[i].GetType());
        device->set_address(list[i].bdf);
        device->set_class_(list[i].GetClass());

        string numa = ((list[i].numa == UNKNOWN_NUMA_NODE) ? "UNKNOWN" : to_string(list[i].numa));
        device->set_numa(numa);
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteGetSmartLogCommand(const GetSmartLogRequest* request, GetSmartLogResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    string deviceName = (request->param()).name();
    if (deviceName == "")
    {
        int eventId = EID(CLI_GET_SMART_LOG_NO_DEVICE_NAME);
        POS_TRACE_WARN(eventId, "");
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    struct spdk_nvme_ctrlr* ctrlr;
    ctrlr = pos::DeviceManagerSingleton::Instance()->GetNvmeCtrlr(deviceName);
    if (ctrlr == nullptr)
    {
        int eventId = EID(CLI_GET_SMART_LOG_DEVICE_NOT_FOUND);
        POS_TRACE_WARN(eventId, "deviceName:{}", deviceName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    struct spdk_nvme_health_information_page payload = {};
    SmartCollector* smartCollector = SmartCollectorSingleton::Instance();
    SmartReturnType ret = smartCollector->CollectPerCtrl(&payload, ctrlr, SmartReqId::NVME_HEALTH_INFO);

    if (ret != SmartReturnType::SUCCESS)
    {
        int eventId = EID(CLI_GET_SMART_LOG_FAILURE);
        POS_TRACE_WARN(eventId, "deviceName:{}, error:{}",
            deviceName, (int)ret);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _FillSmartData(&payload, reply->mutable_result()->mutable_data());

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
        {"journalio", pos::BackendEvent_JournalIO},
        {"metaio", pos::BackendEvent_MetaIO},
        {"flushmap", pos::BackendEvent_FlushMap},
        {"fe_rebuild", pos::BackendEvent_FrontendIO}};

    auto search = eventDict.find(eventName);
    if (search != eventDict.end())
    {
        return (search->second);
    }
    return (pos::BackendEvent_Unknown);
}

grpc::Status
CommandProcessor::ExecuteCreateSubsystemCommand(const CreateSubsystemRequest* request, CreateSubsystemResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    const char* DEFAULT_SERIAL_NUMBER = "POS0000000000000";
    const char* DEFAULT_MODEL_NUMBER = "POS_VOLUME_EXTENTION";
    uint32_t DEFAULT_MAX_NAMESPACES = 256;

    string subnqn = "";
    std::string serialNumber = DEFAULT_SERIAL_NUMBER;
    std::string modelNumber = DEFAULT_MODEL_NUMBER;
    uint32_t maxNamespaces = DEFAULT_MAX_NAMESPACES;
    bool allowAnyHost = false;
    bool anaReporting = false;

    SpdkRpcClient rpcClient;
    NvmfTarget target;

    subnqn = (request->param()).nqn();
    
    if (command == "CREATESUBSYSTEMAUTO")
    {
        if (nullptr != target.FindSubsystem(subnqn))
        {
            POS_TRACE_INFO(EID(SUCCESS), "subnqn:{}", subnqn);
            _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }

        string key("subsystem");
        string number;
        serialNumber = DEFAULT_SERIAL_NUMBER;
        size_t found = subnqn.rfind(key);

        if (found != string::npos)
        {
            size_t index = found + key.length();
            number = subnqn.substr(index);
            serialNumber += number;
        }
        allowAnyHost = true;
    }
    else if (command == "CREATESUBSYSTEM")
    {
        if (nullptr != target.FindSubsystem(subnqn))
        {
            POS_TRACE_INFO(EID(CREATE_SUBSYSTEM_SUBNQN_ALREADY_EXIST), "subnqn:{}", subnqn);
            _SetEventStatus(EID(CREATE_SUBSYSTEM_SUBNQN_ALREADY_EXIST), reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }

        serialNumber = (request->param()).serialnumber();
        modelNumber = (request->param()).modelnumber();
        maxNamespaces = (request->param()).maxnamespaces();
        allowAnyHost = (request->param()).allowanyhost();
        anaReporting = (request->param()).anareporting();
    }

    auto ret = rpcClient.SubsystemCreate(
        subnqn,
        serialNumber,
        modelNumber,
        maxNamespaces,
        allowAnyHost,
        anaReporting);

    if (ret.first != SUCCESS)
    {
        POS_TRACE_INFO(EID(CREATE_SUBSYSTEM_FAILURE), "subnqn:{}, serialNumber:{}, modelNumber:{}, maxNamespaces:{}, allowAnyHost:{}, anaReporting:{}, spdkRpcMsg:{}",
            subnqn, serialNumber, modelNumber, maxNamespaces,allowAnyHost, anaReporting, ret.second);
        _SetEventStatus(EID(CREATE_SUBSYSTEM_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    POS_TRACE_INFO(EID(SUCCESS), "subnqn:{}", subnqn);
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteDeleteSubsystemCommand(const DeleteSubsystemRequest* request, DeleteSubsystemResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string subnqn = (request->param()).subnqn();
    if (subnqn == "")
    {
        POS_TRACE_INFO(EID(DELETE_SUBSYSTEM_FAILURE_SUBNQN_NAME_NOT_SPECIFIED), "input_subnqn:{}", subnqn);
        _SetEventStatus(EID(DELETE_SUBSYSTEM_FAILURE_SUBNQN_NAME_NOT_SPECIFIED), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    int ret = 0;
    SpdkRpcClient rpcClient;
    NvmfTarget* nvmfTarget = NvmfTargetSingleton::Instance();

    if (nullptr == nvmfTarget->FindSubsystem(subnqn))
    {
        POS_TRACE_INFO(EID(DELETE_SUBSYSTEM_FAILURE_NO_SUBNQN), "subnqn:{}", subnqn);
        _SetEventStatus(EID(DELETE_SUBSYSTEM_FAILURE_NO_SUBNQN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    vector<pair<int, string>> attachedVolList = nvmfTarget->GetAttachedVolumeList(subnqn);
    map<string, vector<int>> volListPerArray;
    for (auto& volInfo : attachedVolList)
    {
        volListPerArray[volInfo.second].push_back(volInfo.first);
    }
    for (auto& volList : volListPerArray)
    {
        string arrayName = volList.first;
        IVolumeEventManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

        for (auto& volId : volList.second)
        {
            if (volMgr != nullptr)
            {
                string volName;
                ret = volMgr->Unmount(volId);
                if (ret == EID(DELETE_SUBSYSTEM_FAILURE_VOLUME_NOT_FOUND))
                {
                    POS_TRACE_INFO(EID(DELETE_SUBSYSTEM_FAILURE_VOLUME_NOT_FOUND),
                        "subnqn:{}, volume_id:{}, return_code:{}", subnqn, volId, ret);
                    _SetEventStatus(EID(DELETE_SUBSYSTEM_FAILURE_VOLUME_NOT_FOUND), reply->mutable_result()->mutable_status());
                    _SetPosInfo(reply->mutable_info());
                    return grpc::Status::OK;
                }
                else if (ret != SUCCESS)
                {
                    POS_TRACE_INFO(EID(DELETE_SUBSYSTEM_FAILURE_VOLUME_UNMOUNT_FAILURE),
                        "subnqn:{}, volume_id:{}, return_code:{}", subnqn, volId, ret);
                    _SetEventStatus(EID(DELETE_SUBSYSTEM_FAILURE_VOLUME_UNMOUNT_FAILURE), reply->mutable_result()->mutable_status());
                    _SetPosInfo(reply->mutable_info());
                    return grpc::Status::OK;
                }
            }
        }
    }

    auto result = rpcClient.SubsystemDelete(subnqn);
    if (result.first != SUCCESS)
    {
        POS_TRACE_INFO(EID(DELETE_SUBSYSTEM_FAILURE_SPDK_FAILURE), "subnqn:{}, rpc_return:{}", subnqn, result.second);
        _SetEventStatus(EID(DELETE_SUBSYSTEM_FAILURE_SPDK_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    POS_TRACE_INFO(EID(SUCCESS), "subnqn:{}", subnqn);
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteAddListenerCommand(const AddListenerRequest* request, AddListenerResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    SpdkRpcClient rpcClient;
    NvmfTarget target;

    const char* DEFAULT_ADRFAM = "IPv4";
    string subnqn = (request->param()).subnqn();
    string transportType = (request->param()).transporttype();
    string targetAddress = (request->param()).targetaddress();
    string transportServiceId = (request->param()).transportserviceid();

    if (nullptr == target.FindSubsystem(subnqn))
    {
        POS_TRACE_INFO(EID(ADD_LISTENER_FAILURE_NO_SUBNQN), "subnqn:{}", subnqn);
        _SetEventStatus(EID(ADD_LISTENER_FAILURE_NO_SUBNQN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    auto ret = rpcClient.SubsystemAddListener(
        subnqn,
        transportType,
        DEFAULT_ADRFAM,
        targetAddress,
        transportServiceId);

    if (ret.first != SUCCESS)
    {
        POS_TRACE_INFO(EID(ADD_LISTENER_FAILURE_SPDK_FAILURE), "subnqn:{}, rpc_return:{}", subnqn, ret.second);
        _SetEventStatus(EID(ADD_LISTENER_FAILURE_SPDK_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteListSubsystemCommand(const ListSubsystemRequest* request, ListSubsystemResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    SpdkRpcClient rpcClient;
    NvmfTarget target;

    auto list = rpcClient.SubsystemList();
    for (const auto& subsystem : list)
    {
        grpc_cli::Subsystem* subsystemListItem =
            reply->mutable_result()->mutable_data()->add_subsystemlist();
        subsystemListItem->set_nqn(subsystem["nqn"].asString());
        subsystemListItem->set_subtype(subsystem["subtype"].asString());
        subsystemListItem->set_allowanyhost(subsystem["allow_any_host"].asInt());

        for (const auto& address : subsystem["listen_addresses"])
        {
            grpc_cli::Subsystem_AddressInfo* addressInfoListItem =
                subsystemListItem->add_listenaddresses();
            addressInfoListItem->set_transporttype(address["trtype"].asString());
            addressInfoListItem->set_addressfamily(address["adrfam"].asString());
            addressInfoListItem->set_targetaddress(address["traddr"].asString());
            addressInfoListItem->set_transportserviceid(address["trsvcid"].asString());
        }

        for (const auto& host : subsystem["hosts"])
        {
            grpc_cli::Subsystem_Host* hostListItem =
                subsystemListItem->add_hosts();
            hostListItem->set_nqn(host["nqn"].asString());
        }

        if ("NVMe" == subsystem["subtype"].asString())
        {
            subsystemListItem->set_serialnumber(subsystem["serial_number"].asString());
            subsystemListItem->set_modelnumber(subsystem["model_number"].asString());
            subsystemListItem->set_maxnamespaces(subsystem["max_namespaces"].asInt());
        }

        for (const auto& ns : subsystem["namespaces"])
        {
            grpc_cli::Subsystem_Namespace* namespaceListItem =
                subsystemListItem->add_namespaces();
            namespaceListItem->set_nsid(ns["nsid"].asInt());
            namespaceListItem->set_bdevname(ns["bdev_name"].asString());
            namespaceListItem->set_uuid(ns["uuid"].asString());
        }
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSubsystemInfoCommand(const SubsystemInfoRequest* request, SubsystemInfoResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string subnqn = (request->param()).subnqn();

    SpdkRpcClient rpcClient;
    NvmfTarget target;

    auto list = rpcClient.SubsystemList();
    for (const auto& subsystem : list)
    {
        if (subnqn == subsystem["nqn"].asString())
        {
            grpc_cli::Subsystem* subsystemListItem =
                reply->mutable_result()->mutable_data()->add_subsystemlist();
            subsystemListItem->set_nqn(subsystem["nqn"].asString());
            subsystemListItem->set_subtype(subsystem["subtype"].asString());
            subsystemListItem->set_allowanyhost(subsystem["allow_any_host"].asInt());

            for (const auto& address : subsystem["listen_addresses"])
            {
                grpc_cli::Subsystem_AddressInfo* addressInfoListItem =
                    subsystemListItem->add_listenaddresses();
                addressInfoListItem->set_transporttype(address["trtype"].asString());
                addressInfoListItem->set_addressfamily(address["adrfam"].asString());
                addressInfoListItem->set_targetaddress(address["traddr"].asString());
                addressInfoListItem->set_transportserviceid(address["trsvcid"].asString());
            }

            for (const auto& host : subsystem["hosts"])
            {
                grpc_cli::Subsystem_Host* hostListItem =
                    subsystemListItem->add_hosts();
                hostListItem->set_nqn(host["nqn"].asString());
            }

            if ("NVMe" == subsystem["subtype"].asString())
            {
                subsystemListItem->set_serialnumber(subsystem["serial_number"].asString());
                subsystemListItem->set_modelnumber(subsystem["model_number"].asString());
                subsystemListItem->set_maxnamespaces(subsystem["max_namespaces"].asInt());
            }

            for (const auto& ns : subsystem["namespaces"])
            {
                grpc_cli::Subsystem_Namespace* namespaceListItem =
                    subsystemListItem->add_namespaces();
                namespaceListItem->set_nsid(ns["nsid"].asInt());
                namespaceListItem->set_bdevname(ns["bdev_name"].asString());
                namespaceListItem->set_uuid(ns["uuid"].asString());
            }
        }
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteCreateTransportCommand(const CreateTransportRequest* request, CreateTransportResponse* reply)
{
    string DEFAULT_TRTYPE = "tcp";

    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string trType = DEFAULT_TRTYPE;
    uint32_t bufCacheSize = DEFAULT_BUF_CACHE_SIZE;
    uint32_t numSharedBuf = DEFAULT_NUM_SHARED_BUF;
    uint32_t ioUnitSize = DEFAULT_IO_UNIT_SIZE;

    trType = (request->param()).transporttype();
    bufCacheSize = (request->param()).bufcachesize();
    numSharedBuf = (request->param()).numsharedbuf();

    SpdkRpcClient rpcClient;

    auto ret = rpcClient.TransportCreate(
        trType,
        bufCacheSize,
        numSharedBuf,
        ioUnitSize);

    if (ret.first != SUCCESS)
    {
        POS_TRACE_INFO(EID(ADD_TRANSPORT_FAILURE_SPDK_FAILURE),
            "trType:{}, bufCacheSize:{}, numSharedBuf:{}, rpc_return:{}", trType, bufCacheSize, numSharedBuf, ret.second);
        _SetEventStatus(EID(ADD_TRANSPORT_FAILURE_SPDK_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteCreateVolumeCommand(const CreateVolumeRequest* request, CreateVolumeResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string volumeName = "";
    string arrayName = "";
    uint64_t size = 0;
    uint64_t maxIops = 0;
    uint64_t maxBw = 0;
    bool isWalVol = false;
    string uuid = "";
    int32_t nsid = -1;
    bool isPrimary = true;
    bool isAnaNonoptimized = false;

    volumeName = (request->param()).name();
    arrayName = (request->param()).array();
    size = (request->param()).size();
    maxIops = (request->param()).maxiops();
    maxBw = (request->param()).maxbw();
    isWalVol = (request->param()).iswalvol();
    uuid = (request->param()).uuid();
    nsid = (request->param()).nsid();
    isPrimary = (request->param()).isprimary();
    isAnaNonoptimized = (request->param()).isananonoptimized();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int eventId = EID(CREATE_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (info->arrayInfo->GetState() < ArrayStateEnum::NORMAL)
    {
        int eventId = EID(CREATE_VOL_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "array_name:{}, array_state:{}", arrayName, info->arrayInfo->GetState().ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        maxIops = 0;
        maxBw = 0;
    }

    IVolumeEventManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (volMgr != nullptr)
    {
        int ret = volMgr->Create(volumeName, size, maxIops, maxBw, isWalVol, nsid, isPrimary, isAnaNonoptimized, uuid);
        if (ret == SUCCESS)
        {
            string targetAddress = ArrayMgr()->GetTargetAddress(arrayName);
            reply->mutable_result()->mutable_data()->set_targetaddress(targetAddress);
            
            int eventId = EID(SUCCESS);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        else
        {
            int eventId = ret;
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }

    int eventId = EID(CREATE_VOL_INTERNAL_ERROR);
    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteDeleteVolumeCommand(const DeleteVolumeRequest* request, DeleteVolumeResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string volumeName = "";
    string arrayName = "";

    volumeName = (request->param()).name();
    arrayName = (request->param()).array();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int eventId = EID(DELETE_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (info->arrayInfo->GetState() < ArrayStateEnum::NORMAL)
    {
        int eventId = EID(DELETE_VOL_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "array_name:{}, array_state:{}", arrayName, info->arrayInfo->GetState().ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IVolumeEventManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (volMgr != nullptr)
    {
        int ret = volMgr->Delete(volumeName);
        if (ret == SUCCESS)
        {
            int eventId = EID(SUCCESS);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        else
        {
            int eventId = ret;
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }

    int eventId = EID(DELETE_VOL_INTERNAL_ERROR);
    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteMountVolumeCommand(const MountVolumeRequest* request, MountVolumeResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string volumeName = "";
    string subnqn = "";
    string arrayName = "";

    volumeName = (request->param()).name();
    subnqn = (request->param()).subnqn();
    arrayName = (request->param()).array();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int eventId = EID(MOUNT_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (info->arrayInfo->GetState() < ArrayStateEnum::NORMAL)
    {
        int eventId = EID(MOUNT_VOL_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "array_name:{}, array_state:{}", arrayName, info->arrayInfo->GetState().ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IVolumeEventManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (volMgr != nullptr)
    {
        int ret = volMgr->Mount(volumeName, subnqn);
        if (ret == SUCCESS)
        {
            int eventId = EID(SUCCESS);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        else
        {
            int eventId = ret;
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }

    int eventId = EID(MOUNT_VOL_INTERNAL_ERROR);
    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteUnmountVolumeCommand(const UnmountVolumeRequest* request, UnmountVolumeResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string volumeName = "";
    string arrayName = "";

    volumeName = (request->param()).name();
    arrayName = (request->param()).array();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int eventId = EID(UNMOUNT_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (info->arrayInfo->GetState() < ArrayStateEnum::NORMAL)
    {
        int eventId = EID(UNMOUNT_VOL_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "array_name:{}, array_state:{}", arrayName, info->arrayInfo->GetState().ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IVolumeEventManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (volMgr != nullptr)
    {
        int ret = volMgr->Unmount(volumeName);
        if (ret == SUCCESS)
        {
            int eventId = EID(SUCCESS);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        else
        {
            int eventId = ret;
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }

    int eventId = EID(UNMOUNT_VOL_INTERNAL_ERROR);
    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status CommandProcessor::ExecuteListVolumeCommand(const ListVolumeRequest* request, ListVolumeResponse* reply)
{
    string command = request->command();
    string volumeName = "";
    string arrayName = "";

    reply->set_command(command);
    reply->set_rid(request->rid());

    arrayName = (request->param()).array();
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int eventId = EID(LIST_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    IArrayInfo* array = info->arrayInfo;
    ArrayStateType arrayState = array->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        int eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
        POS_TRACE_WARN(eventId, "arrayName: {}, arrayState: {}", arrayName, arrayState.ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }


    IVolumeInfoManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName); 

    int vol_cnt = 0;

    if (volMgr == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "The requested volume does not exist");
    }
    else
    {
        vol_cnt = volMgr->GetVolumeCount();
    }
    if (vol_cnt > 0)
    {
        VolumeList* volList = volMgr->GetVolumeList();
        int idx = -1;
        while (true)
        {
            VolumeBase* vol = volList->Next(idx);
            if (nullptr == vol)
            {
                break;
            }
            grpc_cli::Volume* volume = reply->mutable_result()->mutable_data()->add_volumes();
			volume->set_name(vol->GetVolumeName());
			volume->set_index(idx);
			volume->set_uuid(vol->GetUuid());
			volume->set_total(vol->GetTotalSize());
            
            VolumeMountStatus volumeStatus = vol->GetVolumeMountStatus();
            if (Mounted == volumeStatus)
            {
                volume->set_remain(vol->RemainingSize());
            }
            volume->set_status(volMgr->GetStatusStr(volumeStatus));
            volume->set_maxiops(vol->GetMaxIOPS());
            volume->set_miniops(vol->GetMinIOPS());
            volume->set_maxbw(vol->GetMaxBW());
            volume->set_minbw(vol->GetMinBW());
        }
        
        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;

}

grpc::Status CommandProcessor::ExecuteVolumeInfoCommand(const VolumeInfoRequest* request, VolumeInfoResponse* reply)
{
    string volumeName = "";
    string arrayName = "";
    arrayName = (request->param()).array();
    volumeName = (request->param()).volume();
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(VOLUME_INFO_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    IVolumeInfoManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (volMgr == nullptr)
    {
        int event = EID(VOL_NOT_FOUND);
        POS_TRACE_WARN(event,"Failed to get an IVolumeInfoManager instance. array name: " + arrayName +" volume name: "+ volumeName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    VolumeBase* vol = volMgr->GetVolume(volMgr->GetVolumeID(volumeName));

    if (vol == nullptr)
    {
        int event = EID(VOL_NOT_FOUND);
        POS_TRACE_WARN(event,"No such volume exists in array. array name: " + arrayName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    grpc_cli::Volume* volume = reply->mutable_result()->mutable_data();
    volume->set_name(vol->GetVolumeName());
    volume->set_uuid(vol->GetUuid());
    volume->set_total(vol->GetTotalSize());
    VolumeMountStatus VolumeMountStatus = vol->GetVolumeMountStatus();

    if (Mounted == VolumeMountStatus)
    {
        volume->set_remain(vol->RemainingSize());
    }
    volume->set_status(volMgr->GetStatusStr(VolumeMountStatus));
    volume->set_maxiops(vol->GetMaxIOPS());
    volume->set_maxbw(vol->GetMaxBW());
    volume->set_minbw(vol->GetMinBW());
    volume->set_miniops(vol->GetMinIOPS());
    volume->set_subnqn(vol->GetSubnqn());
    volume->set_arrayname(vol->GetArrayName());
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;

}

grpc::Status CommandProcessor::ExecuteVolumeRenameCommand(const VolumeRenameRequest* request, VolumeRenameResponse* reply)
{
    string volumeOldName = "";
    string volumeNewName = "";
    string arrayName = "";
    arrayName = (request->param()).array();
    volumeOldName = (request->param()).name();
    volumeNewName = (request->param()).newname(); 
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(RENAME_VOL_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (info->arrayInfo->GetState() < ArrayStateEnum::NORMAL)
    {
        int eventId = EID(RENAME_VOL_CAN_ONLY_BE_WHILE_ONLINE);
        POS_TRACE_WARN(eventId, "array_name:{}, array_state:{}", arrayName, info->arrayInfo->GetState().ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    int ret = EID(RENAME_VOL_INTERNAL_ERROR);
    if (volMgr != nullptr)
    {
        ret = volMgr->Rename(volumeOldName, volumeNewName);
        if (ret == SUCCESS)
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
    else
    {
        _SetEventStatus(ret, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}

grpc::Status CommandProcessor::ExecuteListQOSPolicyCommand(const ListQOSPolicyRequest* request, ListQOSPolicyResponse* reply)
    {
    std::vector<string> volumeNames;
    std::vector<uint32_t> volumeIds;
    string errorMsg;
    int validVol = -1;
    string volName;
    qos_backend_policy backendPolicy;
    qos_vol_policy volPolicy;
    string arrayName = (request->param()).array();
    int eventId;
    if (0 == arrayName.compare(""))
    {
        eventId = EID(QOS_INVALID_ARRAY_NAME);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        eventId = EID(QOS_ARRAY_DOES_NOT_EXIST);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    IArrayInfo* arrayInfo = info->arrayInfo;
    ArrayStateType arrayState = arrayInfo->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    backendPolicy = QosManagerSingleton::Instance()->GetBackendPolicy(BackendEvent_UserdataRebuild);
    string impact = _GetRebuildImpactString(backendPolicy.priorityImpact);
    IVolumeInfoManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (volMgr == nullptr)
    {
        int event = EID(VOL_NOT_FOUND);
        POS_TRACE_WARN(event,"Failed to get an IVolumeInfoManager instance. array name: " + arrayName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    grpc_cli::QOSResult* qosResult = reply->mutable_result()->mutable_data()->add_qosresult();
    grpc_cli::QOSResult_RebuildPolicy* rebuildpolicy = qosResult->add_rebuildpolicy();
	grpc_cli::QOSResult_Arrays* arrays = qosResult->add_arrayname();
    arrays->set_arrayname(arrayName);
    rebuildpolicy->set_rebuild(impact);
    for (int i = 0; i < request->param().vol().size(); i++)
    {
        string volName = (request->param()).vol()[i].volumename();
        validVol = volMgr->CheckVolumeValidity(volName);
        if (EID(SUCCESS) != validVol)
        {
            int event = EID(QOS_CLI_WRONG_MISSING_PARAMETER);
            POS_TRACE_WARN(event,"Invalid Volume Name " + volName);
            _SetEventStatus(event, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        int volId = volMgr->GetVolumeID(volName);
		grpc_cli::QOSResult_VolumePolicies* volumepolicies = qosResult->add_volumepolicies();
        volPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(volId, arrayName);
        volMgr->GetVolumeName(volId, volName);
        volumepolicies->set_name(volName);
        volumepolicies->set_id(volId);
        volumepolicies->set_maxbw(to_string(volPolicy.maxBw));
        volumepolicies->set_maxiops(to_string(volPolicy.maxIops));
        volumepolicies->set_miniops(to_string(volPolicy.minIops));
        volumepolicies->set_minbw(to_string(volPolicy.minBw));
        if (true == volPolicy.minBwGuarantee) 
            volumepolicies->set_min_bw_guarantee("Yes");
        else
            volumepolicies->set_min_bw_guarantee("No");
        if (true == volPolicy.minIopsGuarantee)
            volumepolicies->set_min_iops_guarantee("Yes");
        else
            volumepolicies->set_min_iops_guarantee("No");

    }
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}
grpc::Status
CommandProcessor::ExecuteSetVolumePropertyCommand(const SetVolumePropertyRequest* request, SetVolumePropertyResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    string volumeName = "";
    string arrayName = "";
    string newVolumeName = "";
    bool updatePrimaryVol = false;
    bool isPrimaryVol = false;

    volumeName = (request->param()).name();
    arrayName = (request->param()).array();
    newVolumeName = (request->param()).newvolumename();

    updatePrimaryVol = (request->param()).updateprimaryvol();
    isPrimaryVol = (request->param()).isprimaryvol();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int eventId = EID(SET_VOLUME_PROPERTY_FAILURE_ARRAY_NAME_DOES_NOT_EXIST);
        POS_TRACE_WARN(eventId, "array_name:{}", arrayName);
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    if (info->arrayInfo->GetState() < ArrayStateEnum::NORMAL)
    {
        int eventId = EID(SET_VOLUME_PROPERTY_FAILURE_ARRAY_NOT_ONLINE);
        POS_TRACE_WARN(eventId, "array_name:{}, array_state:{}", arrayName, info->arrayInfo->GetState().ToString());
        _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IVolumeEventManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (volMgr != nullptr)
    {
        if (newVolumeName != "")
        {
            int ret = volMgr->Rename(volumeName, newVolumeName);
            if (ret != SUCCESS)
            {
                _SetEventStatus(ret, reply->mutable_result()->mutable_status());
                _SetPosInfo(reply->mutable_info());
                return grpc::Status::OK;
            }
        }

        if (updatePrimaryVol == true)
        {
            int ret = isPrimaryVol ? volMgr->UpdateReplicationRole(volumeName, ReplicationRole::Primary)
                                   : volMgr->UpdateReplicationRole(volumeName, ReplicationRole::Secondary);

            if (ret != SUCCESS)
            {
                _SetEventStatus(ret, reply->mutable_result()->mutable_status());
                _SetPosInfo(reply->mutable_info());
                return grpc::Status::OK;
            }
        }
    }

    int eventId = EID(SUCCESS);
    _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteQosCreateVolumePolicyCommand(const QosCreateVolumePolicyRequest* request, QosCreateVolumePolicyResponse* reply)
{
    string command = request->command();

    reply->set_command(command);
    reply->set_rid(request->rid());

    std::vector<string> volumeNames;
    std::vector<std::pair<string, uint32_t>> validVolumes;
    std::string errorMsg;

    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        _SetEventStatus(EID(QOS_CLI_FE_QOS_DISABLED), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    string arrayName = (request->param()).array();
    int paramValidity = _HandleInputVolumes(arrayName, (request->param()).vol(), volumeNames, validVolumes);
    if (paramValidity != EID(SUCCESS))
    {
        _SetEventStatus(paramValidity, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        _SetEventStatus(EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IArrayInfo* array = info->arrayInfo;
    ArrayStateType arrayState = array->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        _SetEventStatus(EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    const int32_t NOT_INPUT = -1;
    int64_t maxBw = NOT_INPUT, minBw = NOT_INPUT, maxIops = NOT_INPUT, minIops = NOT_INPUT;
    if ((request->param()).minbw() != -1)
    {
        minBw = (request->param()).minbw();
    }
    if((request->param()).maxbw() != -1)
    {
        maxBw = (request->param()).maxbw();
    }
    if ((request->param()).miniops() != -1)
    {
        minIops = (request->param()).miniops();
    }
    if((request->param()).maxiops() != -1)
    {
        maxIops = (request->param()).maxiops();
    }

    int retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(minBw, maxBw,
        minIops, maxIops, errorMsg, validVolumes, arrayName);

    if (SUCCESS != retVal)
    {
        _SetEventStatus(retVal, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    for (auto volume : validVolumes)
    {
        IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
        qos_vol_policy policy = QosManagerSingleton::Instance()->GetVolumePolicy(volume.second, arrayName);
        retVal = volMgr->UpdateQoSProperty(volume.first, policy.maxIops, policy.maxBw, policy.minIops, policy.minBw);
    }
    if (SUCCESS != retVal)
    {
        _SetEventStatus(retVal, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;

}

grpc::Status
CommandProcessor::ExecuteQosResetVolumePolicyCommand(const QosResetVolumePolicyRequest* request, QosResetVolumePolicyResponse* reply)
{
    string command = request->command();

    reply->set_command(command);
    reply->set_rid(request->rid());

    std::vector<string> volumeNames;
    std::vector<std::pair<string, uint32_t>> validVolumes;
    std::string errorMsg;

    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        _SetEventStatus(EID(QOS_CLI_FE_QOS_DISABLED), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    string arrayName = (request->param()).array();
    int paramValidity = _HandleInputVolumes(arrayName, (request->param()).vol(), volumeNames, validVolumes);
    if (paramValidity != EID(SUCCESS))
    {
        _SetEventStatus(paramValidity, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        _SetEventStatus(EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    IArrayInfo* array = info->arrayInfo;
    ArrayStateType arrayState = array->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        _SetEventStatus(EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    qos_vol_policy newVolPolicy;
    IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    for (auto vol = validVolumes.begin(); vol != validVolumes.end(); vol++)
    {
        std::pair<string, uint32_t> volume = (*vol);
        newVolPolicy.minBwGuarantee = false;
        newVolPolicy.minIopsGuarantee = false;
        newVolPolicy.minBw = 0;
        newVolPolicy.maxBw = 0;
        newVolPolicy.minIops = 0;
        newVolPolicy.maxIops = 0;
        newVolPolicy.policyChange = true;
        newVolPolicy.maxValueChanged = true;
        int retVal = volMgr->UpdateQoSProperty(volume.first, newVolPolicy.maxIops, newVolPolicy.maxBw, newVolPolicy.minIops, newVolPolicy.minBw);
        if (retVal != SUCCESS)
        {
            _SetEventStatus(EID(retVal), reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        int32_t arrayId = QosManagerSingleton::Instance()->GetArrayIdFromMap(arrayName);
        if (arrayId != -1)
        {
            retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(volume.second, newVolPolicy, arrayId);
        }
        if (retVal != SUCCESS)
        {
            _SetEventStatus(EID(retVal), reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

std::string
CommandProcessor::_GetRebuildImpactString(uint8_t impact)
{
    switch (impact)
    {
        case PRIORITY_HIGH:
            return "high";

        case PRIORITY_MEDIUM:
            return "medium";

        case PRIORITY_LOW:
            return "low";

        default:
            return "unknown";
    }
}

void
CommandProcessor::_SetEventStatus(int eventId, grpc_cli::Status* status, std::string message /*=""*/)
{
    std::string eventName = "";
    std::string description = "";
    std::string cause = "";
    std::string solution = "";

    auto event_info = eventManager.GetEventInfo();
    auto it = event_info->find(eventId);
    if (it != event_info->end())
    {
        eventName = it->second.GetEventName();
        description = it->second.GetDescription();
        cause = it->second.GetCause();
        solution = it->second.GetSolution();
    }

    status->set_code(eventId);
    status->set_event_name(eventName);
    status->set_description(description);
    status->set_cause(cause);
    status->set_solution(solution);

    if (message != "")
    {
        status->set_message(message);
    }
}

void
CommandProcessor::_SetPosInfo(grpc_cli::PosInfo* posInfo)
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

void
CommandProcessor::_FillSmartData(
    struct spdk_nvme_health_information_page *payload,
    grpc_cli::SmartLog* data)
{
    char cString[128];

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.available_spare ? "WARNING" : "OK");
    string availableSpareSpace(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.device_reliability ? "WARNING" : "OK");
    string temperature(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.device_reliability ? "WARNING" : "OK");
    string deviceReliability(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.read_only ? "Yes" : "No");
    string readOnly(cString);

    snprintf(cString, sizeof(cString), "%s", payload->critical_warning.bits.volatile_memory_backup ? "WARNING" : "OK");
    string volatileMemoryBackup(cString);

    snprintf(cString, sizeof(cString), "%dC", (int)payload->temperature - 273);
    string currentTemperature(cString);

    snprintf(cString, sizeof(cString), "%u%%", payload->available_spare);
    string availableSpare(cString);

    snprintf(cString, sizeof(cString), "%u%%", payload->available_spare_threshold);
    string availableSpareThreshold(cString);

    snprintf(cString, sizeof(cString), "%u%%", payload->percentage_used);
    string lifePercentageUsed(cString);

    _PrintUint128Dec(payload->data_units_read, cString, sizeof(cString));
    string dataUnitsRead(cString);

    _PrintUint128Dec(payload->data_units_written, cString, sizeof(cString));
    string dataUnitsWritten(cString);

    _PrintUint128Dec(payload->host_read_commands, cString, sizeof(cString));
    string hostReadCommands(cString);

    _PrintUint128Dec(payload->host_write_commands, cString, sizeof(cString));
    string hostWriteCommands(cString);

    _PrintUint128Dec(payload->controller_busy_time, cString, sizeof(cString));
    string controllerBusyTime(cString);

    _PrintUint128Dec(payload->power_cycles, cString, sizeof(cString));
    string powerCycles(cString);

    _PrintUint128Dec(payload->power_on_hours, cString, sizeof(cString));
    string powerOnHours(cString);

    _PrintUint128Dec(payload->unsafe_shutdowns, cString, sizeof(cString));
    string unsafeShutdowns(cString);

    _PrintUint128Dec(payload->media_errors, cString, sizeof(cString));
    string unrecoverableMediaErrors(cString);

    _PrintUint128Dec(payload->num_error_info_log_entries, cString, sizeof(cString));
    string lifetimeErrorLogEntries(cString);

    snprintf(cString, sizeof(cString), "%um", payload->warning_temp_time);
    string warningTemperatureTime(cString);

    snprintf(cString, sizeof(cString), "%um", payload->critical_temp_time);
    string criticalTemperatureTime(cString);

    data->set_availablesparespace(availableSpareSpace);
    data->set_temperature(temperature);
    data->set_devicereliability(deviceReliability);
    data->set_readonly(readOnly);
    data->set_volatilememorybackup(volatileMemoryBackup);
    data->set_currenttemperature(currentTemperature);
    data->set_availablespare(availableSpare);
    data->set_availablesparethreshold(availableSpareThreshold);
    data->set_lifepercentageused(lifePercentageUsed);
    data->set_dataunitsread(dataUnitsRead);
    data->set_dataunitswritten(dataUnitsWritten);
    data->set_hostreadcommands(hostReadCommands);
    data->set_hostwritecommands(hostWriteCommands);
    data->set_controllerbusytime(controllerBusyTime);
    data->set_powercycles(powerCycles);
    data->set_poweronhours(powerOnHours);
    data->set_unsafeshutdowns(unsafeShutdowns);
    data->set_unrecoverablemediaerrors(unrecoverableMediaErrors);
    data->set_lifetimeerrorlogentries(lifetimeErrorLogEntries);
    data->set_warningtemperaturetime(warningTemperatureTime);
    data->set_criticaltemperaturetime(criticalTemperatureTime);

    for (int i = 0; i < 8; i++)
    {
        if (payload->temp_sensor[i] != 0)
        {
            snprintf(cString, sizeof(cString), "%dC", (int)payload->temp_sensor[i] - 273);
            string* temperature = data->add_temperaturesensor();
            *temperature = cString;
        }
    }
}

void
CommandProcessor::_PrintUint128Hex(uint64_t* v, char* s, size_t n)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        snprintf(s, n, "0x%llX%016llX", hi, lo);
    }
    else
    {
        snprintf(s, n, "0x%llX", lo);
    }
}

void
CommandProcessor::_PrintUint128Dec(uint64_t* v, char* s, size_t n)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        _PrintUint128Hex(v, s, n);
    }
    else
    {
        snprintf(s, n, "%llu", (unsigned long long)lo);
    }
}

CommandProcessor::BiosInfo
CommandProcessor::_GetBiosInfo()
{
    const std::string getBiosVersionCmd = "dmidecode -s bios-version";
    const std::string getBiosVendorCmd = "dmidecode -s bios-vendor";
    const std::string getBiosReleaseDateCmd = "dmidecode -s bios-release-date";

    BiosInfo bios;
    bios.version = _ExecuteLinuxCmd(getBiosVersionCmd);
    bios.vendor = _ExecuteLinuxCmd(getBiosVendorCmd);
    bios.releaseDate = _ExecuteLinuxCmd(getBiosReleaseDateCmd);

    return bios;
}

CommandProcessor::SystemInfo
CommandProcessor::_GetSystemInfo()
{
    const std::string getSystemManufacturerCmd = "dmidecode -s system-manufacturer";
    const std::string getSystemProductNameCmd = "dmidecode -s system-product-name";
    const std::string getSystemSerialNumberCmd = "dmidecode -s system-serial-number";
    const std::string getSystemUuidCmd = "dmidecode -s system-uuid";

    SystemInfo system;
    system.manufacturer = _ExecuteLinuxCmd(getSystemManufacturerCmd);
    system.productName = _ExecuteLinuxCmd(getSystemProductNameCmd);
    system.serialNumber = _ExecuteLinuxCmd(getSystemSerialNumberCmd);
    system.uuid = _ExecuteLinuxCmd(getSystemUuidCmd);

    return system;
}

CommandProcessor::BaseboardInfo
CommandProcessor::_GetBaseboardInfo()
{
    const std::string getBaseboardManufacturerCmd = "dmidecode -s baseboard-manufacturer";
    const std::string getBaseboardProductNameCmd = "dmidecode -s baseboard-product-name";
    const std::string getBaseboardSerialNumberCmd = "dmidecode -s baseboard-serial-number";
    const std::string getBaseboardVersionCmd = "dmidecode -s baseboard-version";

    BaseboardInfo baseboard;
    baseboard.manufacturer = _ExecuteLinuxCmd(getBaseboardManufacturerCmd);
    baseboard.productName = _ExecuteLinuxCmd(getBaseboardProductNameCmd);
    baseboard.serialNumber = _ExecuteLinuxCmd(getBaseboardSerialNumberCmd);
    baseboard.version = _ExecuteLinuxCmd(getBaseboardVersionCmd);

    return baseboard;
}

CommandProcessor::ProcessorInfo
CommandProcessor::_GetProcessorInfo()
{
    const std::string getProcessorManufacturerCmd = "dmidecode -s processor-manufacturer";
    const std::string getProcessorVersioneCmd = "dmidecode -s processor-version";
    const std::string getProcessorFrequencyCmd = "dmidecode -s processor-frequency";

    ProcessorInfo processor;
    processor.manufacturer = _ExecuteLinuxCmd(getProcessorManufacturerCmd);
    processor.version = _ExecuteLinuxCmd(getProcessorVersioneCmd);
    processor.frequency = _ExecuteLinuxCmd(getProcessorFrequencyCmd);

    return processor;
}

std::string
CommandProcessor::_ExecuteLinuxCmd(std::string command)
{
    char buffer[MAX_LINUX_CMD_LENGTH];
    string result = "";
    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe)
    {
        return "popen failed!";
    }

    while (!feof(pipe))
    {
        if ((fgets(buffer, MAX_LINUX_CMD_LENGTH, pipe) != NULL))
            result += buffer;
    }

    result.erase(std::remove(result.begin(), result.end(), '\n'),
        result.end());
    pclose(pipe);

    return result;
}

bool
CommandProcessor::_IsValidIpAddress(const std::string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

bool
CommandProcessor::_IsValidFile(const std::string& path)
{
    std::string extension = path.substr(path.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
    [](unsigned char c){ return std::tolower(c); });

    if(extension != "yaml") {
        return false;
    } 

    if (FILE *file = fopen(path.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }

    return false;
}

int
CommandProcessor::_HandleInputVolumes(
    const string arrayName,
    const RepeatedPtrField<QosVolumeNameParam>& volumes,
    std::vector<string>& volumeNames,
    std::vector<std::pair<string, uint32_t>>& validVolumes)
{
    int validVol = -1;
    volumeNames.clear();
    validVolumes.clear();


    if (0 == arrayName.compare(""))
    {
        return EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
    }

    for(int i = 0; i < volumes.size(); i++) {
        string volName = volumes[i].volumename();
        volumeNames.push_back(volName);
    }

    IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (nullptr == volMgr)
    {
        return EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
    }

    for (auto vol = volumeNames.begin(); vol != volumeNames.end(); vol++)
    {
        validVol = volMgr->CheckVolumeValidity(*vol);
        if (EID(SUCCESS) != validVol)
        {
            return validVol;
        }
        else
        {
            IVolumeInfoManager* volInfoMgr =
                VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
            validVolumes.push_back(std::make_pair(*vol, volInfoMgr->GetVolumeID(*vol)));
        }
    }
    return EID(SUCCESS);

}
