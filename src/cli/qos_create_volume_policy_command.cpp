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
const uint32_t KIOPS = 1000;
const uint32_t MIB_IN_BYTE = 1024 * 1024;
const int64_t MIN_IOPS_LIMIT = 10;
const int64_t MIN_BW_LIMIT = 10;
const int64_t MAX_IOPS_LIMIT = INT64_MAX / KIOPS;
const int64_t MAX_BW_LIMIT = INT64_MAX / MIB_IN_BYTE;

QosCreateVolumePolicyCommand::QosCreateVolumePolicyCommand(void)
{
    minBw = 0;
    maxBw = 0;
    minIops = 0;
    maxIops = 0;
    arrayName = "";
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
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, SUCCESS, "QOS Settings Skipped", GetPosInfo());
    }
    if (doc["param"].contains("vol"))
    {
        validInput = _HandleInputVolumes(doc);
        if (false == validInput)
        {
            return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), errorMsg, GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "vol, Parameter Missing", GetPosInfo());
    }
    validInput = _VerifyMultiVolumeInput(doc);
    if (false == validInput)
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), errorMsg, GetPosInfo());
    }
    retVal = _HandleVolumePolicy(doc);
    if (SUCCESS != retVal)
    {
        return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, retVal, errorMsg, GetPosInfo());
    }
    return jFormat.MakeResponse("QOSCREATEVOLUMEPOLICY", rid, SUCCESS, "Volume Qos Policy Create", GetPosInfo());
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
    IVolumeManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
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
            validVolumes.push_back(std::make_pair(*vol, validVol));
        }
    }
    return true;
}

bool
QosCreateVolumePolicyCommand::_VerifyMultiVolumeInput(json& doc)
{
    int64_t minVal = 0;
    if (validVolumes.size() > 1)
    {
        if (doc["param"].contains("minbw"))
        {
            minVal = doc["param"]["minbw"].get<int64_t>();
            if (-1 != minVal)
            {
                errorMsg = "Multiple Volume Minimim Bw Not Supported";
                return false;
            }
        }
        if (doc["param"].contains("miniops"))
        {
            minVal = doc["param"]["miniops"].get<int64_t>();
            if (-1 != minVal)
            {
                errorMsg = "Multiple Volume Minimim Iops Not Supported";
                return false;
            }
        }
    }
    return true;
}

uint32_t
QosCreateVolumePolicyCommand::_HandleVolumePolicy(json& doc)
{
    int retVal = -1;
    IVolumeManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    for (auto vol = validVolumes.begin(); vol != validVolumes.end(); vol++)
    {
        std::pair<string, uint32_t> volume = (*vol);
        prevVolPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(volume.second);
        newVolPolicy = prevVolPolicy;
        newVolPolicy.policyChange = false;
        newVolPolicy.maxValueChanged = false;

        if (doc["param"].contains("minbw"))
        {
            minBw = doc["param"]["minbw"].get<int64_t>();
            if (-1 != minBw)
            {
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
        }

        if (doc["param"].contains("maxbw"))
        {
            maxBw = doc["param"]["maxbw"].get<int64_t>();
            if (-1 != maxBw)
            {
                if (0xFFFFFFFF == maxBw)
                {
                    maxBw = 0;
                }
                else if (maxBw < MIN_BW_LIMIT || maxBw > MAX_BW_LIMIT)
                {
                    errorMsg = "Max Bandwidth value outside allowed range";
                    return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
                }
                newVolPolicy.maxBw = maxBw;
                if (newVolPolicy.maxBw != prevVolPolicy.maxBw)
                {
                    newVolPolicy.policyChange = true;
                    newVolPolicy.maxValueChanged = true;
                }
            }
        }

        if (doc["param"].contains("miniops"))
        {
            minIops = doc["param"]["miniops"].get<int64_t>();
            if (-1 != minIops)
            {
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
        }

        if (doc["param"].contains("maxiops"))
        {
            maxIops = doc["param"]["maxiops"].get<int64_t>();
            if (-1 != maxIops)
            {
                if (0xFFFFFFFF == maxIops)
                {
                    maxIops = 0;
                }
                else if (maxIops < MIN_IOPS_LIMIT || maxIops > MAX_IOPS_LIMIT)
                {
                    errorMsg = "Max IOPS Value outside allowed range";
                    return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
                }
                newVolPolicy.maxIops = maxIops;
                if (newVolPolicy.maxIops != prevVolPolicy.maxIops)
                {
                    newVolPolicy.policyChange = true;
                    newVolPolicy.maxValueChanged = true;
                }
            }
        }

        if (true == newVolPolicy.policyChange)
        {
            if (true == newVolPolicy.minBwGuarantee && true == newVolPolicy.minIopsGuarantee)
            {
                errorMsg = "Either Min IOPS or Min BW Allowed";
                return QosReturnCode::MIN_IOPS_OR_MIN_BW_ONLY_ONE;
            }
            if (true == newVolPolicy.maxValueChanged)
            {
                retVal = volMgr->UpdateQoS(volume.first, newVolPolicy.maxIops, newVolPolicy.maxBw);
                if (retVal != SUCCESS)
                {
                    errorMsg = "QoS update in Volume Manager failed";
                    return retVal;
                }
            }
            retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(volume.second, newVolPolicy);
            if (retVal != SUCCESS)
            {
                errorMsg = "Qos Volume Policy Updated in QosManager failed";
                return retVal;
            }
        }
    }
    return SUCCESS;
}
} // namespace pos_cli
