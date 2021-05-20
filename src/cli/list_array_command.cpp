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

#include "src/cli/list_array_command.h"

#include <vector>
#include <algorithm>

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/helper/time_helper.h"
#include "src/mbr/mbr_info.h"
namespace pos_cli
{
ListArrayCommand::ListArrayCommand(void)
{
}

ListArrayCommand::~ListArrayCommand(void)
{
}

bool
abrCmp(ArrayBootRecord& a, ArrayBootRecord& b)
{
    time_t aTime = GetTimeT(a.createDatetime, "%Y-%m-%d %X %z");
    time_t bTime = GetTimeT(b.createDatetime, "%Y-%m-%d %X %z");
    return aTime < bTime;
}

string
ListArrayCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    JsonElement data("data");
    std::vector<ArrayBootRecord> abrList;
    int result = ArrayMgr::Instance()->GetAbrList(abrList);

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
            return jFormat.MakeResponse("LISTARRAY", rid, result,
                "List array failed", data, GetPosInfo());
        }
    }

    if (abrList.empty())
    {
        data.SetAttribute(JsonAttribute("arrayList", "\"There is no array\""));
    }
    else
    {
        JsonArray jsonArrayList("arrayList");
        sort(abrList.begin(), abrList.end(), abrCmp);
        for (const auto& abr : abrList)
        {
            JsonElement arrayElement("");
            string arrayName(abr.arrayName);
            string createDatetime(abr.createDatetime);
            string updateDatetime(abr.updateDatetime);
            string arrayStatus("Unmounted");
            IArrayInfo* info = ArrayMgr::Instance()->GetArrayInfo(arrayName);
            if (info != nullptr && info->GetState() >= ArrayStateEnum::NORMAL)
                arrayStatus = "Mounted";
            arrayElement.SetAttribute(JsonAttribute("name", "\"" + arrayName + "\""));
            arrayElement.SetAttribute(JsonAttribute("status", "\"" + arrayStatus + "\""));
            arrayElement.SetAttribute(JsonAttribute("createDatetime", "\"" + createDatetime + "\""));
            arrayElement.SetAttribute(JsonAttribute("updateDatetime", "\"" + updateDatetime + "\""));

            JsonArray jsonDeviceList("devicelist");

            for (unsigned int i = 0; i < abr.totalDevNum; i++)
            {
                JsonElement elem("");
                switch (abr.devInfo[i].deviceType)
                {
                    case (int)ArrayDeviceType::NVM:
                        elem.SetAttribute(JsonAttribute("type", "\"BUFFER\""));
                        break;
                    case (int)ArrayDeviceType::DATA:
                        elem.SetAttribute(JsonAttribute("type", "\"DATA\""));
                        break;
                    case (int)ArrayDeviceType::SPARE:
                        elem.SetAttribute(JsonAttribute("type", "\"SPARE\""));
                        break;
                }
                string deviceName(abr.devInfo[i].deviceUid);
                elem.SetAttribute(JsonAttribute("name", "\"" + deviceName + "\""));
                jsonDeviceList.AddElement(elem);
            }

            arrayElement.SetArray(jsonDeviceList);
            jsonArrayList.AddElement(arrayElement);
        }
        data.SetArray(jsonArrayList);
    }

    return jFormat.MakeResponse("LISTARRAY", rid, SUCCESS,
        "list of array and its devices ", data, GetPosInfo());
}
}; // namespace pos_cli
