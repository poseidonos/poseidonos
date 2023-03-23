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
CommandProcessor::ExecuteListQOSPolicyCommand(const ListQOSPolicyRequest* request, ListQOSPolicyResponse* reply)
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
        POS_TRACE_WARN(event, "Failed to get an IVolumeInfoManager instance. array name: " + arrayName);
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
            POS_TRACE_WARN(event, "Invalid Volume Name " + volName);
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
            RestoreManagerSingleton::Instance()->VolumeRename(arrayName, volumeName, newVolumeName);
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
    if ((request->param()).maxbw() != -1)
    {
        maxBw = (request->param()).maxbw();
    }
    if ((request->param()).miniops() != -1)
    {
        minIops = (request->param()).miniops();
    }
    if ((request->param()).maxiops() != -1)
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
