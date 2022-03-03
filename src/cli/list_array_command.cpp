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
#include "src/mbr/mbr_info.h"
#include "src/sys_info/space_info.h"

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
    std::vector<ArrayBootRecord> abrList;
    int result = ArrayManagerSingleton::Instance()->GetAbrList(abrList);

    if (result != 0)
    {
        if (result == (int)POS_EVENT_ID::MBR_DATA_NOT_FOUND)
        {
            result = (int)POS_EVENT_ID::ARRAY_NOT_FOUND;
            return jFormat.MakeResponse("LISTARRAY", rid, result,
                "There is no array, All array data has been reset", data, GetPosInfo());
        }
        else
        {
            if (result == EID(DEVICEMGR_DEVICE_NOT_FOUND))
            {
                int event = EID(CLI_LIST_ARRAY_FAILURE_NO_DEVICE);
                POS_TRACE_WARN(event, "");
                return jFormat.MakeResponse("LISTARRAY", rid, event,
                    "failed to retrieve array list", data, GetPosInfo());
            }
        }
    }

    if (abrList.empty())
    {
        data.SetAttribute(JsonAttribute("arrayList", "\"There is no array\""));
    }
    else
    {
        JsonArray jsonArrayList("arrayList");
        for (const auto& abr : abrList)
        {
            JsonElement arrayElement("");
            string arrayName(abr.arrayName);
            string createDatetime(abr.createDatetime);
            string updateDatetime(abr.updateDatetime);
            string arrayStatus("Unmounted");

            ComponentsInfo* CompInfo = ArrayMgr()->GetInfo(arrayName);
            if (CompInfo == nullptr)
            {
                POS_TRACE_ERROR(EID(ARRAY_GET_COMPONENTS_FAILURE),
                    "Failed to list array"
                    " because of failing to get componentsInfo"
                    " with given array name. ArrayName: {}", arrayName);

                return jFormat.MakeResponse("LISTARRAY", rid,
                    EID(ARRAY_GET_COMPONENTS_FAILURE),
                    "Failed to list array", data, GetPosInfo());
            }
            IArrayInfo* info = CompInfo->arrayInfo;
            if (info == nullptr)
            {
                arrayStatus = "Fault";
                arrayElement.SetAttribute(JsonAttribute("index", ARRAY_ERROR_INDEX));
            }
            else
            {
                if (info->GetState() >= ArrayStateEnum::NORMAL)
                {
                    arrayStatus = "Mounted";
                }
                arrayElement.SetAttribute(JsonAttribute("index", info->GetIndex()));
                arrayElement.SetAttribute(JsonAttribute("data_raid", "\"" + info->GetDataRaidType() + "\""));
            }

            arrayElement.SetAttribute(JsonAttribute("name", "\"" + arrayName + "\""));
            arrayElement.SetAttribute(JsonAttribute("status", "\"" + arrayStatus + "\""));
            arrayElement.SetAttribute(JsonAttribute("create_datetime", "\"" + createDatetime + "\""));
            arrayElement.SetAttribute(JsonAttribute("update_datetime", "\"" + updateDatetime + "\""));
            arrayElement.SetAttribute(JsonAttribute("capacity", to_string(SpaceInfo::SystemCapacity(arrayName))));
            arrayElement.SetAttribute(JsonAttribute("used", to_string(SpaceInfo::Used(arrayName))));
            jsonArrayList.AddElement(arrayElement);
        }
        data.SetArray(jsonArrayList);
    }

    return jFormat.MakeResponse("LISTARRAY", rid, SUCCESS,
        "list of array and its devices ", data, GetPosInfo());
}
}; // namespace pos_cli
