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
#include "src/wbt/wbt_cmd_handler.h"

grpc::Status
CommandProcessor::ExecuteListWBTCommand(const ListWBTRequest* request, ListWBTResponse* reply)
{
    std::list<string> testlist;
    std::list<string>::iterator it;
    reply->set_command(request->command());
    reply->set_rid(request->rid());
    int ret = WbtCmdHandler("list_wbt").GetTestList(*&testlist);

    if (ret >= 0)
    {
        for (it = testlist.begin(); it != testlist.end(); it++)
        {
            grpc_cli::WBTTest* test =
                reply->mutable_result()->mutable_data()->add_testlist();
            test->set_testname(*it);
        }

        _SetEventStatus(EID(SUCCESS), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
    else
    {
        _SetEventStatus(EID(WBT_LIST_FETCH_ERROR), reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}

grpc::Status
CommandProcessor::ExecuteWBTCommand(const WBTRequest* request, WBTResponse* reply)
{
    std::vector<pair<string, string>> dataAttr;
    reply->set_command(request->command());
    reply->set_rid(request->rid());
    string testname = (request->param()).testname();
    map<string, string> argv((request->param()).argv().begin(), (request->param()).argv().end());

    pos::WbtCmdHandler wbtCmdHandler(testname);

    int ret = 0;
    int64_t cmdRetValue;
    JsonElement retElem("json");
    string errMsg = "fail";

    try
    {
        if (wbtCmdHandler.VerifyWbtCommand())
        {
            cmdRetValue = wbtCmdHandler(argv, retElem);
        }
        else
        {
            ret = -1;
            errMsg = "invalid wbt command";
        }

        if (ret != 0)
        {
            _SetEventStatus(ret, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
        else
        {
            retElem.SetAttribute(JsonAttribute("returnCode", to_string(cmdRetValue)));

            if (cmdRetValue == pos_cli::FAIL)
            {
                reply->mutable_result()->mutable_data()->set_testdata(retElem.ToJson());
                _SetEventStatus(cmdRetValue, reply->mutable_result()->mutable_status());
                _SetPosInfo(reply->mutable_info());
                return grpc::Status::OK;
            }
            reply->mutable_result()->mutable_data()->set_testdata(retElem.ToJson());
            _SetEventStatus(pos_cli::SUCCESS, reply->mutable_result()->mutable_status());
            _SetPosInfo(reply->mutable_info());
            return grpc::Status::OK;
        }
    }
    catch (const std::exception& e)
    {
        retElem.SetAttribute(JsonAttribute("returnCode", to_string(pos_cli::BADREQUEST)));
        reply->mutable_result()->mutable_data()->set_testdata(retElem.ToJson());
        _SetEventStatus(pos_cli::BADREQUEST, reply->mutable_result()->mutable_status());
        _SetPosInfo(reply->mutable_info());
        return grpc::Status::OK;
    }
}
