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

#include "src/cli/request_handler.h"

#include <asm/types.h>
#include <stdio.h>

#include <list>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

#include "cli_event_code.h"
#include "mfs.h"
#include "mk/ibof_config.h"
#include "spdk/nvme.h"
#include "spdk/nvme_spec.h"
#include "src/allocator/allocator.h"
#include "src/array/array.h"
#include "src/device/device_manager.h"
#include "src/device/ublock_device.h"
#include "src/gc/garbage_collector.h"
#include "src/journal_manager/journal_manager.h"
#include "src/logger/logger.h"
#include "src/mapper/mapper.h"
#include "src/master_context/mbr_manager.h"
#include "src/master_context/version_provider.h"
#include "src/network/nvmf_target.hpp"
#include "src/volume/volume_manager.h"
#include "src/wbt/wbt_cmd_handler.h"
#if defined QOS_ENABLED_BE
#include "src/qos/qos_common.h"
#include "src/qos/qos_manager.h"
#include "src/scheduler/event_scheduler.h"
#endif

using namespace ibofos;
using json = nlohmann::json;
namespace ibofos_cli
{
const char* DEFAULT_ARRAY_NAME = "POSArray";

string
RequestHandler::CommandProcessing(char* msg)
{
    try
    {
        json jsonDoc = json::parse(msg);
        cout << msg << endl;

        if (jsonDoc.contains("command") && jsonDoc.contains("rid"))
        {
            string command = jsonDoc["command"].get<std::string>();
            string rid = jsonDoc["rid"].get<std::string>();

            auto iter = cmdDictionary.find(command);
            if (iter != cmdDictionary.end())
            {
                HANDLER fp = iter->second;
                return (this->*fp)(jsonDoc, rid);
            }
            else
            {
                JsonFormat jFormat;
                return jFormat.MakeResponse(command, rid, BADREQUEST,
                    "wrong command", POSInfo());
            }
        }
        else
        {
            JsonFormat jFormat;
            return jFormat.MakeResponse("ERROR_RESPONSE", "UNKNOWN", BADREQUEST,
                "wrong parameter", POSInfo());
        }
    }
    catch (const std::exception& e)
    {
        JsonFormat jFormat;
        return jFormat.MakeResponse("EXCEPTION", "UNKNOWN", BADREQUEST,
            e.what(), POSInfo());
    }
}

string
RequestHandler::TimedOut(char* msg)
{
    json jsonDoc = json::parse(msg);
    string command = jsonDoc["command"].get<std::string>();
    string rid = jsonDoc["rid"].get<std::string>();

    JsonFormat jFormat;

    return jFormat.MakeResponse(command, rid, (int)IBOF_EVENT_ID::TIMED_OUT, "TIMED OUT",
        POSInfo());
}

string
RequestHandler::PosBusy(char* msg)
{
    json jsonDoc = json::parse(msg);
    string command = jsonDoc["command"].get<std::string>();
    string rid = jsonDoc["rid"].get<std::string>();

    JsonFormat jFormat;

    return jFormat.MakeResponse(command, rid, (int)IBOF_EVENT_ID::POS_BUSY, "POS IS BUSY",
        POSInfo());
}

JsonElement
RequestHandler::POSInfo(string name)
{
    JsonElement data(name);

    IbofosInfo info = mainHandler.GetInfo();

    data.SetAttribute(JsonAttribute("state", "\"" + info.state + "\""));
    data.SetAttribute(JsonAttribute("situation", "\"" + info.situation + "\""));
    data.SetAttribute(JsonAttribute("rebuildingProgress", "\"" + to_string(info.rebuildingProgress) + "\""));
    data.SetAttribute(JsonAttribute("capacity", to_string(info.totalCapacity)));
    data.SetAttribute(JsonAttribute("used", to_string(info.usedCapacity)));

    return data;
}

string
RequestHandler::CondSignal(json& doc, std::string rid)
{
    JsonFormat jFormat;
    if (isValidCondSignal)
    {
        isExit = true;
        return jFormat.MakeResponse("CONDSIGNAL", rid, SUCCESS,
            "signal cond_wait done", POSInfo());
    }
    else
    {
        return jFormat.MakeResponse("CONDSIGNAL", rid, BADREQUEST,
            "This command is not supported without --with-bdev-fio-plugin build option",
            POSInfo());
    }
}

string
RequestHandler::ExitIbofos(json& doc, string rid)
{
    JsonFormat jFormat;

    int ret = mainHandler.Exit();
    if (ret == 0)
    {
        if (!isExit)
        {
            isExit = true;
            return jFormat.MakeResponse("EXITIBOFOS", rid, SUCCESS,
                "ibofos will be terminated soon", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("EXITIBOFOS", rid, SUCCESS,
                "ibofos is now terminating", POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "EXITIBOFOS", rid, ret,
            "failed to terminate ibofos (code:" + to_string(ret) + ")",
            POSInfo());
    }
}

string
RequestHandler::ScanDevice(json& doc, string rid)
{
    DeviceManagerSingleton::Instance()->ScanDevs();
    MbrManagerSingleton::Instance()->Read();
    JsonFormat jFormat;
    return jFormat.MakeResponse("SCANDEVICE", rid, SUCCESS,
        "device scanning is complete", POSInfo());
}

string
RequestHandler::ListDevice(json& doc, string rid)
{
    JsonFormat jFormat;

    vector<DeviceProperty> list =
        DeviceManagerSingleton::Instance()->ListDevs();

    if (list.size() == 0)
    {
        return jFormat.MakeResponse(
            "LISTDEVICE", rid, SUCCESS, "no any device exists", POSInfo());
    }

    JsonArray array("devicelist");
    for (size_t i = 0; i < list.size(); i++)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("name", "\"" + list[i].name + "\""));
        elem.SetAttribute(
            JsonAttribute("size", to_string(list[i].size)));
        elem.SetAttribute(
            JsonAttribute("mn", "\"" + list[i].mn + "\""));
        elem.SetAttribute(
            JsonAttribute("sn", "\"" + list[i].sn + "\""));
        elem.SetAttribute(
            JsonAttribute("type", "\"" + list[i].GetType() + "\""));
        elem.SetAttribute(
            JsonAttribute("addr", "\"" + list[i].bdf + "\""));
        elem.SetAttribute(
            JsonAttribute("class", "\"" + list[i].GetClass() + "\""));
        array.AddElement(elem);
    }
    JsonElement data("data");
    data.SetArray(array);
    return jFormat.MakeResponse("LISTDEVICE", rid, SUCCESS, "list of existing devices", data,
        POSInfo());
}

string
RequestHandler::ListArrayDevice(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    JsonFormat jFormat;

    if (ArraySingleton::Instance()->ArrayExist(arrayName) == false)
    {
        return jFormat.MakeResponse("LISTARRAYDEVICE", rid, (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            arrayName + " does not exist", POSInfo());
    }

    DeviceSet<string> nameSet = ArraySingleton::Instance()->GetDevNames();

    if (nameSet.nvm.size() == 0 && nameSet.data.size() == 0)
    {
        return jFormat.MakeResponse(
            "LISTARRAYDEVICE", rid, SUCCESS, "no any device exists in " + arrayName,
            POSInfo());
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

    JsonElement data("data");
    data.SetArray(jsonArray);
    return jFormat.MakeResponse("LISTARRAYDEVICE", rid, SUCCESS,
        "list of existing devices in " + arrayName, data, POSInfo());
}

string
RequestHandler::GetIbofosInfo(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("GETIBOFOSINFO", rid, SUCCESS, "DONE",
        POSInfo("data"), POSInfo());
}

string
RequestHandler::AddDevice(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("spare"))
    {
        string devName = doc["param"]["spare"][0]["deviceName"];
        int ret = ArraySingleton::Instance()->AddSpare(devName, arrayName);
        if (ret == 0)
        {
            return jFormat.MakeResponse("ADDDEVICE", rid, SUCCESS,
                devName + "is added to " + arrayName + " successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("ADDDEVICE", rid, ret,
                "failed to add " + devName + " to " + arrayName +
                    "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("ADDDEVICE", rid, BADREQUEST,
            "only spare device can be added", POSInfo());
    }
}

string
RequestHandler::RemoveDevice(json& doc, string rid)
{
    JsonFormat jFormat;

    if (doc["param"].contains("spare"))
    {
        string devName = doc["param"]["spare"][0]["deviceName"];
        string arrayName = DEFAULT_ARRAY_NAME;
        if (doc["param"].contains("array") == true)
        {
            arrayName = doc["param"]["array"].get<std::string>();
        }

        int ret = ArraySingleton::Instance()->RemoveSpare(devName, arrayName);

        if (ret == 0)
        {
            return jFormat.MakeResponse("REMOVEDEVICE", rid, SUCCESS,
                devName + " is removed from " + arrayName + " successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("REMOVEDEVICE", rid, ret,
                "failed to remove " + devName + " from " + arrayName +
                    "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("REMOVEDEVICE", rid, BADREQUEST,
            "only spare device can be deleted", POSInfo());
    }
}

string
RequestHandler::StartDeviceMonitoring(json& doc, string rid)
{
    ibofos::DeviceManagerSingleton::Instance()->StartMonitoring();
    JsonFormat jFormat;
    return jFormat.MakeResponse("STARTDEVICEMONITORING", rid, SUCCESS, "done",
        POSInfo());
}

string
RequestHandler::StopDeviceMonitoring(json& doc, string rid)
{
    ibofos::DeviceManagerSingleton::Instance()->StopMonitoring();
    JsonFormat jFormat;
    return jFormat.MakeResponse("STOPDEVICEMONITORING", rid, SUCCESS, "done",
        POSInfo());
}

string
RequestHandler::DeviceMonitoringState(json& doc, string rid)
{
    vector<pair<string, string>> monitors;
    monitors = ibofos::DeviceManagerSingleton::Instance()->MonitoringState();
    JsonFormat jFormat;

    if (monitors.size() == 0)
    {
        return jFormat.MakeResponse(
            "DEVICEMONITORINGSTATE", rid, SUCCESS, "no any device monitor exists",
            POSInfo());
    }

    JsonArray array("monitorlist");
    for (size_t i = 0; i < monitors.size(); i++)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("name", "\"" + monitors[i].first + "\""));
        elem.SetAttribute(
            JsonAttribute("state", "\"" + monitors[i].second + "\""));
        array.AddElement(elem);
    }
    JsonElement data("data");
    data.SetArray(array);
    return jFormat.MakeResponse("DEVICEMONITORINGSTATE", rid, SUCCESS, "DONE",
        data, POSInfo());
}

std::string
RequestHandler::CreateArray(json& doc, std::string rid)
{
    int ret = 0;
    JsonFormat jFormat;
    DeviceSet<string> nameSet;
    string arrayName = DEFAULT_ARRAY_NAME;

    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    string raidType = "RAID5";
    if (doc["param"].contains("raidtype") == true)
    {
        raidType = doc["param"]["raidtype"].get<std::string>();
    }

    if (doc["param"].contains("buffer"))
    {
        for (unsigned int i = 0; i < doc["param"]["buffer"].size(); i++)
        {
            string name = doc["param"]["buffer"][i]["deviceName"];
            nameSet.nvm.push_back(name);
        }
    }

    if (doc["param"].contains("data"))
    {
        for (unsigned int i = 0; i < doc["param"]["data"].size(); i++)
        {
            string name = doc["param"]["data"][i]["deviceName"];
            nameSet.data.push_back(name);
        }
    }

    if (doc["param"].contains("spare"))
    {
        for (unsigned int i = 0; i < doc["param"]["spare"].size(); i++)
        {
            string name = doc["param"]["spare"][i]["deviceName"];
            nameSet.spares.push_back(name);
        }
    }

    ret = ArraySingleton::Instance()->Create(nameSet, arrayName, "RAID1", raidType);
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "CREATEARRAY", rid, ret, "failed to create " + arrayName + "(code:" + to_string(ret) + ")", POSInfo());
    }
    else
    {
        return jFormat.MakeResponse("CREATEARRAY", rid, SUCCESS,
            arrayName + " is created successfully", POSInfo());
    }
}

