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

#include "src/cli/volume_info_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/array_manager.h"
#include "src/volume/volume_service.h"
#include "src/volume/volume_base.h"

namespace pos_cli
{
VolumeInfoCommand::VolumeInfoCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
VolumeInfoCommand::~VolumeInfoCommand(void)
{
}
// LCOV_EXCL_STOP

string
VolumeInfoCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    string volumeName = "";

    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    if (doc["param"].contains("name") == true)
    {
        volumeName = doc["param"]["name"].get<std::string>();
    }

    JsonFormat jFormat;

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        POS_TRACE_WARN(EID(ARRAY_MGR_NO_ARRAY_MATCHING_REQ_NAME),
            "Failed to get a ComponentsInfo instance."
                " array name: " + arrayName);

        return jFormat.MakeResponse(
            "VOLUMEINFO", rid, EID(ARRAY_MGR_NO_ARRAY_MATCHING_REQ_NAME),
                "Array does not exist. array name: " + arrayName,
                GetPosInfo());
    }

    IVolumeInfoManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    if (volMgr == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND),
            "Failed to get an IVolumeInfoManager instance."
                " array name: " + arrayName +
                " volume name: "+ volumeName);

        return jFormat.MakeResponse(
            "VOLUMEINFO", rid, EID(VOL_NOT_FOUND),
                "Failed to find a volume because of an error."
                " array name: " + arrayName +
                " volume name: "+ volumeName,
                GetPosInfo());
    }

    VolumeBase* vol = volMgr->GetVolume(volMgr->GetVolumeID(volumeName));

    if (vol == nullptr)
    {
        return jFormat.MakeResponse(
            "VOLUMEINFO", rid, EID(VOL_NOT_FOUND),
                "No such volume exists in array. array name: " + arrayName,
                GetPosInfo());
    }

    JsonElement data("data");
    data.SetAttribute(JsonAttribute("name", "\"" + vol->GetName() + "\""));
    data.SetAttribute(JsonAttribute("uuid", "\"" + vol->GetUuid() + "\""));
    data.SetAttribute(JsonAttribute("total", to_string(vol->TotalSize())));

    VolumeStatus volumeStatus = vol->GetStatus();

    if (Mounted == volumeStatus)
    {
        data.SetAttribute(JsonAttribute("remain", to_string(vol->RemainingSize())));
    }

    data.SetAttribute(JsonAttribute("status", "\"" + volMgr->GetStatusStr(volumeStatus) + "\""));

    data.SetAttribute(JsonAttribute("maxiops", to_string(vol->MaxIOPS())));
    data.SetAttribute(JsonAttribute("maxbw", to_string(vol->MaxBW())));
    data.SetAttribute(JsonAttribute("minbw", to_string(vol->MinBW())));
    data.SetAttribute(JsonAttribute("miniops", to_string(vol->MinIOPS())));
    data.SetAttribute(JsonAttribute("subnqn", "\"" + vol->GetSubnqn() + "\""));
    data.SetAttribute(JsonAttribute("array_name", "\"" + vol->GetArrayName() + "\""));

    return jFormat.MakeResponse("VOLUMEINFO", rid, SUCCESS,
      "information of volume: " + volumeName + " of array: " + arrayName, data,
      GetPosInfo());
}
}; // namespace pos_cli
