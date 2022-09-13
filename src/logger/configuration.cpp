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

#include "configuration.h"

#include <iostream>

#include "spdlog/spdlog.h"
#include "src/include/pos_event_id.h"
using namespace pos;
namespace pos_logger
{
uint32_t
Configuration::LogSizePerFileInMB()
{
    int SUCCESS = EID(SUCCESS);
    uint32_t size_mb = SIZE_MB;
    int ret = ConfigManagerSingleton::Instance()->GetValue("logger", "logfile_size_in_mb",
        &size_mb, ConfigType::CONFIG_TYPE_UINT32);
    if (ret == SUCCESS)
    {
        if (size_mb >= MIN_SIZE_PER_FILE_MB && size_mb <= MAX_SIZE_PER_FILE_MB)
        {
            return size_mb;
        }
    }
    return SIZE_MB;
}

uint32_t
Configuration::NumOfLogFilesForRotation()
{
    int SUCCESS = EID(SUCCESS);
    uint32_t rotation = ROTATION;
    int ret = ConfigManagerSingleton::Instance()->GetValue("logger", "logfile_rotation_count",
        &rotation, ConfigType::CONFIG_TYPE_UINT32);
    if (ret == SUCCESS)
    {
        if (rotation >= MIN_ROTATION && rotation <= MAX_ROTATION)
        {
            return rotation;
        }
    }
    return ROTATION;
}

string
Configuration::LogLevel()
{
    int SUCCESS = EID(SUCCESS);
    string loglvl = "";
    int ret = ConfigManagerSingleton::Instance()->GetValue("logger", "min_allowable_log_level",
        &loglvl, ConfigType::CONFIG_TYPE_STRING);
    if (ret == SUCCESS)
    {
        return loglvl;
    }
    return LOG_LEVEL;
}

bool
Configuration::IsStrLoggingEnabled()
{
    int SUCCESS = EID(SUCCESS);
    bool enable_structured_logging = false;
    int ret = ConfigManagerSingleton::Instance()->GetValue("logger", "enable_structured_logging",
        &enable_structured_logging, ConfigType::CONFIG_TYPE_BOOL);
    if (ret == SUCCESS)
    {
        return enable_structured_logging;
    }
    return ENABLE_STRUCTURED_LOGGING;
}
} // namespace pos_logger
