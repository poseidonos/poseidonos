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

#include "src/cli/add_device_command.h"

#include "src/cli/cli_event_code.h"
#include "src/array_mgmt/array_manager.h"

namespace pos_cli
{
AddDeviceCommand::AddDeviceCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
AddDeviceCommand::~AddDeviceCommand(void)
{
}
// LCOV_EXCL_STOP

string
AddDeviceCommand::Execute(json& doc, string rid)
{
    string arrayName = DEFAULT_ARRAY_NAME;
    if (doc["param"].contains("array") == true)
    {
        arrayName = doc["param"]["array"].get<std::string>();
    }

    JsonFormat jFormat;
    if (doc["param"].contains("spare"))
    {
        string devName = doc["param"]["spare"][0]["deviceName"];
        IArrayMgmt* array = ArrayMgr();
        int ret = array->AddDevice(arrayName, devName);
        if (ret == 0)
        {
            int event = EID(CLI_ADD_DEVICE_SUCCESS);
            POS_TRACE_INFO(event, "device_name:{}, array_name:{}", devName, arrayName);
            return jFormat.MakeResponse("ADDDEVICE", rid, event,
                devName + "has been added to " + arrayName + " successfully", GetPosInfo());
        }
        else
        {
            int event = EID(CLI_ADD_DEVICE_FAILURE);
            POS_TRACE_WARN(event, "");
            return jFormat.MakeResponse("ADDDEVICE", rid, event,
                "failed to add " + devName + " to " + arrayName +
                    "internal event:" + to_string(ret) + ")",
                GetPosInfo());
        }
    }
    else
    {
        int event = EID(CLI_ADD_DEVICE_FAILURE_NO_SPARE);
        POS_TRACE_WARN(event, "");
        return jFormat.MakeResponse("ADDDEVICE", rid, event,
            "failed to add spare device", GetPosInfo());
    }
}

}; // namespace pos_cli