std::string
RequestHandler::LoadArray(json& doc, std::string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    int ret = ArraySingleton::Instance()->Load(arrayName);
    JsonFormat jFormat;
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "LOADARRAY", rid, ret, "failed to load " + arrayName + "(code:" + to_string(ret) + ")", POSInfo());
    }
    return jFormat.MakeResponse("LOADARRAY", rid, SUCCESS,
        arrayName + " is loaded successfully", POSInfo());
}

std::string
RequestHandler::DeleteArray(json& doc, std::string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    int ret = ArraySingleton::Instance()->Delete(arrayName);
    JsonFormat jFormat;
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "DELETEARRAY", rid, ret, "failed to delete " + arrayName + "(code:" + to_string(ret) + ")", POSInfo());
    }
    return jFormat.MakeResponse("DELETEARRAY", rid, SUCCESS,
        arrayName + " is deleted successfully", POSInfo());
}

std::string
RequestHandler::ArrayInfo(json& doc, std::string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    JsonFormat jFormat;
    JsonElement data("data");

    if (ArraySingleton::Instance()->ArrayExist(arrayName) == false)
    {
        return jFormat.MakeResponse("ARRAYINFO", rid, (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            arrayName + " does not exist", POSInfo());
    }

    string state = ArraySingleton::Instance()->GetCurrentStateStr();
    data.SetAttribute(JsonAttribute("state", "\"" + state + "\""));
    data.SetAttribute(JsonAttribute("name", "\"" + arrayName + "\""));

    DeviceSet<string> nameSet = ArraySingleton::Instance()->GetDevNames();

    if (nameSet.nvm.size() == 0 && nameSet.data.size() == 0)
    {
        return jFormat.MakeResponse("ARRAYINFO", rid, SUCCESS,
            arrayName + " information", data, POSInfo());
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
        arrayName + " information", data, POSInfo());
}

std::string
RequestHandler::MountIbofos(json& doc, std::string rid)
{
    JsonFormat jFormat;

    int ret = 0;
    ret = mainHandler.Mount();
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "MOUNTIBOFOS", rid, ret,
            "failed to mount ibofos (code:" + to_string(ret) + ")",
            POSInfo());
    }
    return jFormat.MakeResponse(
        "MOUNTIBOFOS", rid, SUCCESS,
        "ibofos is mounted successfully", POSInfo());
}

