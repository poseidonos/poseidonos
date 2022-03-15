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

#include "src/cli/mount_array_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/array_manager.h"
#include "src/qos/qos_manager.h"

namespace pos_cli
{
MountArrayCommand::MountArrayCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
MountArrayCommand::~MountArrayCommand(void)
{
}
// LCOV_EXCL_STOP

string
MountArrayCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    bool isWTenabled = false;
    if (doc["param"].contains("enable_write_through") == true)
    {
        isWTenabled = doc["param"]["enable_write_through"].get<bool>();
    }

    JsonFormat jFormat;

    IArrayMgmt* array = ArrayMgr();
    int ret = array->Mount(arrayName, isWTenabled);
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "MOUNTARRAY", rid, ret,
            "failed to mount " + arrayName + "(code:" + to_string(ret) + ")",
            GetPosInfo());
    }
    QosManagerSingleton::Instance()->UpdateArrayMap(arrayName);
    return jFormat.MakeResponse(
        "MOUNTARRAY", rid, SUCCESS,
        arrayName + " is mounted successfully", GetPosInfo());
}
}; // namespace pos_cli
