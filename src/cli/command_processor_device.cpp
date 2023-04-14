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
    RestoreManagerSingleton::Instance()->DeviceCreate(name, devType, blockSize, numBlocks, numa);
    return grpc::Status::OK;
}

grpc::Status
CommandProcessor::ExecuteScanDeviceCommand(const ScanDeviceRequest* request, ScanDeviceResponse* reply)
{
    reply->set_command(request->command());
    reply->set_rid(request->rid());

    bool isInitScan = DeviceManagerSingleton::Instance()->ScanDevs();
    if (isInitScan == true)
    {
        int result = ArrayManagerSingleton::Instance()->Load();
        if (result != 0)
        {
            POS_TRACE_WARN(result, "");
        }
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