string
RequestHandler::UnmountIbofos(json& doc, string rid)
{
    JsonFormat jFormat;

    int ret = 0;
    ret = mainHandler.Unmount();
    if (ret != 0)
    {
        return jFormat.MakeResponse(
            "UNMOUNTIBOFOS", rid, ret,
            "failed to unmount ibofos (code:" + to_string(ret) + ")",
            POSInfo());
    }
    return jFormat.MakeResponse("UNMOUNTIBOFOS", rid, SUCCESS,
        "ibofos is unmounted successfully", POSInfo());
}

std::string
RequestHandler::MountArray(json& doc, std::string rid)
{
    JsonFormat jFormat;

    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    if (ArraySingleton::Instance()->GetArrayName() != arrayName)
    {
        return jFormat.MakeResponse(
            "MOUNTARRAY", rid, (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array does not exist named: " + arrayName,
            POSInfo());
    }

    int ret = mainHandler.Mount();
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "MOUNTARRAY", rid, ret,
            "failed to mount array (code:" + to_string(ret) + ")",
            POSInfo());
    }
    return jFormat.MakeResponse(
        "MOUNTARRAY", rid, SUCCESS,
        "array is mounted successfully", POSInfo());
}

string
RequestHandler::UnmountArray(json& doc, string rid)
{
    JsonFormat jFormat;

    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    if (ArraySingleton::Instance()->GetArrayName() != arrayName)
    {
        return jFormat.MakeResponse(
            "UNMOUNTARRAY", rid, (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array does not exist named: " + arrayName,
            POSInfo());
    }

    int ret = mainHandler.Unmount();
    if (ret != 0)
    {
        return jFormat.MakeResponse(
            "UNMOUNTARRAY", rid, ret,
            "failed to unmount array (code:" + to_string(ret) + ")",
            POSInfo());
    }
    return jFormat.MakeResponse("UNMOUNTARRAY", rid, SUCCESS,
        "Array is unmounted successfully", POSInfo());
}

string
RequestHandler::CreateVolume(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
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

        int ret = VolumeManagerSingleton::Instance()->Create(
            volName, size, arrayName, maxiops, maxbw);

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("CREATEVOLUME", rid, ret,
                volName + "is created successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "CREATEVOLUME", rid, ret,
                "failed to create " + volName + "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "CREATEVOLUME", rid, BADREQUEST,
            "volume name and volume size are not entered or there is an error",
            POSInfo());
    }
}

