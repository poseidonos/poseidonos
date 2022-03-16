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

#include "src/cli/delete_array_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/array_manager.h"

namespace pos_cli
{
DeleteArrayCommand::DeleteArrayCommand(NvmfTarget* nvmfTarget)
: nvmfTarget(nvmfTarget)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
DeleteArrayCommand::~DeleteArrayCommand(void)
{
}
// LCOV_EXCL_STOP

string
DeleteArrayCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    IArrayMgmt* array = ArrayMgr();
    int ret = array->Delete(arrayName);
    JsonFormat jFormat;
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "DELETEARRAY", rid, ret, "failed to delete " + arrayName + "(code:" + to_string(ret) + ")", GetPosInfo());
    }
    bool deleteDone = nvmfTarget->DeletePosBdevAll(arrayName);
    if (false == deleteDone)
    {
        return jFormat.MakeResponse(
            "DELETEARRAY", rid, (int)POS_EVENT_ID::IONVMF_VOL_DELETE_TIMEOUT,
            "Some or every volumes in " + arrayName + " failed to delete.", GetPosInfo());
    }
    return jFormat.MakeResponse("DELETEARRAY", rid, SUCCESS,
        arrayName + " is deleted successfully", GetPosInfo());
}
}; // namespace pos_cli
