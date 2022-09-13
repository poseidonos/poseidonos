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

#include "src/telemetry/telemetry_config/file_config_reader.h"

#include <string>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
FileConfigReader::FileConfigReader(void)
: fileName("")
{
    priority = ConfigPriority::Priority_3rd;
}

FileConfigReader::~FileConfigReader(void)
{
}

bool
FileConfigReader::Init(std::string fileName)
{
    this->fileName = fileName;

    YAML::Node config;

    try
    {
        config = YAML::LoadFile(fileName)[ROOT];

        for (auto it = config.begin(); it != config.end(); ++it)
        {
            std::string tag = it->first.as<std::string>();

            if (tag == "client")
            {
                client.Init(it->second);
            }
            else if (tag == "server")
            {
                server.Init(it->second);
            }
            else
            {
                POS_TRACE_ERROR(EID(TELEMETRY_CONFIG_INVALID_FIELD),
                    "Invalid config file: {}, invalid tag: {}", fileName, tag);
                return false;
            }
        }
    }
    catch (YAML::Exception& e)
    {
        POS_TRACE_ERROR(EID(TELEMETRY_CONFIG_BAD_FILE),
            "Invalid config file: {}", fileName);
        POS_TRACE_DEBUG(EID(TELEMETRY_DEBUG_MSG),
            "{}", e.msg);

        return false;
    }

    return true;
}
} // namespace pos
