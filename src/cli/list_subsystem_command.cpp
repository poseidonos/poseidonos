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

#include "src/cli/list_subsystem_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"
#include "src/network/nvmf_target.h"

namespace pos_cli
{
ListSubsystemCommand::ListSubsystemCommand(void)
{
    errorMessage = "Failed to list subsystem. ";
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ListSubsystemCommand::~ListSubsystemCommand(void)
{
}
// LCOV_EXCL_STOP

string
ListSubsystemCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    SpdkRpcClient rpcClient;
    NvmfTarget target;
    JsonElement data("data");
    string subnqn = "";
    bool getSubsystemInfo = false;
    string command = doc["command"].get<std::string>();

    if (command == "SUBSYSTEMINFO" && (doc["param"].contains("name") == true))
    {
        subnqn = doc["param"]["name"].get<std::string>();
        if (nullptr == target.FindSubsystem(subnqn))
        {
            POS_EVENT_ID result = EID(IONVMF_FAIL_TO_FIND_SUBSYSTEM);
            return jFormat.MakeResponse(command, rid, static_cast<int>(result),
                "Subsystem is not found", data, GetPosInfo());
        }
        getSubsystemInfo = true;
    }

    auto list = rpcClient.SubsystemList();

    JsonArray array("subsystemlist");
    for (const auto& subsystem : list)
    {
        if (true == getSubsystemInfo && (subnqn.compare(subsystem["nqn"].asString()) != 0))
        {
            continue;
        }
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("subnqn", "\"" + subsystem["nqn"].asString() + "\""));
        elem.SetAttribute(
            JsonAttribute("subtype", "\"" + subsystem["subtype"].asString() + "\""));
        elem.SetAttribute(
            JsonAttribute("allowAnyHost", to_string(subsystem["allow_any_host"].asInt())));

        JsonArray addressArray("listen_addresses");
        for (const auto& address : subsystem["listen_addresses"])
        {
            JsonElement elemAddr("");
            elemAddr.SetAttribute(
                JsonAttribute("transportType", "\"" + address["trtype"].asString() + "\""));
            elemAddr.SetAttribute(
                JsonAttribute("addressFamily", "\"" + address["adrfam"].asString() + "\""));
            elemAddr.SetAttribute(
                JsonAttribute("targetAddress", "\"" + address["traddr"].asString() + "\""));
            elemAddr.SetAttribute(
                JsonAttribute("transportServiceId", "\"" + address["trsvcid"].asString() + "\""));
            addressArray.AddElement(elemAddr);
        }
        elem.SetArray(addressArray);

        JsonArray hostArray("hosts");
        for (const auto& host : subsystem["hosts"])
        {
            JsonElement elemHost("");
            elemHost.SetAttribute(
                JsonAttribute("nqn", "\"" + host["nqn"].asString() + "\""));
            hostArray.AddElement(elemHost);
        }
        elem.SetArray(hostArray);

        if ("NVMe" == subsystem["subtype"].asString())
        {
            elem.SetAttribute(
                JsonAttribute("serialNumber", "\"" + subsystem["serial_number"].asString() + "\""));
            elem.SetAttribute(
                JsonAttribute("modelNumber", "\"" + subsystem["model_number"].asString() + "\""));
            elem.SetAttribute(
                JsonAttribute("maxNamespaces", to_string(subsystem["max_namespaces"].asInt())));

            JsonArray namespaceArray("namespaces");
            for (const auto& nameSpace : subsystem["namespaces"])
            {
                JsonElement elemNmsp("");
                elemNmsp.SetAttribute(
                    JsonAttribute("nsid", to_string(nameSpace["nsid"].asInt())));
                elemNmsp.SetAttribute(
                    JsonAttribute("bdevName", "\"" + nameSpace["bdev_name"].asString() + "\""));
                elemNmsp.SetAttribute(
                    JsonAttribute("uuid", "\"" + nameSpace["uuid"].asString() + "\""));
                namespaceArray.AddElement(elemNmsp);
            }
            elem.SetArray(namespaceArray);
        }
        array.AddElement(elem);
        if (true == getSubsystemInfo)
        {
            break;
        }
    }

    data.SetArray(array);

    return jFormat.MakeResponse(
        command, rid, SUCCESS,
        "list of existing subsystems", data, GetPosInfo());
}
}; // namespace pos_cli
