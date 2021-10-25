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

#include "src/cli/smart_log_command.h"

#include <spdk/nvme_spec.h>

#include "src/cli/cli_event_code.h"
#include "src/device/device_manager.h"
#include "src/logger/logger.h"

namespace pos_cli
{
SMARTLOGCommand::SMARTLOGCommand(void)
{
}

// Exclude destructor of abstract class from function coverage report to avoid known issues in gcc/gcov
// LCOV_EXCL_START
SMARTLOGCommand::~SMARTLOGCommand(void)
{
}
// LCOV_EXCL_STOP

void
SMARTLOGCommand::_Complete(void* arg, const spdk_nvme_cpl* cpl)
{
    POS_TRACE_INFO(SUCCESS, "nvme admin completion done", SUCCESS);
}

void
SMARTLOGCommand::_PrintUint128Hex(uint64_t* v, char* s, size_t n)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        snprintf(s, n, "0x%llX%016llX", hi, lo);
    }
    else
    {
        snprintf(s, n, "0x%llX", lo);
    }
}

void
SMARTLOGCommand::_PrintUint128Dec(uint64_t* v, char* s, size_t n)
{
    unsigned long long lo = v[0], hi = v[1];
    if (hi)
    {
        _PrintUint128Hex(v, s, n);
    }
    else
    {
        snprintf(s, n, "%llu", (unsigned long long)lo);
    }
}

string
SMARTLOGCommand::Execute(json& doc, string rid)
{
    JsonFormat jFormat;

    if (doc["param"].contains("name"))
    {
        string deviceName = doc["param"]["name"].get<std::string>();
        struct spdk_nvme_ctrlr* ctrlr;
        struct spdk_nvme_health_information_page payload = {};

        ctrlr = pos::DeviceManagerSingleton::Instance()->GetNvmeCtrlr(deviceName);

        if (ctrlr == nullptr)
        {
            return jFormat.MakeResponse("SMARTLOG", rid, BADREQUEST, "Can't get nvme ctrlr", GetPosInfo());
        }

        if (spdk_nvme_ctrlr_cmd_get_log_page(ctrlr, SPDK_NVME_LOG_HEALTH_INFORMATION, SPDK_NVME_GLOBAL_NS_TAG, &payload, sizeof(payload), 0, &_Complete, NULL))
        {
            return jFormat.MakeResponse("SMARTLOG", rid, BADREQUEST, "Can't get log page", GetPosInfo());
        }

        int ret;

        while ((ret = spdk_nvme_ctrlr_process_admin_completions(ctrlr)) <= 0)
        {
            if (ret < 0)
                return jFormat.MakeResponse("SMARTLOG", rid, BADREQUEST, "Can't process completions", GetPosInfo());
        }

        JsonElement data("data");
        char cString[128];

        snprintf(cString, sizeof(cString), "%s", payload.critical_warning.bits.available_spare ? "WARNING" : "OK");
        string s1(cString);
        data.SetAttribute(JsonAttribute("available_spare_space", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%s", payload.critical_warning.bits.temperature ? "WARNING" : "OK");
        s1 = cString;
        data.SetAttribute(JsonAttribute("temperature", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%s", payload.critical_warning.bits.device_reliability ? "WARNING" : "OK");
        s1 = cString;
        data.SetAttribute(JsonAttribute("device_reliability", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%s", payload.critical_warning.bits.read_only ? "Yes" : "No");
        s1 = cString;
        data.SetAttribute(JsonAttribute("read_only", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%s", payload.critical_warning.bits.volatile_memory_backup ? "WARNING" : "OK");
        s1 = cString;
        data.SetAttribute(JsonAttribute("volatile_memory_backup", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%dC", (int)payload.temperature - 273);
        s1 = cString;
        data.SetAttribute(JsonAttribute("current_temperature", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%u%%", payload.available_spare);
        s1 = cString;
        data.SetAttribute(JsonAttribute("available_spare", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%u%%", payload.available_spare_threshold);
        s1 = cString;
        data.SetAttribute(JsonAttribute("available_spare_threshold", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%u%%", payload.percentage_used);
        s1 = cString;
        data.SetAttribute(JsonAttribute("life_percentage_used", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.data_units_read, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("data_units_read", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.data_units_written, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("data_units_written", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.host_read_commands, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("host_read_commands", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.host_write_commands, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("host_write_commands", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.controller_busy_time, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("controller_busy_time", "\"" + s1 + "m" + "\""));
        _PrintUint128Dec(payload.power_cycles, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("power_cycles", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.power_on_hours, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("power_on_hours", "\"" + s1 + "h" + "\""));
        _PrintUint128Dec(payload.unsafe_shutdowns, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("unsafe_shutdowns", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.media_errors, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("unrecoverable_media_errors", "\"" + s1 + "\""));
        _PrintUint128Dec(payload.num_error_info_log_entries, cString, sizeof(cString));
        s1 = cString;
        data.SetAttribute(JsonAttribute("lifetime_error_log_entries", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%um", payload.warning_temp_time);
        s1 = cString;
        data.SetAttribute(JsonAttribute("warning_temperature_time", "\"" + s1 + "\""));
        snprintf(cString, sizeof(cString), "%um", payload.critical_temp_time);
        s1 = cString;
        data.SetAttribute(JsonAttribute("critical_temperature_time", "\"" + s1 + "\""));
        for (int i = 0; i < 8; i++)
        {
            if (payload.temp_sensor[i] != 0)
            {
                snprintf(cString, sizeof(cString), "%dC", (int)payload.temp_sensor[i] - 273);
                s1 = cString;
                data.SetAttribute(JsonAttribute("temperature_sensor" + to_string(i + 1), "\"" + s1 + "\""));
            }
        }

        return jFormat.MakeResponse("SMARTLOG", rid, SUCCESS, "DONE", data, GetPosInfo());
    }
    else
    {
        return jFormat.MakeResponse("SMARTLOG", rid, BADREQUEST, "Check parameters", GetPosInfo());
    }
}
}; // namespace pos_cli
