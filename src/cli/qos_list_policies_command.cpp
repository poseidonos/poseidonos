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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "src/cli/qos_list_policies_command.h"
#include "src/cli/cli_event_code.h"
#include "src/qos/qos_manager.h"
#include "src/qos/qos_common.h"
#include "src/array/array.h"
#include "src/volume/volume_manager.h"

namespace pos_cli
{
QosListPoliciesCommand::QosListPoliciesCommand(void)
{
}

QosListPoliciesCommand::~QosListPoliciesCommand(void)
{
}

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

    if (doc["param"].contains("minbw"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "minbw, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("maxbw"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "maxbw, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("miniops"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "miniops, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("maxiops"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "maxiops, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("policy"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "policy, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("event"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "event, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("prio"))
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "prio, Invalid Parameter", GetPosInfo());
    }
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }
    JsonElement data("data");
    JsonArray rebuildImpact("rebuildPolicy");
    JsonArray volPolicies("volumePolicies");

    rebuildPolicy = QosManagerSingleton::Instance()->GetRebuildPolicy();
    string impact = GetRebuildImpactString(rebuildPolicy.rebuildImpact);

    JsonElement rebuild("");
    rebuild.SetAttribute(JsonAttribute("rebuild impact", "\"" + impact + "\""));
    rebuildImpact.AddElement(rebuild);
    data.SetArray(rebuildImpact);

    IVolumeManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (nullptr == volMgr)
    {
        return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, "Invalid Array Name", GetPosInfo());
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
                return jFormat.MakeResponse("QOSLISTPOLICIES", rid, BADREQUEST, errorMsg, GetPosInfo());
            }
            else
            {
                volumeIds.push_back(validVol);
            }
        }
        for (auto vol = volumeIds.begin(); vol != volumeIds.end(); vol++)
        {
            volPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(*vol);
            volMgr->VolumeName(*vol, volName);
            JsonElement volume("");
            volume.SetAttribute(JsonAttribute("name", "\"" + volName + "\""));
            volume.SetAttribute(JsonAttribute("id", to_string(*vol)));
            volume.SetAttribute(JsonAttribute("minbw", to_string(volPolicy.minBw)));
            volume.SetAttribute(JsonAttribute("miniops", to_string(volPolicy.minIops)));
            volume.SetAttribute(JsonAttribute("maxbw", to_string(volPolicy.maxBw)));
            volume.SetAttribute(JsonAttribute("maxiops", to_string(volPolicy.maxIops)));
            volume.SetAttribute(JsonAttribute("min bw guarantee", ((true == volPolicy.minBwGuarantee)? "\"Yes\"" : "\"No\"")));
            volume.SetAttribute(JsonAttribute("min iops guarantee", ((true == volPolicy.minIopsGuarantee)? "\"Yes\"" : "\"No\"")));
            volPolicies.AddElement(volume);
        }
        data.SetArray(volPolicies);
    }
    else
    {
        int vol_cnt = volMgr->GetVolumeCount();
        if (vol_cnt > 0)
        {
            VolumeList *volList = volMgr->GetVolumeList();
            int idx = -1;
            while (true)
            {
                VolumeBase *vol = volList->Next(idx);
                if (vol == nullptr)
                {
                    break;
                }
                volPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(idx);

                JsonElement volume("");
                volume.SetAttribute(JsonAttribute("name", "\"" + vol->GetName() + "\""));
                volume.SetAttribute(JsonAttribute("id", to_string(idx)));
                volume.SetAttribute(JsonAttribute("minbw", to_string(volPolicy.minBw)));
                volume.SetAttribute(JsonAttribute("maxbw", to_string(volPolicy.maxBw)));
                volume.SetAttribute(JsonAttribute("miniops", to_string(volPolicy.minIops)));
                volume.SetAttribute(JsonAttribute("maxiops", to_string(volPolicy.maxIops)));
                volume.SetAttribute(JsonAttribute("min bw guarantee", ((true == volPolicy.minBwGuarantee)? "\"Yes\"" : "\"No\"")));
                volume.SetAttribute(JsonAttribute("min iops guarantee", ((true == volPolicy.minIopsGuarantee)? "\"Yes\"" : "\"No\"")));
                volPolicies.AddElement(volume);
            }
            data.SetArray(volPolicies);
        }
    }
    return jFormat.MakeResponse("QOSLISTPOLICIES", rid, SUCCESS, "List of Volume Policies in " + arrayName, data, GetPosInfo());
}

string
QosListPoliciesCommand::GetRebuildImpactString(uint8_t impact)
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
