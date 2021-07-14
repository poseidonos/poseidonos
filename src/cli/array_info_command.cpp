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

#include "src/cli/array_info_command.h"

#include "src/array_mgmt/array_manager.h"
#include "src/cli/cli_event_code.h"
#include "src/mbr/mbr_info.h"
#include "src/sys_info/space_info.h"

namespace pos_cli
{
ArrayInfoCommand::ArrayInfoCommand(void)
{
}

ArrayInfoCommand::~ArrayInfoCommand(void)
{
}

string
ArrayInfoCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    JsonFormat jFormat;
    IArrayInfo* array = ArrayMgr::Instance()->GetArrayInfo(arrayName);
    if (array == nullptr)
    {
        int result = ArrayMgr::Instance()->AbrExists(arrayName);
        if (result)
        {
            return jFormat.MakeResponse("ARRAYINFO", rid, (int)POS_EVENT_ID::ARRAY_LOAD_FAIL,
                "Failed to load" + arrayName, GetPosInfo());
        }

        return jFormat.MakeResponse("ARRAYINFO", rid, (int)POS_EVENT_ID::ARRAY_WRONG_NAME,
            arrayName + " does not exist", GetPosInfo());
    }

    JsonElement data("data");
    string state = array->GetStateCtx()->ToStateType().ToString();
    string situ = array->GetStateCtx()->GetSituation().ToString();
    data.SetAttribute(JsonAttribute("index", to_string(array->GetIndex())));
    data.SetAttribute(JsonAttribute("name", "\"" + arrayName + "\""));
    data.SetAttribute(JsonAttribute("state", "\"" + state + "\""));
    data.SetAttribute(JsonAttribute("situation", "\"" + situ + "\""));
    data.SetAttribute(JsonAttribute("createDatetime", "\"" + array->GetCreateDatetime() + "\""));
    data.SetAttribute(JsonAttribute("updateDatetime", "\"" + array->GetUpdateDatetime() + "\""));
    data.SetAttribute(JsonAttribute("rebuildingProgress", "\"" + to_string(array->GetRebuildingProgress()) + "\""));
    data.SetAttribute(JsonAttribute("capacity", to_string(SpaceInfo::SystemCapacity(arrayName))));
    data.SetAttribute(JsonAttribute("used", to_string(SpaceInfo::Used(arrayName))));
    DeviceSet<string> nameSet = array->GetDevNames();

    if (nameSet.nvm.size() == 0 && nameSet.data.size() == 0)
    {
        return jFormat.MakeResponse("ARRAYINFO", rid, SUCCESS,
            arrayName + " information", data, GetPosInfo());
    }

    JsonArray jsonArray("devicelist");

    for (string name : nameSet.nvm)
    {
        JsonElement elem("");
        elem.SetAttribute(JsonAttribute("type", "\"BUFFER\""));
        elem.SetAttribute(JsonAttribute("name", "\"" + name + "\""));
        jsonArray.AddElement(elem);
    }
    for (string name : nameSet.data)
    {
        JsonElement elem("");
        elem.SetAttribute(JsonAttribute("type", "\"DATA\""));
        elem.SetAttribute(JsonAttribute("name", "\"" + name + "\""));
        jsonArray.AddElement(elem);
    }
    for (string name : nameSet.spares)
    {
        JsonElement elem("");
        elem.SetAttribute(JsonAttribute("type", "\"SPARE\""));
        elem.SetAttribute(JsonAttribute("name", "\"" + name + "\""));
        jsonArray.AddElement(elem);
    }

    data.SetArray(jsonArray);
    return jFormat.MakeResponse("ARRAYINFO", rid, SUCCESS,
        arrayName + " information", data, GetPosInfo());
}

}; // namespace pos_cli