string
RequestHandler::DeleteVolume(json& doc, string rid)
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
        int ret = VolumeManagerSingleton::Instance()->Delete(volName, arrayName);

        if (ret == 0)
        {
            return jFormat.MakeResponse("DELETEVOLUME", rid, ret,
                volName + "is deleted successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "DELETEVOLUME", rid, ret,
                "failed to delete " + volName + "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "DELETEVOLUME", rid, BADREQUEST,
            "volume name is not entered", POSInfo());
    }
}

string
RequestHandler::MountVolume(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    string subnqn = "";
    if (doc["param"].contains("subnqn") == true)
    {
        subnqn = doc["param"]["subnqn"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("name"))
    {
        string volName = doc["param"]["name"].get<std::string>();
        int ret = VolumeManagerSingleton::Instance()->Mount(volName, arrayName, subnqn);
        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("MOUNTVOLUME", rid, ret,
                volName + "is mounted successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "MOUNTVOLUME", rid, ret,
                "failed to mount " + volName + "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "MOUNTVOLUME", rid, BADREQUEST,
            "volume name is not entered", POSInfo());
    }
}

string
RequestHandler::UnmountVolume(json& doc, string rid)
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
        int ret = VolumeManagerSingleton::Instance()->Unmount(volName, arrayName);
        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("UNMOUNTVOLUME", rid, ret,
                volName + "is unmounted successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "UNMOUNTVOLUME", rid, ret,
                "failed to unmount " + volName + "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("UNMOUNTVOLUME", rid, BADREQUEST,
            "volume name is not entered", POSInfo());
    }
}

string
RequestHandler::ListVolume(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (ArraySingleton::Instance()->GetArrayName() != arrayName)
    {
        return jFormat.MakeResponse(
            "LISTVOLUME", rid, (int)IBOF_EVENT_ID::ARRAY_WRONG_NAME,
            "array does not exist named: " + arrayName,
            POSInfo());
    }

    int vol_cnt = VolumeManagerSingleton::Instance()->GetVolumeCount();
    if (vol_cnt > 0)
    {
        JsonElement data("data");
        JsonArray array("volumes");

        VolumeList* volList =
            VolumeManagerSingleton::Instance()->GetVolumeList();
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
            elem.SetAttribute(JsonAttribute("status", "\"" + VolumeManagerSingleton::Instance()->GetStatusStr(volumeStatus) + "\""));

            elem.SetAttribute(JsonAttribute("maxiops", to_string(vol->MaxIOPS())));
            elem.SetAttribute(JsonAttribute("maxbw", to_string(vol->MaxBW())));
            array.AddElement(elem);
        }

        data.SetArray(array);
        return jFormat.MakeResponse("LISTVOLUME", rid, SUCCESS,
            "list of volumes in " + arrayName, data,
            POSInfo());
    }
    else
    {
        return jFormat.MakeResponse(
            "LISTVOLUME", rid, SUCCESS,
            "no any volume exist in " + arrayName,
            POSInfo());
    }
}

#if defined QOS_ENABLED_BE
ibofos::BackendEvent
RequestHandler::_GetEventId(string eventName)
{
    auto search = eventDict.find(eventName);
    if (search != eventDict.end())
    {
        return (search->second);
    }
    return (ibofos::BackendEvent_Unknown);
}

