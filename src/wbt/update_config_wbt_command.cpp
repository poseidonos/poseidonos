/*
 *   BSD LICENSE
 *   Copyright (c) 2021 Samsung Electronics Corporation
 *   All rights reterved.
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
 *   "AS IS" AND ANY EXPretS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
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

#include "update_config_wbt_command.h"
#include "src/master_context/config_manager.h"
#include "src/logger/logger.h"
#include <algorithm>

namespace pos
{
UpdateConfigWbtCommand::UpdateConfigWbtCommand(void)
:   WbtCommand(UPDATE_CONFIG, "update_config")
{
}
// LCOV_EXCL_START
UpdateConfigWbtCommand::~UpdateConfigWbtCommand(void)
{
}
// LCOV_EXCL_STOP
int
UpdateConfigWbtCommand::Execute(Args &argv, JsonElement &elem)
{
    int ARGS_MISSING_ERROR = -2;
    int TYPE_ERROR = -3;
    int CONVERT_ERROR = -4;
    int ret = ARGS_MISSING_ERROR;
    string module = _GetParameter(argv, "module");
    if (module.empty())
    {
        return ret;
    }
    string key = _GetParameter(argv, "key");
    if (key.empty())
    {
        return ret;
    }
    string valueStr = _GetParameter(argv, "value");
    if (valueStr.empty())
    {
        return ret;
    }
    string typeStr = _GetParameter(argv, "type");
    if (typeStr.empty())
    {
        return ret;
    }

    std::transform(typeStr.begin(), typeStr.end(),typeStr.begin(), ::toupper);

    ret = TYPE_ERROR;
    int typeIdx = -1;
    for (int i = 0; i < (int)ConfigType::CONFIG_TYPE_COUNT; i++)
    {
        if (CONFIG_TYPE_STR[i] == typeStr)
        {
            typeIdx = i;
            break;
        }
    }

    if (typeIdx == -1)
    {
        return ret;
    }

    ConfigType type = (ConfigType)(typeIdx);
    void* value = nullptr;
    switch (type)
    {
        case ConfigType::CONFIG_TYPE_STRING:
        {
            value = &valueStr;
            ret = ConfigManagerSingleton::Instance()->_SetValue(module, key, value, type);
            break;
        }
        case ConfigType::CONFIG_TYPE_INT:
        {
            int val = stoi(valueStr);
            value = &val;
            ret = ConfigManagerSingleton::Instance()->_SetValue(module, key, value, type);
            break;
        }
        case ConfigType::CONFIG_TYPE_UINT32:
        {
            uint32_t val = stoul(valueStr, nullptr, 0);
            value = &val;
            ret = ConfigManagerSingleton::Instance()->_SetValue(module, key, value, type);
            break;
        }
        case ConfigType::CONFIG_TYPE_UINT64:
        {
            uint64_t val = stoull(valueStr, nullptr, 0);
            value = &val;
            ret = ConfigManagerSingleton::Instance()->_SetValue(module, key, value, type);
            break;
        }
        case ConfigType::CONFIG_TYPE_BOOL:
        {
            std::transform(valueStr.begin(), valueStr.end(),valueStr.begin(), ::tolower);
            bool val = true;
            if (valueStr == "false")
            {
                val = false;
            }
            value = &val;
            ret = ConfigManagerSingleton::Instance()->_SetValue(module, key, value, type);
            break;
        }
        default:
        {
            break;
        }
    }

    if (value == nullptr)
    {
        return CONVERT_ERROR;
    }
    if (ret == 0)
    {
        POS_TRACE_WARN(9999, "Config has been updated via WBT, {}-{}, value:{}", module, key, valueStr);
    }
    return ret;
}
} // namespace pos
