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

#include "src/cli/create_qos_volume_policy_command.h"

#include "src/cli/cli_event_code.h"
#include "src/include/array_config.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_base.h"
#include "src/volume/volume_manager.h"
namespace pos_cli
{
QosCreateVolumePolicyCommand::QosCreateVolumePolicyCommand(void)
{
    maxBw = 0;
    maxIops = 0;
    arrayName = "";
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
QosCreateVolumePolicyCommand::~QosCreateVolumePolicyCommand(void)
{
}
// LCOV_EXCL_STOP

string
QosCreateVolumePolicyCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    int retVal = -1;
    bool validInput = false;
    string ioType;
    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, EID(QOS_CLI_FE_QOS_DISABLED), "Fe qos is disabled. So skipping QOS Settings.", GetPosInfo());
    }

    if (doc["param"].contains("array") && doc["param"].contains("vol"))
    {
        string arrayName = doc["param"]["array"].get<std::string>();

        ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
        if (info == nullptr)
        {
            return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, FAIL,
                 "failed to create a qos policy", GetPosInfo());
        }

        IArrayInfo* array = info->arrayInfo;
        ArrayStateType arrayState = array->GetState();
        if (arrayState == ArrayStateEnum::BROKEN)
        {
            int eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
            POS_TRACE_WARN(eventId, "arrayName: {}, arrayState: {}",
                arrayName, arrayState.ToString());

            return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, FAIL,
                 "failed to create a qos policy", GetPosInfo());
        }

        validInput = _HandleInputVolumes(doc);
        if (false == validInput)
        {
            return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, static_cast<int>(EID(QOS_CLI_WRONG_MISSING_PARAMETER)), errorMsg, GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, static_cast<int>(EID(QOS_CLI_WRONG_MISSING_PARAMETER)), "vol, Parameter Missing", GetPosInfo());
    }
    const int32_t NOT_INPUT = -1;
    int64_t maxBw = NOT_INPUT, minBw = NOT_INPUT, maxIops = NOT_INPUT, minIops = NOT_INPUT;
    if (doc["param"].contains("minbw"))
    {
        minBw = doc["param"]["minbw"].get<int64_t>();
    }
    if (doc["param"].contains("maxbw"))
    {
        maxBw = doc["param"]["maxbw"].get<int64_t>();
    }
    if (doc["param"].contains("miniops"))
    {
        minIops = doc["param"]["miniops"].get<int64_t>();
    }
    if (doc["param"].contains("maxiops"))
    {
        maxIops = doc["param"]["maxiops"].get<int64_t>();
    }

    retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(minBw, maxBw,
        minIops, maxIops, errorMsg, validVolumes, arrayName);
    if (SUCCESS != retVal)
    {
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, retVal, errorMsg, GetPosInfo());
    }
    for (auto volume : validVolumes)
    {
        IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
        qos_vol_policy policy = QosManagerSingleton::Instance()->GetVolumePolicy(volume.second, arrayName);
        retVal = volMgr->UpdateQoSProperty(volume.first, policy.maxIops, policy.maxBw, policy.minIops, policy.minBw);
    }
    if (retVal != SUCCESS)
    {
        errorMsg = "QoS update in Volume Manager failed";
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, retVal, errorMsg, GetPosInfo());
    }

    if (SUCCESS != retVal)
    {
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, retVal, errorMsg, GetPosInfo());
    }
    return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, SUCCESS, "Volume Qos Policy Create", GetPosInfo());
}

bool
QosCreateVolumePolicyCommand::_HandleInputVolumes(json& doc)
{
    int validVol = -1;
    volumeNames.clear();
    validVolumes.clear();

    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }
    if (0 == arrayName.compare(""))
    {
        errorMsg = "Array Name Missing";
        return false;
    }
    for (unsigned int i = 0; i < doc["param"]["vol"].size(); i++)
    {
        string volName = doc["param"]["vol"][i]["volumeName"];
        volumeNames.push_back(volName);
    }
    IVolumeEventManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (nullptr == volMgr)
    {
        errorMsg = "Invalid Array Name";
        return false;
    }
    for (auto vol = volumeNames.begin(); vol != volumeNames.end(); vol++)
    {
        validVol = volMgr->CheckVolumeValidity(*vol);
        if (EID(SUCCESS) != validVol)
        {
            errorMsg = "Invalid Volume Name " + (*vol);
            return false;
        }
        else
        {
            IVolumeInfoManager* volInfoMgr =
                VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
            validVolumes.push_back(std::make_pair(*vol, volInfoMgr->GetVolumeID(*vol)));
        }
    }
    return true;
}
} // namespace pos_cli
