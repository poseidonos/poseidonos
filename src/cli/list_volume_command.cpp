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

#include "src/cli/list_volume_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/array_manager.h"
#include "src/volume/volume_service.h"
#include "src/volume/volume_base.h"

namespace pos_cli
{
ListVolumeCommand::ListVolumeCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ListVolumeCommand::~ListVolumeCommand(void)
{
}
// LCOV_EXCL_STOP

string
ListVolumeCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;

    int ret = FAIL;
    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        return jFormat.MakeResponse("LISTVOLUME", rid, ret,
            "failed to list volume", GetPosInfo());
    }

    IArrayInfo* array = info->arrayInfo;

    ArrayStateType arrayState = array->GetState();
    if (arrayState == ArrayStateEnum::BROKEN)
    {
        int eventId = EID(CLI_COMMAND_FAILURE_ARRAY_BROKEN);
        POS_TRACE_WARN(eventId, "arrayName: {}, arrayState: {}",
            arrayName, arrayState.ToString());

        return jFormat.MakeResponse("LISTVOLUME", rid, ret,
             "failed to list volume", GetPosInfo());
    }

    IVolumeManager* volMgr =
        VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);

    int vol_cnt = 0;

    if (volMgr == nullptr)
    {
        POS_TRACE_WARN(EID(VOL_NOT_FOUND), "The requested volume does not exist");
    }
    else
    {
        vol_cnt = volMgr->GetVolumeCount();
    }

    if (vol_cnt > 0)
    {
        JsonElement data("data");
        JsonArray array("volumes");

        VolumeList* volList = volMgr->GetVolumeList();
        int idx = -1;
        while (true)
        {
            VolumeBase* vol = volList->Next(idx);
            if (nullptr == vol)
            {
                break;
            }

            JsonElement elem("");
            elem.SetAttribute(JsonAttribute("name", "\"" + vol->GetName() + "\""));
            elem.SetAttribute(JsonAttribute("id", to_string(idx)));
            elem.SetAttribute(JsonAttribute("total", to_string(vol->TotalSize())));

            VolumeStatus volumeStatus = vol->GetStatus();
            if (Mounted == volumeStatus)
            {
                elem.SetAttribute(JsonAttribute("remain", to_string(vol->RemainingSize())));
            }

            elem.SetAttribute(JsonAttribute("status", "\"" + volMgr->GetStatusStr(volumeStatus) + "\""));

            elem.SetAttribute(JsonAttribute("maxiops", to_string(vol->MaxIOPS())));
            elem.SetAttribute(JsonAttribute("maxbw", to_string(vol->MaxBW())));
            elem.SetAttribute(JsonAttribute("minbw", to_string(vol->MinBW())));
            elem.SetAttribute(JsonAttribute("miniops", to_string(vol->MinIOPS())));
            array.AddElement(elem);
        }

        data.SetArray(array);
        return jFormat.MakeResponse("LISTVOLUME", rid, SUCCESS,
            "list of volumes in " + arrayName, data,
            GetPosInfo());
    }
    else
    {
        return jFormat.MakeResponse(
            "LISTVOLUME", rid, SUCCESS,
            "no any volume exist in " + arrayName,
            GetPosInfo());
    }
}
}; // namespace pos_cli
