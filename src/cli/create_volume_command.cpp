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

#include "src/cli/create_volume_command.h"

#include "src/cli/cli_event_code.h"
#include "src/qos/qos_manager.h"
#include "src/array_mgmt/array_manager.h"
#include "src/volume/volume_service.h"

namespace pos_cli
{
CreateVolumeCommand::CreateVolumeCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
CreateVolumeCommand::~CreateVolumeCommand(void)
{
}
// LCOV_EXCL_STOP

string
CreateVolumeCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    string qosMsg = "QoS parameters are set";

    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("name") && doc["param"].contains("size") &&
        doc["param"]["size"].is_number_unsigned() == true)
    {
        string volName = doc["param"]["name"].get<std::string>();
        uint64_t size = doc["param"]["size"].get<uint64_t>();
        uint64_t maxiops = 0;
        uint64_t maxbw = 0;

        if (doc["param"].contains("maxiops") &&
            doc["param"]["maxiops"].is_number_unsigned() == true)
        {
            maxiops = doc["param"]["maxiops"].get<uint64_t>();
        }
        if (doc["param"].contains("maxbw") &&
            doc["param"]["maxbw"].is_number_unsigned() == true)
        {
            maxbw = doc["param"]["maxbw"].get<uint64_t>();
        }

        int ret = FAIL;

        ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
        if (info == nullptr)
        {
            return jFormat.MakeResponse("CREATEVOLUME", rid, ret,
                "failed to create volume: " + volName, GetPosInfo());
        }
        IArrayInfo* array = info->arrayInfo;
        ArrayStateType arrayState = array->GetState();
        if (arrayState == ArrayStateEnum::BROKEN)
        {
            int eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
            POS_TRACE_WARN(eventId, "arrayName: {}, arrayState: {}",
                arrayName, arrayState.ToString());

            return jFormat.MakeResponse("CREATEVOLUME", rid, ret,
                "failed to create volume: " + volName, GetPosInfo());
        }

        if (false == QosManagerSingleton::Instance()->IsFeQosEnabled())
        {
            maxiops = 0;
            maxbw = 0;
            qosMsg = "Parameter setting skipped. Fe_qos is disabled.";
        }

        IVolumeManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

        if (volMgr != nullptr)
        {
            ret = volMgr->Create(volName, size, maxiops, maxbw);
        }

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("CREATEVOLUME", rid, ret,
                volName + " is created successfully." + qosMsg, GetPosInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "CREATEVOLUME", rid, ret,
                "failed to create " + volName + "(code:" + to_string(ret) + ")",
                GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "CREATEVOLUME", rid, BADREQUEST,
            "volume name and volume size are not entered or there is an error",
            GetPosInfo());
    }
}

}; // namespace pos_cli