string
RequestHandler::UpdateEventWrrPolicy(json& doc, string rid)
{
    JsonFormat jFormat;
    int ret = 0;
    if (doc["param"].contains("name"))
    {
        string eventName = doc["param"]["name"].get<std::string>();
        uint64_t priority = 0;
        if (doc["param"].contains("prio"))
        {
            priority = doc["param"]["prio"].get<uint64_t>();
            if (priority > 2)
            {
                return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, BADREQUEST, "Wrong Priority", POSInfo());
            }
        }
        uint64_t weight = 0;
        if (doc["param"].contains("weight"))
        {
            weight = doc["param"]["weight"].get<uint64_t>();
            if ((1 > weight) || (weight > 3))
            {
                return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, BADREQUEST, "Wrong Weight", POSInfo());
            }
        }
        else
        {
            return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, BADREQUEST, "Wrong Weight", POSInfo());
        }

        BackendEvent event = _GetEventId(eventName);
        if (event == ibofos::BackendEvent_Unknown)
        {
            return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, BADREQUEST, "Wrong Event", POSInfo());
        }

        ret = QosManagerSingleton::Instance()->SetEventPolicy(event, static_cast<EventPriority>(priority), weight);

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, ret, "DONE", POSInfo());
        }
        else if (ret == QosReturnCode::VOLUME_POLICY_IN_EFFECT)
        {
            return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, ret, "Volume Min Policy In Effect", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, ret, "FAILED", POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("UPDATEEVENTWRRPOLICY", rid, BADREQUEST, "Check parameters", POSInfo());
    }
}

string
RequestHandler::ResetEventWrrPolicy(json& doc, string rid)
{
    JsonFormat jFormat;
    QosManagerSingleton::Instance()->ResetEventPolicy();
    return jFormat.MakeResponse("RESETEVENTWRRPOLICY", rid, SUCCESS, "DONE", POSInfo());
}
#endif

#if defined QOS_ENABLED_FE
string
RequestHandler::UpdateVolumeMinimumPolicy(json& doc, string rid)
{
    JsonFormat jFormat;
    if (doc["param"].contains("name"))
    {
        string volName = doc["param"]["name"].get<std::string>();
        int minBW = 0;
        int ioType = 0;
        int ret = 0;
        struct qos_vol_policy prevVolPolicy;
        struct qos_vol_policy newVolPolicy;

        prevVolPolicy = VolumeManagerSingleton::Instance()->GetVolumePolicy(volName);

        if (doc["param"].contains("iotype"))
        {
            ioType = doc["param"]["iotype"].get<uint64_t>();
            if (ioType >= QosWorkloadType_Mixed)
            {
                return jFormat.MakeResponse("UPDATEVOLUMEPOLICY", rid, BADREQUEST, "Wrong IO Type.Only 0,1 supported", POSInfo());
            }
            newVolPolicy.workLoad = static_cast<QosWorkloadType>(ioType);
        }
        else
        {
            newVolPolicy.workLoad = prevVolPolicy.workLoad;
        }

        if (doc["param"].contains("minbw"))
        {
            minBW = doc["param"]["minbw"].get<uint64_t>();
            newVolPolicy.minBW = minBW;
        }
        else
        {
            newVolPolicy.minBW = prevVolPolicy.minBW;
        }

        ret = VolumeManagerSingleton::Instance()->UpdateVolumePolicy(volName,
            newVolPolicy);
        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, ret, "DONE", POSInfo());
        }
        else if (ret == QosReturnCode::EVENT_POLICY_IN_EFFECT)
        {
            return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, ret, "Event WRR Policy In Effect", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, ret, "FAILED", POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("UPDATEVOLUMEMINPOLICY", rid, BADREQUEST, "Check parameters", POSInfo());
    }
}
#endif

string
RequestHandler::UpdateVolumeQoS(json& doc, string rid)
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

        int ret =
            VolumeManagerSingleton::Instance()->UpdateQoS(volName, arrayName,
                maxiops, maxbw);

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("UPDATEVOLUMEQOS", rid, ret,
                "QoS of " + volName + " updated successfully", POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("UPDATEVOLUMEQOS", rid, ret,
                "failed to update QoS of " + volName + "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "UPDATEVOLUMEQOS", rid, BADREQUEST,
            "volume name is not entered", POSInfo());
    }
}

string
RequestHandler::RenameVolume(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("name") && doc["param"].contains("newname"))
    {
        string oldName = doc["param"]["name"].get<std::string>();
        string newName = doc["param"]["newname"].get<std::string>();

        int ret =
            VolumeManagerSingleton::Instance()->Rename(oldName,
                newName, arrayName);

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("RENAMEVOLUME", rid, ret,
                oldName + " is renamed to " + newName,
                POSInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "RENAMEVOLUME", rid, ret,
                "failed to rename " + oldName + " to " + newName +
                    "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "RENAMEVOLUME", rid, (int)IBOF_EVENT_ID::INVALID_PARAM,
            "volume name or newname is not entered", POSInfo());
    }
}

string
RequestHandler::ResizeVolume(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("name") && doc["param"].contains("size"))
    {
        string volName = doc["param"]["name"].get<std::string>();
        uint64_t size = doc["param"]["size"].get<uint64_t>();

        int ret =
            VolumeManagerSingleton::Instance()->Resize(volName, arrayName, size);

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("RESIZEVOLUME", rid, ret,
                volName + "is resized to " + to_string(size), POSInfo());
        }
        else
        {
            return jFormat.MakeResponse(
                "RESIZEVOLUME", rid, ret,
                "failed to resize " + volName + "(code:" + to_string(ret) + ")",
                POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "RESIZEVOLUME", rid, BADREQUEST,
            "volume name is not entered", POSInfo());
    }
}

