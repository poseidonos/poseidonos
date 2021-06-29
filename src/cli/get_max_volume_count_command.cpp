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

#include "src/cli/get_max_volume_count_command.h"

#include <vector>
#include <algorithm>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/mbr/mbr_info.h"
#include "src/volume/volume_list.h"

namespace pos_cli
{
GetMaxVolumeCountCommand::GetMaxVolumeCountCommand(void)
{
}

GetMaxVolumeCountCommand::~GetMaxVolumeCountCommand(void)
{
}

string
GetMaxVolumeCountCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    JsonElement data("data");

    std::vector<ArrayBootRecord> abrList;
    ArrayMgr::Instance()->GetAbrList(abrList);
    int arrayCnt = 0;

    if (!abrList.empty())
    {
        arrayCnt = abrList.size();
    }
    
    int totalvolcnt = arrayCnt * MAX_VOLUME_COUNT;

    data.SetAttribute(JsonAttribute("current array count", "\"" + to_string(arrayCnt) + "\""));
    data.SetAttribute(JsonAttribute("max volume count per Array", "\"" + to_string(MAX_VOLUME_COUNT) + "\""));
    data.SetAttribute(JsonAttribute("total max volume count", "\"" + to_string(totalvolcnt) + "\""));

    return jFormat.MakeResponse("GETMAXVOLUMECOUNT", rid, SUCCESS, "DONE", data,
        GetPosInfo());
}
}; // namespace pos_cli
