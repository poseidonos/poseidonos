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

#include "src/cli/list_qos_policies_command.h"

#include "src/array/array.h"
#include "src/cli/cli_event_code.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_manager.h"
#include "src/array_mgmt/array_manager.h"

#include <vector>

namespace pos_cli
{
QosListPoliciesCommand::QosListPoliciesCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
QosListPoliciesCommand::~QosListPoliciesCommand(void)
{
}
// LCOV_EXCL_STOP

string
QosListPoliciesCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    std::vector<string> volumeNames;
    std::vector<uint32_t> volumeIds;
    string errorMsg;
    int validVol = -1;
    string volName;
    qos_rebuild_policy rebuildPolicy;
    qos_vol_policy volPolicy;

    string arrayName = "";
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
        if (0 == arrayName.compare(""))
        {
            return jFormat.MakeResponse("LISTQOSPOLICIES", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "Array Name Missing", GetPosInfo());
        }
    }
    JsonElement data("data");

    JsonArray rebuildImpact("rebuildPolicy");
    JsonArray volPolicies("volumePolicies");
    JsonArray arrayJson("arrayName");

    JsonElement array("");
    array.SetAttribute(JsonAttribute("ArrayName", "\"" + arrayName + "\""));
    arrayJson.AddElement(array);
    data.SetArray(arrayJson);


    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        return jFormat.MakeResponse("LISTQOSPOLICIES", rid, FAIL,
             "failed to list qos volume: " + volName, GetPosInfo());
    }

    IArrayInfo* arrayInfo = info->arrayInfo;
    ArrayStateType arrayState = arrayInfo->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        int eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
        POS_TRACE_WARN(eventId, "arrayName: {}, arrayState: {}",
            arrayName, arrayState.ToString());

        return jFormat.MakeResponse("LISTQOSPOLICIES", rid, FAIL,
             "failed to list qos volume: " + volName, GetPosInfo());
    }

    rebuildPolicy = QosManagerSingleton::Instance()->GetRebuildPolicy(arrayName);
    string impact = _GetRebuildImpactString(rebuildPolicy.rebuildImpact);

    JsonElement rebuild("");
    rebuild.SetAttribute(JsonAttribute("rebuild", "\"" + impact + "\""));
    rebuildImpact.AddElement(rebuild);
    data.SetArray(rebuildImpact);

    IVolumeManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (nullptr == volMgr)
    {
        return jFormat.MakeResponse("LISTQOSPOLICIES", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "Invalid Array Name", GetPosInfo());
    }
    if (doc["param"].contains("vol"))
    {
        for (unsigned int i = 0; i < doc["param"]["vol"].size(); i++)
        {
            string volName = doc["param"]["vol"][i]["volumeName"];
            volumeNames.push_back(volName);
        }
        for (auto vol = volumeNames.begin(); vol != volumeNames.end(); vol++)
        {
            validVol = volMgr->VolumeID(*vol);
            if (-1 == validVol)
            {
                errorMsg = "Invalid Volume Name " + (*vol);
                return jFormat.MakeResponse("LISTQOSPOLICIES", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), errorMsg, GetPosInfo());
            }
            else
            {
                volumeIds.push_back(validVol);
            }
        }
        for (auto vol = volumeIds.begin(); vol != volumeIds.end(); vol++)
        {
            volPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(*vol, arrayName);
            volMgr->VolumeName(*vol, volName);
            JsonElement volume("");
            volume.SetAttribute(JsonAttribute("name", "\"" + volName + "\""));
            volume.SetAttribute(JsonAttribute("id", to_string(*vol)));
            volume.SetAttribute(JsonAttribute("minbw", to_string(volPolicy.minBw)));
            volume.SetAttribute(JsonAttribute("miniops", to_string(volPolicy.minIops)));
            volume.SetAttribute(JsonAttribute("maxbw", to_string(volPolicy.maxBw)));
            volume.SetAttribute(JsonAttribute("maxiops", to_string(volPolicy.maxIops)));
            volume.SetAttribute(JsonAttribute("min_bw_guarantee", ((true == volPolicy.minBwGuarantee) ? "\"Yes\"" : "\"No\"")));
            volume.SetAttribute(JsonAttribute("min_iops_guarantee", ((true == volPolicy.minIopsGuarantee) ? "\"Yes\"" : "\"No\"")));
            volPolicies.AddElement(volume);
        }
        data.SetArray(volPolicies);
    }
    return jFormat.MakeResponse("LISTQOSPOLICIES", rid, SUCCESS, "List of Volume Policies in " + arrayName, data, GetPosInfo());
}

string
QosListPoliciesCommand::_GetRebuildImpactString(uint8_t impact)
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
} // namespace pos_cli