string
RequestHandler::GetMaxVolumeCount(json& doc, string rid)
{
    JsonFormat jFormat;
    JsonElement data("data");

    data.SetAttribute(JsonAttribute("count", "\"" + to_string(MAX_VOLUME_COUNT) + "\""));

    return jFormat.MakeResponse("GETMAXVOLUMECOUNT", rid, SUCCESS, "DONE", data,
        POSInfo());
}

string
RequestHandler::GetHostNQN(json& doc, string rid)
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
        VolumeList* volList =
            VolumeManagerSingleton::Instance()->GetVolumeList();
        VolumeBase* vol = volList->GetVolume(volName);
        if (vol == nullptr)
        {
            return jFormat.MakeResponse("GETHOSTNQN", rid, FAIL,
                "requested volume does not exist", POSInfo());
        }
        string subnqn = vol->GetSubnqn();
        if (subnqn.empty())
        {
            return jFormat.MakeResponse("GETHOSTNQN", rid, FAIL,
                volName + " does not have subnqn", POSInfo());
        }
        NvmfTarget nvmfTarget;
        vector<string> list = nvmfTarget.GetHostNqn(subnqn);
        if (list.size() == 0)
        {
            return jFormat.MakeResponse("GETHOSTNQN", rid, SUCCESS,
                "host nqn of " + volName + " does not exist", POSInfo());
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
                "get host nqn of volume " + volName + " successfully", data, POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse("GETHOSTNQN", rid, BADREQUEST,
            "volume name is not enterned", POSInfo());
    }
}

string
RequestHandler::StopRebuilding(json& doc, string rid)
{
    JsonFormat jFormat;
    ArraySingleton::Instance()->StopRebuilding();
    return jFormat.MakeResponse("STOPREBUILDING", rid, SUCCESS,
        "Rebuilding stopped", POSInfo());
}

string
RequestHandler::RebuildPerfImpact(json& doc, string rid)
{
    JsonFormat jFormat;

    if (doc["param"].contains("level"))
    {
        string level = doc["param"]["level"].get<std::string>();

        int ret = QosManagerSingleton::Instance()->SetEventPolicy("rebuild", level);

        return jFormat.MakeResponse("REBUILDPERFIMPACT", rid, ret,
            "rebuild perf impact is set.", POSInfo());
    }
    return jFormat.MakeResponse("REBUILDPERFIMPACT", rid, BADREQUEST,
        "wrong request", POSInfo());
}

string
RequestHandler::ApplyLogFilter(json& doc, string rid)
{
    JsonFormat jFormat;
    int ret = logger()->ApplyFilter();
    if (ret == SUCCESS)
    {
        return jFormat.MakeResponse("APPLYLOGFILTER", rid, SUCCESS,
            "filter is applied", POSInfo());
    }

    return jFormat.MakeResponse("APPLYLOGFILTER", rid, ret,
        "failed to apply filter(code:" + to_string(ret) + ")",
        POSInfo());
}

string
RequestHandler::SetLogLevel(json& doc, string rid)
{
    JsonFormat jFormat;
    if (doc["param"].contains("level"))
    {
        string level = doc["param"]["level"].get<std::string>();

        int ret = logger()->SetLevel(level);

        if (ret == SUCCESS)
        {
            return jFormat.MakeResponse("SETLOGLEVEL", rid, ret,
                "log level changed to " + level, POSInfo());
        }
        else
        {
            return jFormat.MakeResponse("SETLOGLEVEL", rid, ret,
                "failed to change log level to " + level, POSInfo());
        }
    }
    else
    {
        return jFormat.MakeResponse(
            "SETLOGLEVEL", rid, BADREQUEST,
            "level is not entered", POSInfo());
    }
}

string
RequestHandler::GetLogLevel(json& doc, string rid)
{
    string lvl = logger()->GetLevel();
    JsonFormat jFormat;
    JsonElement data("data");
    data.SetAttribute(JsonAttribute("level", "\"" + lvl + "\""));
    return jFormat.MakeResponse("GETLOGLEVEL", rid, SUCCESS, "current log level", data, POSInfo());
}

string
RequestHandler::LoggerInfo(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("LOGGERINFO", rid, SUCCESS,
        "ibofs logger info", logger()->GetPreference(), POSInfo());
}

void
SmartCompletion(void* arg, const struct spdk_nvme_cpl* cpl)
{
    IBOF_TRACE_INFO(SUCCESS, "nvme admin completion done", SUCCESS);
}

static void
print_uint128_hex(uint64_t* v, char* s)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        sprintf(s, "0x%llX%016llX", hi, lo);
    }
    else
    {
        sprintf(s, "0x%llX", lo);
    }
}

static void
print_uint128_dec(uint64_t* v, char* s)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        print_uint128_hex(v, s);
    }
    else
    {
        sprintf(s, "%llu", (unsigned long long)lo);
    }
}

