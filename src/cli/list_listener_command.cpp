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

#include "src/cli/list_listener_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/network/nvmf_target.h"
#include "src/logger/logger.h"

namespace pos_cli
{
ListListenerCommand::ListListenerCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ListListenerCommand::~ListListenerCommand(void)
{
}
// LCOV_EXCL_STOP

string
ListListenerCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    SpdkRpcClient rpcClient;
    NvmfTarget target;
    JsonElement data("data");
    string subnqn = doc["param"]["name"].get<string>();
    string errorMessage;

    if (nullptr == target.FindSubsystem(subnqn))
    {
        errorMessage = "Failed to list listener. Requested Subsystem does not exist or invalid subnqn. ";
        POS_EVENT_ID result = EID(LIST_LISTENER_FAILURE_NO_SUBNQN);
        return jFormat.MakeResponse(
            "LISTLISTENER", rid, static_cast<int>(result), errorMessage, data, GetPosInfo());
    }

    auto list = rpcClient.SubsystemListListener(subnqn);

    JsonArray array("listenerlist");
    for (const auto& listener : list)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("ana_state", "\"" + listener["ana_state"].asString() + "\""));

        array.AddElement(elem);
    }

    data.SetArray(array);

    int event = EID(CLI_LIST_LISTENER_SUCCESS);
    POS_TRACE_INFO(event, "");
    return jFormat.MakeResponse(
        "LISTLISTENER", rid, SUCCESS,
        "list of existing listeners", data, GetPosInfo());
}
} // namespace pos_cli
