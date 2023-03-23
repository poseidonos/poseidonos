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
        RestoreManagerSingleton::Instance()->ArrayCreate(arrayName);
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
        RestoreManagerSingleton::Instance()->ArrayCreate(arrayName);
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
    RestoreManagerSingleton::Instance()->ArrayDelete(arrayName);
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

        array->SetTargetAddress(arrayName, targetAddress);
    }

    QosManagerSingleton::Instance()->UpdateArrayMap(arrayName);
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    RestoreManagerSingleton::Instance()->SetArrayMountState(arrayName, true, isWTenabled, targetAddress);
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
    RestoreManagerSingleton::Instance()->SetArrayMountState(arrayName, false, false);
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteListArrayCommand(const ListArrayRequest* request, ListArrayResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    vector<const ComponentsInfo*> infoList = ArrayMgr()->GetInfo();
    if (infoList.size() == 0)
    {
        int result = EID(CLI_LIST_ARRAY_NO_ARRAY_EXISTS);
        _SetEventStatus(result, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    for (const ComponentsInfo* ci : infoList)
    {
        IArrayInfo* info = ci->arrayInfo;
        JsonArray jsonArrayList("arrayList");

        string arrayName(info->GetName());
        string createDatetime(info->GetCreateDatetime());
        string updateDatetime(info->GetUpdateDatetime());
        string arrayStatus(DEFAULT_ARRAY_STATUS);
        grpc_cli::Array* array =
            reply->mutable_result()->mutable_data()->add_arraylist();
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

    for (auto dev : arrayInfo->GetDevices())
    {
        ArrayDeviceType type = dev->GetType();
        string typeStr = "NONE";
        if (type == ArrayDeviceType::DATA)
        {
            typeStr = "DATA";
        }
        else if (type == ArrayDeviceType::SPARE)
        {
            typeStr = "SPARE";
        }
        else if (type == ArrayDeviceType::NVM)
        {
            typeStr = "BUFFER";
        }
        grpc_cli::Device* device = array->add_devicelist();
        device->set_type(typeStr);
        device->set_name(dev->GetName());
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
    IArrayMgmt* array = ArrayMgr();
    int ret = array->Rebuild(arrayName);
    _SetEventStatus(ret, reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}
