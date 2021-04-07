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

#include "src/cli/cli_event_code.h"
#include "src/cli/request_handler.h"
#include "src/helper/json_helper.h"

namespace ibofos_cli
{
string
RequestHandler::CommandProcessing(char* msg)
{
    try
    {
        json jsonDoc = json::parse(msg);

        if (jsonDoc.contains("command") && jsonDoc.contains("rid"))
        {
            string command = jsonDoc["command"].get<std::string>();
            string rid = jsonDoc["rid"].get<std::string>();

            auto iter = cmdDictionary.find(command);
            if (iter != cmdDictionary.end())
            {
                if (command == "EXITSYSTEM")
                {
                    isExit = true;
                }

                HANDLER fp = iter->second;
                return (this->*fp)(jsonDoc, rid);
            }
            else
            {
                JsonFormat jFormat;
                return jFormat.MakeResponse(command, rid, BADREQUEST, "WRONG COMMAND");
            }
        }
        else
        {
            JsonFormat jFormat;
            return jFormat.MakeResponse("ERROR_RESPONSE", "UNKNOWN", BADREQUEST, "WRONG PARAMETER");
        }
    }
    catch (const std::exception& e)
    {
        JsonFormat jFormat;
        return jFormat.MakeResponse("ERROR_RESPONSE", "UNKNOWN", BADREQUEST, "WRONG FORMAT");
    }
}

string
RequestHandler::ExitSystem(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("EXITSYSTEM", rid, SUCCESS, "Exit System Done");
}

string
RequestHandler::ScanDevice(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("SCANDEVICE", rid, SUCCESS, "Scan Device Done");
}

string
RequestHandler::ListDevice(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse(
        "LISTDEVICE", rid, SUCCESS, "NO DEVICE EXIST");
}

string
RequestHandler::ListArrayDevice(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse(
        "LISTARRAYDEVICE", rid, SUCCESS, "NO ARRAY DEVICE EXIST");
}

string
RequestHandler::ArrayState(json& doc, string rid)
{
    JsonFormat jFormat;
    JsonElement data("data");
    data.SetAttribute(JsonAttribute("state", QUOT + "Unmounted" + QUOT));
    return jFormat.MakeResponse("ARRAYSTATE", rid, SUCCESS, "DONE", data);
}

string
RequestHandler::SystemState(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("SYSSTATE", rid, SUCCESS, "alive");
}

string
RequestHandler::AddDevice(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("ADDDEVICE", rid, SUCCESS, "pass");
}

string
RequestHandler::RemoveDevice(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("REMOVEDEVICE", rid, SUCCESS, "pass");
}

string
RequestHandler::StartDeviceMonitoring(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("STARTDEVICEMONITORING", rid, SUCCESS, "done");
}

string
RequestHandler::StopDeviceMonitoring(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("STOPDEVICEMONITORING", rid, SUCCESS, "done");
}

string
RequestHandler::DeviceMonitoringState(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse(
        "DEVICEMONITORINGSTATE", rid, SUCCESS, "NO DEVICE MONITOR EXIST");
}

string
RequestHandler::MountArray(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("MOUNTARRAY", rid, SUCCESS, "DONE");
}

string
RequestHandler::UnmountArray(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("UNMOUNTARRAY", rid, SUCCESS, "DONE");
}

string
RequestHandler::CreateVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("CREATEVOLUME", rid, SUCCESS, "DONE");
}

string
RequestHandler::DeleteVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("DELETEVOLUME", rid, SUCCESS, "DONE");
}

string
RequestHandler::MountVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("MOUNTVOLUME", rid, SUCCESS, "DONE");
}

string
RequestHandler::UnmountVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("UNMOUNTVOLUME", rid, SUCCESS, "DONE");
}

string
RequestHandler::ListVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse(
        "LISTVOLUME", rid, SUCCESS, "NO VOLUME EXIST");
}

string
RequestHandler::UpdateVolumeQoS(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("UPDATEVOLUMEQOS", rid, SUCCESS, "DONE");
}

string
RequestHandler::RenameVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("RENAMEVOLUME", rid, SUCCESS, "DONE");
}

string
RequestHandler::ResizeVolume(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("RESIZEVOLUME", rid, SUCCESS, "DONE");
}

string
RequestHandler::ListWBT(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("ListWBT", rid, SUCCESS, "DONE");
}

string
RequestHandler::WBT(json& doc, string rid)
{
    JsonFormat jFormat;
    return jFormat.MakeResponse("WBT ", rid, SUCCESS, "pass");
}
}; // namespace ibofos_cli
