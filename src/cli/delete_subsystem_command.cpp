/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Samsung Electronics Corporation nor the names of
 *       its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "src/cli/delete_subsystem_command.h"

#include <map>
#include <utility>
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/network/nvmf_target.h"
#include "src/volume/volume_service.h"

namespace pos_cli
{
DeleteSubsystemCommand::DeleteSubsystemCommand(void)
: nvmfTarget(NvmfTargetSingleton::Instance())
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
DeleteSubsystemCommand::~DeleteSubsystemCommand(void)
{
}
// LCOV_EXCL_STOP

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
            "DELETESUBSYSTEM", rid, ret, errorMessage, GetPosInfo());
    }

    return jFormat.MakeResponse(
        "DELETESUBSYSTEM", rid, SUCCESS,
        "Subsystem ( " + subnqn + ") has been deleted.", GetPosInfo());
}

int
DeleteSubsystemCommand::_DeleteSubsystem(json& doc)
{
    SpdkRpcClient rpcClient;
    if (nullptr == nvmfTarget->FindSubsystem(subnqn))
    {
        errorMessage = "Failed to delete subsystem. Requested Subsystem does not exist or invalid subnqn. ";
        return FAIL;
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
            int ret = FAIL;
            if (volMgr != nullptr)
            {
                string volName;
                ret = volMgr->Unmount(volId);
                if (ret == EID(VOL_NOT_FOUND))
                {
                    errorMessage = "Failed to delete subsystem. Failed to find volume name. Only some of volumes are unmounted.";
                    return ret;
                }
                else if (ret != SUCCESS)
                {
                    errorMessage = "Failed to delete subsystem. Failed to unmount volume. Only some of volumes are unmounted.";
                    return ret;
                }
            }
        }
    }

    auto ret = rpcClient.SubsystemDelete(subnqn);
    if (ret.first != SUCCESS)
    {
        errorMessage = "Failed to delete subsystem. " + ret.second;
    }
    return ret.first;
}

bool
DeleteSubsystemCommand::_CheckParamValidityAndGetNqn(json& doc)
{
    auto param = doc["param"];
    if (!param.contains("name"))
    {
        errorMessage = "Failed to delete subsystem. Subsystem nqn must be included.";
        return false;
    }
    subnqn = param["name"].get<string>();
    return true;
}
} // namespace pos_cli
