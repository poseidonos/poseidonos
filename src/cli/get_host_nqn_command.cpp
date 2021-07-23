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

#include "src/cli/get_host_nqn_command.h"

#include <vector>

#include "src/cli/cli_event_code.h"
#include "src/volume/volume_service.h"
#include "src/volume/volume_list.h"
#include "src/volume/volume_base.h"
#include "src/network/nvmf_target.h"

namespace pos_cli
{
GetHostNqnCommand::GetHostNqnCommand(void)
{
}

GetHostNqnCommand::~GetHostNqnCommand(void)
{
}

string
GetHostNqnCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("name"))
    {
        string volName = doc["param"]["name"].get<std::string>();

        IVolumeManager* volMgr =
            VolumeServiceSingleton::Instance()->GetVolumeManager(arrayName);
        VolumeList* volList = nullptr;
        VolumeBase* vol = nullptr;

        if (volMgr != nullptr)
        {
            volList = volMgr->GetVolumeList();
            vol = volList->GetVolume(volName);
        }

        if (vol == nullptr)
        {
            return jFormat.MakeResponse("GETHOSTNQN", rid, FAIL,
                "requested volume does not exist", GetPosInfo());
        }
        string subnqn = vol->GetSubnqn();
        if (subnqn.empty())
        {
            return jFormat.MakeResponse("GETHOSTNQN", rid, FAIL,
                volName + " does not have subnqn", GetPosInfo());
        }
        vector<string> list = NvmfTargetSingleton::Instance()->GetHostNqn(subnqn);
        if (list.size() == 0)
        {
            return jFormat.MakeResponse("GETHOSTNQN", rid, FAIL,
                "There is no hostnqn mapped to this volume: " + volName + \
                ". Connection to the POS Subsystem is required. ", GetPosInfo());
        }
        else
        {
            JsonElement data("data");
            JsonArray array("HostNqns");
            for (size_t i = 0; i < list.size(); i++)
            {
                JsonElement elem("");
                elem.SetAttribute(JsonAttribute("hostnqn", "\"" + list[i] + "\""));
                array.AddElement(elem);
            }
            data.SetArray(array);
            return jFormat.MakeResponse("GETHOSTNQN", rid, SUCCESS,
                "get host nqn of volume " + volName + " successfully", data, GetPosInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("GETHOSTNQN", rid, BADREQUEST,
            "volume name is not enterned", GetPosInfo());
    }
}
}; // namespace pos_cli
