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

#include "handle_wbt_command.h"

#include <list>
#include <utility>
#include <vector>

#include "cli_event_code.h"
#include "src/wbt/wbt_cmd_handler.h"

namespace pos_cli
{
HandleWbtCommand::HandleWbtCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
HandleWbtCommand::~HandleWbtCommand(void)
{
}
// LCOV_EXCL_STOP

std::string
HandleWbtCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    std::vector<pair<string, string>> dataAttr;

    string testname = doc["param"]["testname"].get<std::string>();
    json argv = doc["param"]["argv"];

    pos::WbtCmdHandler wbtCmdHandler(testname);

    int ret = 0;
    int64_t cmdRetValue;
    JsonElement retElem("data");
    string errMsg = "fail";

    if (wbtCmdHandler.VerifyWbtCommand())
    {
        cmdRetValue = wbtCmdHandler(argv, retElem);
    }
    else
    {
        ret = -1;
        errMsg = "invalid wbt command";
    }

    if (ret != 0)
    {
        return jFormat.MakeResponse("WBT " + testname, rid, ret, errMsg,
                GetPosInfo());
    }
    else
    {
        retElem.SetAttribute(JsonAttribute("returnCode", to_string(cmdRetValue)));

        return cmdRetValue == FAIL ? jFormat.MakeResponse("WBT " + testname, rid,
                FAIL, "FAIL", retElem, GetPosInfo()) : jFormat.MakeResponse("WBT " + testname,
                rid, SUCCESS, "PASS", retElem, GetPosInfo());
    }
}
}; // namespace pos_cli
