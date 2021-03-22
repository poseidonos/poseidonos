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

#include "src/cli/update_volume_min_policy_command.h"

#include "src/cli/cli_event_code.h"
#include "src/volume/volume_service.h"
#include "src/qos/qos_common.h"

namespace pos_cli
{
UpdateVolumeMinPolicyCommand::UpdateVolumeMinPolicyCommand(void)
{
}

UpdateVolumeMinPolicyCommand::~UpdateVolumeMinPolicyCommand(void)
{
}

string
UpdateVolumeMinPolicyCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    JsonFormat jFormat;
    if (doc["param"].contains("name"))
    {
        string volName = doc["param"]["name"].get<std::string>();
        int minBW = 0;
        int ioType = 0;
        int ret = FAIL;
        struct qos_vol_policy prevVolPolicy;
        struct qos_vol_policy newVolPolicy;

        IVolumeManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

        if (volMgr != nullptr)
        {
            prevVolPolicy = volMgr->GetVolumePolicy(volName);
        }
        else
        {
            return jFormat.MakeResponse("UPDATEVOLUMEPOLICY", rid, BADREQUEST, "Specified Volume Not Found", GetPosInfo());
        }

        if (doc["param"].contains("iotype"))
        {
            ioType = doc["param"]["iotype"].get<uint64_t>();
            if (ioType >= QosWorkloadType_Mixed)
            {
                return jFormat.MakeResponse("UPDATEVOLUMEPOLICY", rid, BADREQUEST, "Wrong IO Type.Only 0,1 supported", GetPosInfo());
            }
            newVolPolicy.workLoad = static_cast<QosWorkloadType>(ioType);
        }
        else
        {
            newVolPolicy.workLoad = prevVolPolicy.workLoad;
        }

        if (doc["param"].contains("minbw"))
        {
            minBW = doc["param"]["minbw"].get<uint64_t>();
            newVolPolicy.minBW = minBW;
        }
        else
        {
            newVolPolicy.minBW = prevVolPolicy.minBW;
        }

        if (volMgr != nullptr)
        {
            ret = volMgr->UpdateVolumePolicy(volName, newVolPolicy);
        }
        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, ret, "DONE", GetPosInfo());
        }
        else if (ret == QosReturnCode::EVENT_POLICY_IN_EFFECT)
        {
            return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, ret, "Event WRR Policy In Effect", GetPosInfo());
        }
        else
        {
            return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, ret, "FAILED", GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, BADREQUEST, "Check parameters", GetPosInfo());
    }
}
}; // namespace pos_cli
