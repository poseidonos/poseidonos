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
            subnqn, serialNumber, modelNumber, maxNamespaces, allowAnyHost, anaReporting, ret.second);
        _SetEventStatus(EID(CREATE_SUBSYSTEM_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    POS_TRACE_INFO(EID(SUCCESS), "subnqn:{}", subnqn);
    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    RestoreManagerSingleton::Instance()->SubsystemCreate(subnqn, serialNumber, modelNumber, maxNamespaces, allowAnyHost, anaReporting);
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
    RestoreManagerSingleton::Instance()->SubsystemDelete(subnqn);
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
    RestoreManagerSingleton::Instance()->ListenerAdd(subnqn, transportType, targetAddress, transportServiceId);
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteRemoveListenerCommand(const RemoveListenerRequest* request, RemoveListenerResponse* reply)
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
        POS_TRACE_INFO(EID(REMOVE_LISTENER_FAILURE_NO_SUBNQN), "subnqn:{}", subnqn);
        _SetEventStatus(EID(REMOVE_LISTENER_FAILURE_NO_SUBNQN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    auto ret = rpcClient.SubsystemRemoveListener(
        subnqn,
        transportType,
        DEFAULT_ADRFAM,
        targetAddress,
        transportServiceId);

    if (ret.first != SUCCESS)
    {
        POS_TRACE_INFO(EID(REMOVE_LISTENER_FAILURE_SPDK_FAILURE), "subnqn:{}, rpc_return:{}", subnqn, ret.second);
        _SetEventStatus(EID(REMOVE_LISTENER_FAILURE_SPDK_FAILURE), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    RestoreManagerSingleton::Instance()->ListenerRemove(subnqn, transportType, targetAddress, transportServiceId);
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteListListenerCommand(const ListListenerRequest* request, ListListenerResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    SpdkRpcClient rpcClient;
    NvmfTarget target;

    string subnqn = (request->param()).subnqn();

    if (nullptr == target.FindSubsystem(subnqn))
    {
        POS_TRACE_INFO(EID(LIST_LISTENER_FAILURE_NO_SUBNQN), "subnqn:{}", subnqn);
        _SetEventStatus(EID(LIST_LISTENER_FAILURE_NO_SUBNQN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    auto list = rpcClient.SubsystemListListener(subnqn);

    for (const auto& listener : list)
    {
        grpc_cli::Listener* listenerListItem =
            reply->mutable_result()->mutable_data()->add_listenerlist();
        listenerListItem->set_anastate(listener["ana_state"].asString());
        auto addr = listenerListItem->mutable_address();
        addr->set_adrfam(listener["address"]["adrfam"].asString());
        addr->set_traddr(listener["address"]["traddr"].asString());
        addr->set_trsvcid(listener["address"]["trsvcid"].asString());
        addr->set_trtype(listener["address"]["trtype"].asString());
        addr->set_uuid(listener["address"]["uuid"].asString());
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteSetListenerAnaStateCommand(const SetListenerAnaStateRequest* request, SetListenerAnaStateResponse* reply)
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
    string anaState = (request->param()).anastate();

    if (nullptr == target.FindSubsystem(subnqn))
    {
        POS_TRACE_INFO(EID(SET_LISTENER_ANA_STATE_FAILURE_NO_SUBNQN), "subnqn:{}", subnqn);
        _SetEventStatus(EID(SET_LISTENER_ANA_STATE_FAILURE_NO_SUBNQN), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }

    auto ret = rpcClient.SubsystemSetListenerAnaState(
        subnqn,
        transportType,
        DEFAULT_ADRFAM,
        targetAddress,
        transportServiceId,
        anaState);

    if (ret.first != SUCCESS)
    {
        POS_TRACE_INFO(EID(SET_LISTENER_ANA_STATE_FAILURE_SPDK_FAILURE), "subnqn:{}, rpc_return:{}", subnqn, ret.second);
        _SetEventStatus(EID(SET_LISTENER_ANA_STATE_FAILURE_SPDK_FAILURE), reply->mutable_result()->mutable_status());
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
            addressInfoListItem->set_uuid(address["uuid"].asString());
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
            subsystemListItem->set_uuid(subsystem["uuid"].asString());
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
                addressInfoListItem->set_uuid(address["uuid"].asString());
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
                subsystemListItem->set_uuid(subsystem["uuid"].asString());
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
    bufCacheSize = ((request->param()).bufcachesize()) != 0 ? (request->param()).bufcachesize() : DEFAULT_BUF_CACHE_SIZE;
    numSharedBuf = ((request->param()).numsharedbuf()) != 0 ? (request->param()).numsharedbuf() : DEFAULT_NUM_SHARED_BUF;

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
    RestoreManagerSingleton::Instance()->TransportCreate(trType, bufCacheSize, numSharedBuf, ioUnitSize);
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteListTransportCommand(const ListTransportRequest* request, ListTransportResponse* reply)
{
    string command = request->command();
    reply->set_command(command);
    reply->set_rid(request->rid());

    SpdkRpcClient rpcClient;

    auto list = rpcClient.TransportList();

    for (const auto& transport : list)
    {
        grpc_cli::Transport* trListItem =
            reply->mutable_result()->mutable_data()->add_transportlist();
        trListItem->set_type(transport["trtype"].asString());
        trListItem->set_maxqueuedepth(transport["max_queue_depth"].asInt());
        trListItem->set_maxioqpairsperctrlr(transport["max_io_qpairs_per_ctrlr"].asInt());
        trListItem->set_incapsuledatasize(transport["in_capsule_data_size"].asInt());
        trListItem->set_maxiosize(transport["max_io_size"].asInt());
        trListItem->set_iounitsize(transport["io_unit_size"].asInt());
        trListItem->set_aborttimeoutsec(transport["abort_timeout_sec"].asInt());
        trListItem->set_bufcachesize(transport["buf_cache_size"].asInt());
        trListItem->set_numsharedbuf(transport["num_shared_buffers"].asInt());
    }

    _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
    _SetPosInfo(reply->mutable_info());
    return grpc::Status::OK;
}
