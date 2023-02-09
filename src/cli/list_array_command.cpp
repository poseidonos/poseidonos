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

#include "src/cli/list_array_command.h"

#include <algorithm>
#include <vector>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/sys_info/space_info.h"
#include "src/array_models/interface/i_array_info.h"

namespace pos_cli
{
ListArrayCommand::ListArrayCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ListArrayCommand::~ListArrayCommand(void)
{
}
// LCOV_EXCL_STOP

string
ListArrayCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    JsonElement data("data");

    JsonArray jsonArrayList("arrayList");
    vector<const ComponentsInfo*> infoList = ArrayMgr()->GetInfo();

    if (infoList.size() == 0)
    {
        return jFormat.MakeResponse("LISTARRAY", rid, EID(CLI_LIST_ARRAY_NO_ARRAY_EXISTS),
            "there is no array", data, GetPosInfo());
    }

    for (const ComponentsInfo* ci : infoList)
    {
        IArrayInfo* info = ci->arrayInfo;

        JsonElement arrayElement("");
        string arrayName(info->GetName());
        string createDatetime(info->GetCreateDatetime());
        string updateDatetime(info->GetUpdateDatetime());
        string arrayStatus("Unmounted");
        if (info->GetState() >= ArrayStateEnum::NORMAL)
        {
            arrayStatus = "Mounted";
        }
        arrayElement.SetAttribute(JsonAttribute("index", info->GetIndex()));
        arrayElement.SetAttribute(JsonAttribute("dataRaid", "\"" + info->GetDataRaidType() + "\""));
        arrayElement.SetAttribute(JsonAttribute("writeThroughEnabled", info->IsWriteThroughEnabled() ? "true" : "false"));
        arrayElement.SetAttribute(JsonAttribute("capacity", "\"" + to_string(SpaceInfo::TotalCapacity(info->GetIndex())) + "\""));
        arrayElement.SetAttribute(JsonAttribute("used", "\"" + to_string(SpaceInfo::Used(info->GetIndex())) + "\""));
        arrayElement.SetAttribute(JsonAttribute("name", "\"" + arrayName + "\""));
        arrayElement.SetAttribute(JsonAttribute("status", "\"" + arrayStatus + "\""));
        arrayElement.SetAttribute(JsonAttribute("createDatetime", "\"" + createDatetime + "\""));
        arrayElement.SetAttribute(JsonAttribute("updateDatetime", "\"" + updateDatetime + "\""));
        jsonArrayList.AddElement(arrayElement);
    }
    data.SetArray(jsonArrayList);
    return jFormat.MakeResponse("LISTARRAY", rid, SUCCESS,
        "list of array and its devices ", data, GetPosInfo());
}
}; // namespace pos_cli
