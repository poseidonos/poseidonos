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
    int32_t nsid = 0;
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
        string targetAddress = "";
        if (ret == SUCCESS)
        {
            ret = ArrayMgr()->GetTargetAddress(arrayName, targetAddress);
        }
        if (ret == SUCCESS)
        {
            reply->mutable_result()->mutable_data()->set_targetaddress(targetAddress);
            int eventId = EID(SUCCESS);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            RestoreManagerSingleton::Instance()->VolumeCreate(arrayName, volumeName, nsid);
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
            RestoreManagerSingleton::Instance()->VolumeDelete(arrayName, volumeName);
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
    uint32_t nsid = 0;

    volumeName = (request->param()).name();
    subnqn = (request->param()).subnqn();
    arrayName = (request->param()).array();
    nsid = (request->param()).nsid();

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
        int ret = volMgr->Mount(volumeName, subnqn, nsid);
        if (ret == SUCCESS)
        {
            int eventId = EID(SUCCESS);
            _SetEventStatus(eventId, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            IVolumeInfoManager* volInfoMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
            VolumeBase* vol = volInfoMgr->GetVolume(volInfoMgr->GetVolumeID(volumeName));
            RestoreManagerSingleton::Instance()->SetVolumeMountState(arrayName, volumeName, true, vol->GetSubnqn(), vol->GetNsid());
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
            RestoreManagerSingleton::Instance()->SetVolumeMountState(arrayName, volumeName, false);
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

grpc::Status
CommandProcessor::ExecuteListVolumeCommand(const ListVolumeRequest* request, ListVolumeResponse* reply)
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

grpc::Status
CommandProcessor::ExecuteVolumeInfoCommand(const VolumeInfoRequest* request, VolumeInfoResponse* reply)
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
        POS_TRACE_WARN(event, "Failed to get an IVolumeInfoManager instance. array name: " + arrayName + " volume name: " + volumeName);
        _SetEventStatus(event, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    VolumeBase* vol = volMgr->GetVolume(volMgr->GetVolumeID(volumeName));

    if (vol == nullptr)
    {
        int event = EID(VOL_NOT_FOUND);
        POS_TRACE_WARN(event, "No such volume exists in array. array name: " + arrayName);
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

grpc::Status
CommandProcessor::ExecuteVolumeRenameCommand(const VolumeRenameRequest* request, VolumeRenameResponse* reply)
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
            RestoreManagerSingleton::Instance()->VolumeRename(arrayName, volumeOldName, volumeNewName);
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
