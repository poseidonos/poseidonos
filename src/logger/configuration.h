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

#pragma once

#include <string>

#include "src/master_context/config_manager.h"

using namespace std;
namespace pos_logger
{
class Configuration
{
public:
    uint32_t LogSizePerFileInMB();
    uint32_t NumOfLogFilesForRotation();
    string LogLevel();
    bool IsStrLoggingEnabled();
    bool IsBurstFilterEnabled();
    uint32_t GetBurstFilterWindowSize();

private:
    const uint32_t SIZE_MB = 50;
    const uint32_t MAX_SIZE_PER_FILE_MB = 2000;
    const uint32_t MIN_SIZE_PER_FILE_MB = 1;
    const uint32_t ROTATION = 20;
    const uint32_t MAX_ROTATION = 100;
    const uint32_t MIN_ROTATION = 1;
    const string LOG_LEVEL = "debug";
    // TODO (mj): The default value of STRUCTURED_LOGGING will be
    // TRUE after implementing structured logging functionality.
    const bool ENABLE_STRUCTURED_LOGGING = false;
    const bool ENABLE_BURST_FILTER = false;
};
} // namespace pos_logger
