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

#include "src/cli/create_array_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/array_manager.h"

namespace pos_cli
{
CreateArrayCommand::CreateArrayCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
CreateArrayCommand::~CreateArrayCommand(void)
{
}
// LCOV_EXCL_STOP

string CreateArrayCommand::Execute(json& doc, string rid)
{
    int ret = 0;
    JsonFormat jFormat;
    DeviceSet<string> nameSet;
    string arrayName = DEFAULT_ARRAY_NAME;

    if (doc["param"].contains("name") == true)
    {
        arrayName = doc["param"]["name"].get<std::string>();
    }

    string dataFt = "RAID5";
    if (doc["param"].contains("raidtype") == true)
    {
        dataFt = doc["param"]["raidtype"].get<std::string>();
    }

    string metaFt = "RAID10";
    if (dataFt == "RAID0" || dataFt == "NONE")
    {
        metaFt = dataFt;
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

    IArrayMgmt* array = ArrayMgr();
    ret = array->Create(arrayName, nameSet, metaFt, dataFt);
    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "CREATEARRAY", rid, ret, "failed to create array " + arrayName, GetPosInfo());
    }
    else
    {
        return jFormat.MakeResponse("CREATEARRAY", rid, SUCCESS,
            arrayName + " has been created successfully", GetPosInfo());
    }
}
}; // namespace pos_cli
