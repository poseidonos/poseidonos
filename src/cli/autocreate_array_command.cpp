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

#include "src/cli/autocreate_array_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/numa_awared_array_creation.h"
#include "src/array_mgmt/array_manager.h"
#include "src/device/device_manager.h"
#include "src/qos/qos_manager.h"

#include <vector>

namespace pos_cli
{
AutocreateArrayCommand::AutocreateArrayCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
AutocreateArrayCommand::~AutocreateArrayCommand(void)
{
}
// LCOV_EXCL_STOP

string AutocreateArrayCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;
    vector<string> buffers;
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
            buffers.push_back(name);
        }
    }

    int dataCnt = 0;
    if (doc["param"].contains("numData"))
    {
        dataCnt = doc["param"]["numData"].get<int>();
    }

    int spareCnt = 0;
    if (doc["param"].contains("numSpare"))
    {
        spareCnt = doc["param"]["numSpare"].get<int>();
    }

    NumaAwaredArrayCreation creationDelegate(buffers, dataCnt, spareCnt, DeviceManagerSingleton::Instance());
    NumaAwaredArrayCreationResult res = creationDelegate.GetResult();
    if (res.code != 0)
    {
        return jFormat.MakeResponse(
            "AUTOCREATEARRAY", rid, res.code,
                "failed to create " + arrayName, GetPosInfo());
    }

    IArrayMgmt* array = ArrayMgr();
    // TODO(SRM): interactive cli to select from multiple-options.
    int ret = array->Create(arrayName, res.options.front().devs, metaFt, dataFt);

    if (0 != ret)
    {
        return jFormat.MakeResponse(
            "AUTOCREATEARRAY", rid, ret,
                "failed to create " + arrayName, GetPosInfo());
    }
    else
    {
        QosManagerSingleton::Instance()->UpdateArrayMap(arrayName);
        POS_TRACE_INFO(EID(CLI_AUTOCREATE_ARRAY_SUCCESS), "");
        return jFormat.MakeResponse("AUTOCREATEARRAY", rid, SUCCESS,
            arrayName + " has been created successfully", GetPosInfo());
    }
}
}; // namespace pos_cli