string
RequestHandler::Smart(json& doc, string rid)
{
    JsonFormat jFormat;

    if (doc["param"].contains("name"))
    {
        string deviceName = doc["param"]["name"].get<std::string>();
        struct spdk_nvme_ctrlr* ctrlr;
        struct spdk_nvme_health_information_page payload = {};

        ctrlr = ibofos::DeviceManagerSingleton::Instance()->GetNvmeCtrlr(deviceName);

        if (ctrlr == nullptr)
        {
            return jFormat.MakeResponse("NVMEADMINCOMMAND", rid, BADREQUEST, "Can't get nvme ctrlr", POSInfo());
        }

        if (spdk_nvme_ctrlr_cmd_get_log_page(ctrlr, SPDK_NVME_LOG_HEALTH_INFORMATION, SPDK_NVME_GLOBAL_NS_TAG, &payload, sizeof(payload), 0, &SmartCompletion, NULL))
        {
            return jFormat.MakeResponse("NVMEADMINCOMMAND", rid, BADREQUEST, "Can't get log page", POSInfo());
        }

        int ret;

        while ((ret = spdk_nvme_ctrlr_process_admin_completions(ctrlr)) <= 0)
        {
            if (ret < 0)
                return jFormat.MakeResponse("NVMEADMINCOMMAND", rid, BADREQUEST, "Can't process completions", POSInfo());
        }

        JsonElement data("data");
        char cString[128];

        sprintf(cString, "%s", payload.critical_warning.bits.available_spare ? "WARNING" : "OK");
        string s1(cString);
        data.SetAttribute(JsonAttribute("availableSpareSpace", "\"" + s1 + "\""));
        sprintf(cString, "%s", payload.critical_warning.bits.temperature ? "WARNING" : "OK");
        s1 = cString;
        data.SetAttribute(JsonAttribute("temperature", "\"" + s1 + "\""));
        sprintf(cString, "%s", payload.critical_warning.bits.device_reliability ? "WARNING" : "OK");
        s1 = cString;
        data.SetAttribute(JsonAttribute("deviceReliability", "\"" + s1 + "\""));
        sprintf(cString, "%s", payload.critical_warning.bits.read_only ? "Yes" : "No");
        s1 = cString;
        data.SetAttribute(JsonAttribute("readOnly", "\"" + s1 + "\""));
        sprintf(cString, "%s", payload.critical_warning.bits.volatile_memory_backup ? "WARNING" : "OK");
        s1 = cString;
        data.SetAttribute(JsonAttribute("volatileMemoryBackup", "\"" + s1 + "\""));
        sprintf(cString, "%dC", (int)payload.temperature - 273);
        s1 = cString;
        data.SetAttribute(JsonAttribute("currentTemperature", "\"" + s1 + "\""));
        sprintf(cString, "%u%%", payload.available_spare);
        s1 = cString;
        data.SetAttribute(JsonAttribute("availableSpare", "\"" + s1 + "\""));
        sprintf(cString, "%u%%", payload.available_spare_threshold);
        s1 = cString;
        data.SetAttribute(JsonAttribute("availableSpareThreshold", "\"" + s1 + "\""));
        sprintf(cString, "%u%%", payload.percentage_used);
        s1 = cString;
        data.SetAttribute(JsonAttribute("lifePercentageUsed", "\"" + s1 + "\""));
        print_uint128_dec(payload.data_units_read, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("dataUnitsRead", "\"" + s1 + "\""));
        print_uint128_dec(payload.data_units_written, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("dataUnitsWritten", "\"" + s1 + "\""));
        print_uint128_dec(payload.host_read_commands, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("hostReadCommands", "\"" + s1 + "\""));
        print_uint128_dec(payload.host_write_commands, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("hostWriteCommands", "\"" + s1 + "\""));
        print_uint128_dec(payload.controller_busy_time, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("contollerBusyTime", "\"" + s1 + "m" + "\""));
        print_uint128_dec(payload.power_cycles, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("powerCycles", "\"" + s1 + "\""));
        print_uint128_dec(payload.power_on_hours, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("powerOnHours", "\"" + s1 + "h" + "\""));
        print_uint128_dec(payload.unsafe_shutdowns, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("unsafeShutdowns", "\"" + s1 + "\""));
        print_uint128_dec(payload.media_errors, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("unrecoverableMediaErrors", "\"" + s1 + "\""));
        print_uint128_dec(payload.num_error_info_log_entries, cString);
        s1 = cString;
        data.SetAttribute(JsonAttribute("lifetimeErrorLogEntries", "\"" + s1 + "\""));
        sprintf(cString, "%um", payload.warning_temp_time);
        s1 = cString;
        data.SetAttribute(JsonAttribute("warningTemperatureTime", "\"" + s1 + "\""));
        sprintf(cString, "%um", payload.critical_temp_time);
        s1 = cString;
        data.SetAttribute(JsonAttribute("criticalTemperatureTime", "\"" + s1 + "\""));
        for (int i = 0; i < 8; i++)
        {
            if (payload.temp_sensor[i] != 0)
            {
                sprintf(cString, "%dC", (int)payload.temp_sensor[i] - 273);
                s1 = cString;
                data.SetAttribute(JsonAttribute("temperatureSensor" + to_string(i + 1), "\"" + s1 + "\""));
            }
        }

        return jFormat.MakeResponse("NVMEADMINCOMMAND", rid, SUCCESS, "DONE", data, POSInfo());
    }
    else
    {
        return jFormat.MakeResponse("NVMEADMINCOMMAND", rid, BADREQUEST, "Check parameters", POSInfo());
    }
}

