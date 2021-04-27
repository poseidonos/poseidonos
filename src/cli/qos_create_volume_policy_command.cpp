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

#include "src/cli/qos_create_volume_policy_command.h"
#include "src/cli/cli_event_code.h"
#include "src/qos/qos_manager.h"
#include "src/volume/volume_manager.h"

namespace pos_cli
{
QosCreateVolumePolicyCommand::QosCreateVolumePolicyCommand(void)
{
    minBw = 0;
    maxBw = 0;
    minIops = 0;
    maxIops = 0;
}

QosCreateVolumePolicyCommand::~QosCreateVolumePolicyCommand(void)
{
}

string
QosCreateVolumePolicyCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    int retVal = -1;
    bool validInput = false;
    string ioType;
    if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, BADREQUEST, "FE QOS Disabled", GetPosInfo());
    }
    if (doc["param"].contains("level"))
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, BADREQUEST, "level, Invalid Parameter", GetPosInfo());
    }

    if (doc["param"].contains("vol"))
    {
        validInput = _HandleInputVolumes(doc);
        if (false == validInput)
        {
            return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, BADREQUEST, errorMsg, GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, BADREQUEST, "vol, Parameter Missing", GetPosInfo());
    }
    validInput = _VerifyMultiVolumeInput(doc);
    if (false == validInput)
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, BADREQUEST, errorMsg, GetPosInfo());
    }
    retVal = _HandleVolumePolicy(doc);
    if (SUCCESS != retVal)
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, retVal, "FAILED", GetPosInfo());
    }
    return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, SUCCESS, "Volume Qos Policy Create", GetPosInfo());
}

bool
QosCreateVolumePolicyCommand::_HandleInputVolumes(json& doc)
{
    int validVol = -1;
    string arrayName = DEFAULT_ARRAY_NAME;
    volumeNames.clear();
    volumeIds.clear();

    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }
    for (unsigned int i = 0; i < doc["param"]["vol"].size(); i++)
    {
        string volName = doc["param"]["vol"][i]["volumeName"];
        volumeNames.push_back(volName);
    }
    IVolumeManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    if (nullptr == volMgr)
    {
        errorMsg = "Invalid Array Name";
        return false;
    }
    for (auto vol = volumeNames.begin(); vol != volumeNames.end(); vol++)
    {
        validVol = volMgr->VolumeID(*vol);
        if (-1 == validVol)
        {
            errorMsg = "Invalid Volume Name " + (*vol);
            return false;
        }
        else
        {
            volumeIds.push_back(validVol);
        }
    }
    return true;
}

bool
QosCreateVolumePolicyCommand::_VerifyMultiVolumeInput(json& doc)
{
    if (volumeIds.size() > 1)
    {
        if (doc["param"].contains("minbw"))
        {
            errorMsg = "Multiple Volume Minimim Bw Not Supported";
            return false;
        }
        if (doc["param"].contains("miniops"))
        {
            errorMsg = "Multiple Volume Minimim Iops Not Supported";
            return false;
        }
    }
    return true;
}

uint32_t
QosCreateVolumePolicyCommand::_HandleVolumePolicy(json& doc)
{
    int retVal = -1;
    for (auto vol = volumeIds.begin(); vol != volumeIds.end(); vol++)
    {
        prevVolPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(*vol);
        newVolPolicy = prevVolPolicy;
        newVolPolicy.policyChange = false;
        newVolPolicy.maxValueChanged = false;

        if (doc["param"].contains("minbw"))
        {
            minBw = doc["param"]["minbw"].get<uint64_t>();
            if (0xFFFFFFFF == minBw)
            {
                newVolPolicy.minBwGuarantee = false;
                minBw = 0;
            }
            else
            {
                newVolPolicy.minBwGuarantee = true;
            }
            newVolPolicy.minBw = minBw;
            if (newVolPolicy.minBw != prevVolPolicy.minBw)
            {
                newVolPolicy.policyChange = true;
            }
        }

        if (doc["param"].contains("maxbw"))
        {
            maxBw = doc["param"]["maxbw"].get<uint64_t>();
            if (0xFFFFFFFF == maxBw)
            {
                maxBw = 0;
            }
            newVolPolicy.maxBw = maxBw;
            if (newVolPolicy.maxBw != prevVolPolicy.maxBw)
            {
                newVolPolicy.policyChange = true;
                newVolPolicy.maxValueChanged = true;
            }
        }

        if (doc["param"].contains("miniops"))
        {
            minIops = doc["param"]["miniops"].get<uint64_t>();
            if (0xFFFFFFFF == minIops)
            {
                newVolPolicy.minIopsGuarantee = false;
                minIops = 0;
            }
            else
            {
                newVolPolicy.minIopsGuarantee = true;
            }
            newVolPolicy.minIops = minIops;
            if (newVolPolicy.minIops != prevVolPolicy.minIops)
            {
                newVolPolicy.policyChange = true;
            }
        }

        if (doc["param"].contains("maxiops"))
        {
            maxIops = doc["param"]["maxiops"].get<uint64_t>();
            if (0xFFFFFFFF == maxIops)
            {
                maxIops = 0;
            }
            newVolPolicy.maxIops = maxIops;
            if (newVolPolicy.maxIops != prevVolPolicy.maxIops)
            {
                newVolPolicy.policyChange = true;
                newVolPolicy.maxValueChanged = true;
            }
        }

        if (true == newVolPolicy.policyChange)
        {
            retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(*vol, newVolPolicy);
            if (retVal != SUCCESS)
            {
                return retVal;
            }
        }
    }
    return SUCCESS;
}
} // namespace pos_cli
