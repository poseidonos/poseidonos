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

#include "src/array_mgmt/array_manager.h"
#include "src/cli/array_info_command.h"
#include "src/cli/cli_event_code.h"
#include "src/mbr/mbr_info.h"
#include "src/sys_info/space_info.h"

namespace pos_cli
{
ArrayInfoCommand::ArrayInfoCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ArrayInfoCommand::~ArrayInfoCommand(void)
{
}
// LCOV_EXCL_STOP

string
ArrayInfoCommand::Execute(json& doc, string rid)
{
    string arrayName = "";
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    JsonFormat jFormat;
    if (arrayName == "")
    {   JsonFormat jFormat;
        int event = EID(CLI_ARRAY_INFO_NO_ARRAY_NAME);
        POS_TRACE_WARN(event, "");
        return jFormat.MakeResponse("ARRAYINFO", rid, event,
            "please type array name", GetPosInfo());
    }

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        return jFormat.MakeResponse("ARRAYINFO", rid, event,
            arrayName + " does not exist", GetPosInfo());
    }
    IArrayInfo* array = info->arrayInfo;
    IGCInfo* gc = info->gcInfo;

    JsonElement data("data");
    string state = array->GetStateCtx()->ToStateType().ToString();
    string situ = array->GetStateCtx()->GetSituation().ToString();
    data.SetAttribute(JsonAttribute("index", to_string(array->GetIndex())));
    data.SetAttribute(JsonAttribute("unique_id", to_string(array->GetUniqueId())));
    data.SetAttribute(JsonAttribute("name", "\"" + arrayName + "\""));
    data.SetAttribute(JsonAttribute("state", "\"" + state + "\""));
    data.SetAttribute(JsonAttribute("situation", "\"" + situ + "\""));
    data.SetAttribute(JsonAttribute("create_datetime", "\"" + array->GetCreateDatetime() + "\""));
    data.SetAttribute(JsonAttribute("update_datetime", "\"" + array->GetUpdateDatetime() + "\""));
    data.SetAttribute(JsonAttribute("rebuilding_progress", "\"" + to_string(array->GetRebuildingProgress()) + "\""));
    data.SetAttribute(JsonAttribute("capacity", to_string(SpaceInfo::SystemCapacity(arrayName))));
    data.SetAttribute(JsonAttribute("used", to_string(SpaceInfo::Used(arrayName))));
    data.SetAttribute(JsonAttribute("meta_raid", "\"" + array->GetMetaRaidType() + "\""));
    data.SetAttribute(JsonAttribute("data_raid", "\"" + array->GetDataRaidType() + "\""));

    if (array->GetState() >= ArrayStateEnum::NORMAL)
    {
        data.SetAttribute(JsonAttribute("gcMode", "\"" +
            _GetGCMode(gc, arrayName) + "\""));
    }

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

std::string
ArrayInfoCommand::_GetGCMode(IGCInfo* gc, string arrayName)
{
    if (arrayName == "")
    {
        return "N/A";
    }

    int isEnabled = gc->IsEnabled();
    if (0 != isEnabled)
    {
        return "N/A";
    }

    IContextManager* iContextManager = AllocatorServiceSingleton::Instance()->GetIContextManager(arrayName);
    SegmentCtx* segmentCtx = iContextManager->GetSegmentCtx();
    GcCtx* gcCtx = iContextManager->GetGcCtx();

    ComponentsInfo* info = ArrayMgr()->GetInfo(arrayName);
    if (info == nullptr)
    {
        int event = EID(CLI_ARRAY_INFO_ARRAY_NOT_EXIST);
        POS_TRACE_WARN(event, "array_name:{}", arrayName);
        return "N/A";
    }

    uint32_t arrayId = iContextManager->GetArrayId();
    segmentCtx->UpdateGcFreeSegment(arrayId);
    int numOfFreeSegments = segmentCtx->GetNumOfFreeSegment();
    GcMode gcMode = gcCtx->GetCurrentGcMode(numOfFreeSegments);

    std::string strGCMode;

    if (gcMode == GcMode::MODE_URGENT_GC)
    {
        strGCMode = "urgent";
    }
    else if (gcMode == GcMode::MODE_NORMAL_GC)
    {
        strGCMode = "normal";
    }
    else
    {
        strGCMode = "none";
    }

    return strGCMode;
}

}; // namespace pos_cli
