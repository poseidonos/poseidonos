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

#include "src/cli/list_transport_command.h"

#include "src/cli/cli_event_code.h"
#include "src/helper/rpc/spdk_rpc_client.h"
#include "src/network/nvmf_target.h"
#include "src/logger/logger.h"

namespace pos_cli
{
ListTransportCommand::ListTransportCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
ListTransportCommand::~ListTransportCommand(void)
{
}
// LCOV_EXCL_STOP

string
ListTransportCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    SpdkRpcClient rpcClient;
    NvmfTarget target;
    JsonElement data("data");
    string errorMessage;

    auto list = rpcClient.TransportList();

    JsonArray array("transportlist");
    for (const auto& listener : list)
    {
        JsonElement elem("");
        elem.SetAttribute(
            JsonAttribute("type", "\"" + listener["type"].asString() + "\"")
        );
        elem.SetAttribute(
            JsonAttribute("max_queue_depth", "\"" + listener["max_queue_depth"].asString() + "\"")
        );

        array.AddElement(elem);
    }

    data.SetArray(array);

    int event = EID(CLI_LIST_TRANSPORT_SUCCESS);
    POS_TRACE_INFO(event, "");
    return jFormat.MakeResponse(
        "LISTTRANSPORT", rid, SUCCESS,
        "list of existing transports", data, GetPosInfo());
}
} // namespace pos_cli
