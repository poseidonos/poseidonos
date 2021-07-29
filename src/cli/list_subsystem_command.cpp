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

#include "src/cli/list_subsystem_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/spdk_rpc_client.h"
#include "src/network/nvmf_target.h"

namespace pos_cli
{
ListSubsystemCommand::ListSubsystemCommand(void)
{
    errorMessage = "Failed to list subsystem. ";
}

ListSubsystemCommand::~ListSubsystemCommand(void)
{
}

string
ListSubsystemCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    SpdkRpcClient rpcClient;
    NvmfTarget target;

    auto list = rpcClient.SubsystemList();

    JsonArray array("subsystemlist");
    for (const auto& subsystem : list)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("nqn", "\"" + subsystem["nqn"].asString() + "\""));
        elem.SetAttribute(
            JsonAttribute("subtype", "\"" + subsystem["subtype"].asString() + "\""));
        elem.SetAttribute(
            JsonAttribute("allow_any_host", to_string(subsystem["allow_any_host"].asInt())));

        JsonArray addressArray("listen_addresses");
        for (const auto& address : subsystem["listen_addresses"])
        {
            JsonElement elemAddr("");
            elemAddr.SetAttribute(
                JsonAttribute("transport_type", "\"" + address["trtype"].asString() + "\""));
            elemAddr.SetAttribute(
                JsonAttribute("address_family", "\"" + address["adrfam"].asString() + "\""));
            elemAddr.SetAttribute(
                JsonAttribute("target_address", "\"" + address["traddr"].asString() + "\""));
            elemAddr.SetAttribute(
                JsonAttribute("transport_service_id", "\"" + address["trsvcid"].asString() + "\""));
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
                JsonAttribute("serial_number", "\"" + subsystem["serial_number"].asString() + "\""));
            elem.SetAttribute(
                JsonAttribute("model_number", "\"" + subsystem["model_number"].asString() + "\""));
            elem.SetAttribute(
                JsonAttribute("max_namespaces", to_string(subsystem["max_namespaces"].asInt())));

            JsonArray namespaceArray("namespaces");
            for (const auto& nameSpace : subsystem["namespaces"])
            {
                JsonElement elemNmsp("");
                elemNmsp.SetAttribute(
                    JsonAttribute("nsid", to_string(nameSpace["nsid"].asInt())));
                elemNmsp.SetAttribute(
                    JsonAttribute("bdev_name", "\"" + nameSpace["bdev_name"].asString() + "\""));
                elemNmsp.SetAttribute(
                    JsonAttribute("uuid", "\"" + nameSpace["uuid"].asString() + "\""));
                namespaceArray.AddElement(elemNmsp);
            }
            elem.SetArray(namespaceArray);
        }
        array.AddElement(elem);
    }

    JsonElement data("data");
    data.SetArray(array);
    if (doc["param"].contains("name") && nullptr != target.FindSubsystem(doc["param"]["name"].get<string>()))
    {
        data.SetAttribute(
            JsonAttribute("target_subnqn", "\"" + doc["param"]["name"].get<string>() + "\""));
    }

    return jFormat.MakeResponse(
        "LISTSUBSYSTEM", rid, SUCCESS,
        "list of existing subsystems", data, GetPosInfo());
}
}; // namespace pos_cli