string
RequestHandler::GetVersion(json& doc, string rid)
{
    string version = VersionProviderSingleton::Instance()->GetVersion();
    JsonFormat jFormat;
    JsonElement data("data");
    data.SetAttribute(JsonAttribute("version", "\"" + version + "\""));
    return jFormat.MakeResponse("GETVERSION", rid, SUCCESS, "get version", data, POSInfo());
}

/*
RequestHandler::PassThroughNvmeAdminCommand(json &doc, string rid)
{
    int retError = -1;
    std::string deviceName("unvme-ns-1");
    struct spdk_nvme_cmd cmd;
    void *buffer = nullptr;
    uint32_t bufferSizeInBytes = 0;
    
    memset(&cmd, 0, sizeof(struct spdk_nvme_cmd));
    {
        // SMART / Health Information (Log Identifier 02h)
        uint32_t payloadSizeInBytes = 512;
        uint32_t numDwords = payloadSizeInBytes / sizeof(uint32_t) - 1u;
        uint32_t numDwordsLower = numDwords & 0xFFFFu;
        uint32_t numDwordsUpper = (numDwords >> 16) & 0xFFFFu;

        uint64_t logPageOffset = 0;
        uint32_t logPageOffsetLower = (uint32_t)logPageOffset;
        uint32_t logPageOffsetUpper = (uint32_t)(logPageOffset >> 32);

        uint8_t logPageId = SPDK_NVME_LOG_HEALTH_INFORMATION;

        cmd.opc = SPDK_NVME_OPC_GET_LOG_PAGE;
        cmd.nsid = SPDK_NVME_GLOBAL_NS_TAG;
        cmd.cdw10 = numDwordsLower << 16;
        cmd.cdw10 |= logPageId;
        cmd.cdw11 = numDwordsUpper;
        cmd.cdw12 = logPageOffsetLower;
        cmd.cdw13 = logPageOffsetUpper;

        bufferSizeInBytes = payloadSizeInBytes;
    }

    {
        const uint32_t BYTES_PER_ADMIN_BUFFER_UNIT = 256 * SZ_1KB;
        
        buffer = Memory<BYTES_PER_ADMIN_BUFFER_UNIT>::Alloc(
            DivideUp(bufferSizeInBytes, BYTES_PER_ADMIN_BUFFER_UNIT));
        if (nullptr != buffer)
        {
        
			retError = DeviceManager::Instance()->PassThroughNvmeAdminCommand(
                                deviceName, &cmd, buffer, bufferSizeInBytes);
			
			Memory<>::Free(buffer);
        }
    }

    JsonFormat jFormat;
    if(0 > retError)
    {
        return jFormat.MakeResponse("PASSTHROUGHNVMEADMIN", rid,
                    retError, to_string(retError), POSInfo());
    }
    else
    {
        return jFormat.MakeResponse("PASSTHROUGHNVMEADMIN", rid,
                    SUCCESS, "Done", POSInfo());
    }
}
*/

string
RequestHandler::ListWBT(json& doc, string rid)
{
    std::list<string> testlist;
    std::list<string>::iterator it;
    int ret = WbtCmdHandler("list_wbt").GetTestList(*&testlist);
    JsonFormat jFormat;

    if (ret >= 0)
    {
        JsonArray array("testlist");
        for (it = testlist.begin(); it != testlist.end(); it++)
        {
            JsonElement elem("");
            elem.SetAttribute(
                JsonAttribute("testName", "\"" + *it + "\""));
            array.AddElement(elem);
        }
        JsonElement data("data");
        data.SetArray(array);
        return jFormat.MakeResponse("LISTWBT", rid, SUCCESS, "DONE", data,
            POSInfo());
    }
    else
    {
        return jFormat.MakeResponse(
            "LISTWBT", rid, ret, to_string(ret), POSInfo());
    }
}

string
RequestHandler::WBT(json& doc, string rid)
{
    JsonFormat jFormat;
    std::vector<pair<string, string>> dataAttr;

    string testname = doc["param"]["testname"].get<std::string>();
    json argv = doc["param"]["argv"];

    WbtCmdHandler wbtCmdHandler(testname);

    int ret = 0;
    int64_t cmdRetValue;
    JsonElement retElem("data");
    string errMsg = "fail";

    if (wbtCmdHandler.IsValid())
    {
        cmdRetValue = wbtCmdHandler(argv, retElem);
    }
    else
    {
        ret = -1;
        errMsg = "invalid wbt command";
    }

    if (ret < 0)
    {
        return jFormat.MakeResponse("WBT " + testname, rid, ret, errMsg,
            POSInfo());
    }
    else
    {
        retElem.SetAttribute(JsonAttribute("returnCode", to_string(cmdRetValue)));

        return cmdRetValue == FAIL ? jFormat.MakeResponse("WBT " + testname, rid, FAIL, "FAIL", retElem, POSInfo()) : jFormat.MakeResponse("WBT " + testname, rid, SUCCESS, "PASS", retElem, POSInfo());
    }
}
}; // namespace ibofos_cli
