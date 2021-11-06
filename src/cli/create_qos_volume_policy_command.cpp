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
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, QosReturnCode::FAILURE, "QOS Settings Skipped", GetPosInfo());
    }
    if (doc["param"].contains("vol"))
    {
        validInput = _HandleInputVolumes(doc);
        if (false == validInput)
        {
            return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), errorMsg, GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("CREATEQOSVOLUMEPOLICY", rid, static_cast<int>(POS_EVENT_ID::QOS_CLI_WRONG_MISSING_PARAMETER), "vol, Parameter Missing", GetPosInfo());
    }
    retVal = _HandleVolumePolicy(doc);
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

uint32_t
QosCreateVolumePolicyCommand::_HandleVolumePolicy(json& doc)
{
    int retVal = -1;
    IVolumeManager* volMgr = VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
    for (auto vol = validVolumes.begin(); vol != validVolumes.end(); vol++)
    {
        std::pair<string, uint32_t> volume = (*vol);
        prevVolPolicy = QosManagerSingleton::Instance()->GetVolumePolicy(volume.second, arrayName);
        newVolPolicy = prevVolPolicy;
        newVolPolicy.policyChange = false;
        newVolPolicy.maxValueChanged = false;

        if (doc["param"].contains("minbw"))
        {
            int64_t minBwFromCli = doc["param"]["minbw"].get<int64_t>();
            if (-1 != minBwFromCli)
            {
                if (minBwFromCli < 0 || ((0 != minBwFromCli) && ((uint64_t)minBwFromCli > MAX_BW_LIMIT)))
                {
                    errorMsg = "Min Bandwidth value outside allowed range";
                    return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
                }
                newVolPolicy.minBwGuarantee = true;
                newVolPolicy.minBw = minBwFromCli;
                if (newVolPolicy.minBw != prevVolPolicy.minBw)
                {
                    newVolPolicy.policyChange = true;
                }
            }
        }

        if (doc["param"].contains("maxbw"))
        {
            int64_t maxBwFromCli = doc["param"]["maxbw"].get<int64_t>();
            if (-1 != maxBwFromCli)
            {
                maxBw = maxBwFromCli;
                // value 0 means no throttling set
                if ((0 != maxBw) && (maxBw < MIN_BW_LIMIT || maxBw > MAX_BW_LIMIT))
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
            int64_t minIopsFromCli = doc["param"]["miniops"].get<int64_t>();
            if (-1 != minIopsFromCli)
            {
                if (minIopsFromCli < 0 || ((0 != minIopsFromCli) && ((uint64_t)minIopsFromCli > MAX_IOPS_LIMIT)))
                {
                    errorMsg = "Min Iops value outside allowed range";
                    return static_cast<int>(POS_EVENT_ID::OUT_OF_QOS_RANGE);
                }
                newVolPolicy.minIopsGuarantee = true;
                newVolPolicy.minIops = minIopsFromCli;
                if (newVolPolicy.minIops != prevVolPolicy.minIops)
                {
                    newVolPolicy.policyChange = true;
                }
            }
        }

        if (doc["param"].contains("maxiops"))
        {
            int64_t maxIopsFromCli = doc["param"]["maxiops"].get<int64_t>();
            if (-1 != maxIopsFromCli)
            {
                maxIops = maxIopsFromCli;
                // value 0 means no throttling set.
                if ((0 != maxIops) && (maxIops < MIN_IOPS_LIMIT || maxIops > MAX_IOPS_LIMIT))
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
            retVal = volMgr->UpdateQoS(volume.first, newVolPolicy.maxIops, newVolPolicy.maxBw, newVolPolicy.minIops, newVolPolicy.minBw);
            if (retVal != SUCCESS)
            {
                errorMsg = "QoS update in Volume Manager failed";
                return retVal;
            }
            uint32_t arrayId = QosManagerSingleton::Instance()->GetArrayIdFromMap(arrayName);
            retVal = QosManagerSingleton::Instance()->UpdateVolumePolicy(volume.second, newVolPolicy, arrayId);
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
