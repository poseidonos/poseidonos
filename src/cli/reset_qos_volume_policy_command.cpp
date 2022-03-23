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

#include "src/cli/reset_qos_volume_policy_command.h"

#include <utility>
#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_manager.h"

namespace pos_cli
{
QosResetVolumePolicyCommand::QosResetVolumePolicyCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
QosResetVolumePolicyCommand::~QosResetVolumePolicyCommand(void)
{
}
// LCOV_EXCL_STOP

string
QosResetVolumePolicyCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    std::vector<string> volumeNames;
    std::vector<std::pair<string, uint32_t>> validVolumes;
    string errorMsg = "";
    string arrayName = "";
    int validVol = -1;
    int retVal = -1;
    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, QosReturnCode::FAILURE, "Fe qos is disabled. So skipping QOS Settings.", GetPosInfo());
    }
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }
    if (0 == arrayName.compare(""))
    {
        return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "Array Name Missing", GetPosInfo());
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    IArrayInfo* array = info->arrayInfo;
    ArrayStateType arrayState = array->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        int eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
        POS_TRACE_WARN(eventId, "arrayName: {}, arrayState: {}",
            arrayName, arrayState.ToString());
        return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, FAIL,
             "failed to reset qos policy for volumes", GetPosInfo());
    }

    IVolumeManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (nullptr == volMgr)
    {
        return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "Invalid Array Name", GetPosInfo());
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
                return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), errorMsg, GetPosInfo());
            }
            else
            {
                validVolumes.push_back(std::make_pair(*vol, validVol));
            }
        }
    }
    else
    {
        return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "vol, Parameter Missing", GetPosInfo());
    }

    qos_vol_policy newVolPolicy;
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
        retVal = volMgr->UpdateQoS(volume.first, newVolPolicy.maxIops, newVolPolicy.maxBw, newVolPolicy.minIops, newVolPolicy.minBw);
        if (retVal != SUCCESS)
        {
            return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, retVal, "FAILED", GetPosInfo());
        }
        int32_t arrayId = QosManagerSingleton::Instance()->GetArrayIdFromMap(arrayName);
        if (arrayId != -1)
        {
            retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(volume.second, newVolPolicy, arrayId);
        }
        if (retVal != SUCCESS)
        {
            return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, retVal, "FAILED", GetPosInfo());
        }
    }
    return jFormat.MakeResponse("RESETQOSVOLUMEPOLICY", rid, SUCCESS, "Volume Qos Policy Reset", GetPosInfo());
}
} // namespace pos_cli
