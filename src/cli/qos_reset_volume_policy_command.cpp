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

#include "src/cli/qos_reset_volume_policy_command.h"
#include "src/cli/cli_event_code.h"
#include "src/qos/qos_manager.h"
#include "src/qos/qos_common.h"
#include "src/volume/volume_manager.h"

namespace pos_cli
{
QosResetVolumePolicyCommand::QosResetVolumePolicyCommand(void)
{
}

QosResetVolumePolicyCommand::~QosResetVolumePolicyCommand(void)
{
}

string
QosResetVolumePolicyCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }
    JsonFormat jFormat;
    std::vector<string> volumeNames;
    std::vector<uint32_t> volumeIds;
    string errorMsg;
    int validVol = -1;
    int retVal = -1;
    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "FE QOS Disabled", GetPosInfo());
    }
    if (doc["param"].contains("minbw"))
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "minbw, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("maxbw"))
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "maxbw, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("miniops"))
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "miniops, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("maxiops"))
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "maxiops, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("level"))
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "level, Invalid Parameter", GetPosInfo());
    }
    if (doc["param"].contains("vol"))
    {
        for (unsigned int i = 0; i < doc["param"]["vol"].size(); i++)
        {
            string volName = doc["param"]["vol"][i]["volumeName"];
            volumeNames.push_back(volName);
        }
        IVolumeManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
        if (nullptr == volMgr)
        {
            return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "Invalid Array Name", GetPosInfo());
        }
        for (auto vol = volumeNames.begin(); vol != volumeNames.end(); vol++)
        {
            validVol = volMgr->VolumeID(*vol);
            if (-1 == validVol)
            {
                errorMsg = "Invalid Volume Name " + (*vol);
                return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, errorMsg, GetPosInfo());
            }
            else
            {
                volumeIds.push_back(validVol);
            }
        }
    }
    else
    {
        return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, BADREQUEST, "vol, Parameter Missing", GetPosInfo());
    }

    qos_vol_policy newVolPolicy;
    for (auto vol = volumeIds.begin(); vol != volumeIds.end(); vol++)
    {
        newVolPolicy.minBwGuarantee = false;
        newVolPolicy.minIopsGuarantee = false;
        newVolPolicy.minBw = 0;
        newVolPolicy.maxBw = 0;
        newVolPolicy.minIops = 0;
        newVolPolicy.maxIops = 0;
        newVolPolicy.policyChange = true;
        newVolPolicy.maxValueChanged = true;
        retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(*vol, newVolPolicy);
        if (retVal != SUCCESS)
        {
            return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, retVal, "FAILED", GetPosInfo());
        }
    }
    return jFormat.MakeResponse("QOSRESETVOLUMEPOLICY", rid, SUCCESS, "Volume Qos Policy Reset", GetPosInfo());
}
} // namespace pos_cli
