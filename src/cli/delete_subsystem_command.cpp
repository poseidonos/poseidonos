#include "src/cli/delete_subsystem_command.h"

#include <map>
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/helper/spdk_rpc_client.h"
#include "src/network/nvmf_target.hpp"
#include "src/volume/volume_service.h"

namespace pos_cli
{
DeleteSubsystemCommand::DeleteSubsystemCommand(void)
{
    errorMessage = "Failed to delete subsystem.";
}

DeleteSubsystemCommand::~DeleteSubsystemCommand(void)
{
}

string
DeleteSubsystemCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    if (_CheckParamValidityAndGetNqn(doc) == false)
    {
        return jFormat.MakeResponse(
            "DELETESUBSYSTEM", rid, BADREQUEST,
            errorMessage, GetPosInfo());
    }

    int ret = 0;
    ret = _DeleteSubsystem(doc);
    if (ret != SUCCESS)
    {
        return jFormat.MakeResponse(
            "DELETESUBSYSTEM", rid, FAIL,
            errorMessage, GetPosInfo());
    }

    return jFormat.MakeResponse(
        "DELETESUBSYSTEM", rid, SUCCESS,
        "Subsystem ( " + subnqn + ") has been deleted.", GetPosInfo());
}

int
DeleteSubsystemCommand::_DeleteSubsystem(json& doc)
{
    SpdkRpcClient rpcClient;
    NvmfTarget target;
    vector<pair<int, string>> attachedVolList = target.GetAttachedVolumeList(subnqn);
    map<string, vector<int>> volListPerArray;
    for (auto& volInfo : attachedVolList)
    {
        volListPerArray[volInfo.second].push_back(volInfo.first);
    }
    for (auto& volList : volListPerArray)
    {
        string arrayName = volList.first;
        IVolumeManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

        for (auto& volId : volList.second)
        {
            int ret = FAIL;
            if (volMgr != nullptr)
            {
                string volName;
                ret = volMgr->VolumeName(volId, volName);
                if (ret != SUCCESS)
                {
                    errorMessage += "Failed to find volume name. Only some of volumes are unmounted.";
                    return ret;
                }
                ret = volMgr->Unmount(volName);
                if (ret != SUCCESS)
                {
                    errorMessage += "Failed to unmount volume. Only some of volumes are unmounted.";
                    return ret;
                }
            }
        }
    }

    auto ret = rpcClient.SubsystemDelete(subnqn);
    if (ret.first != SUCCESS)
    {
        errorMessage += ret.second;
    }
    return ret.first;
}

bool
DeleteSubsystemCommand::_CheckParamValidityAndGetNqn(json& doc)
{
    auto param = doc["param"];
    if (!param.contains("name"))
    {
        errorMessage += "Subsystem nqn must be included";
        return false;
    }
    subnqn = param["name"].get<string>();
    return true;
}
} // namespace pos_cli