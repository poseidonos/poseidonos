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

#include "config_manager.h"

#include <fcntl.h>
#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "src/helper/json/json_helper.h"
#include "src/include/pos_event_id.h"

namespace pos
{
ConfigManager::ConfigManager(void)
{
}

int
ConfigManager::ReadFile(void)
{
    if (read == true)
    {
        return (int)POS_EVENT_ID::CONFIG_FILE_READ_DONE;
    }

    std::lock_guard<std::recursive_mutex> guard(configManagerMutex);

    string filePath = defaultConfig.ConfigurationFilePath();
    ifstream openFile;
    openFile.open(filePath.data());

    if (!openFile.is_open())
    {
        string defaultFilePath =
            defaultConfig.DefaultConfigurationFilePath();
        openFile.open(defaultFilePath.data());
        if (!openFile.is_open())
        {
            defaultConfig.Restore();
            openFile.open(defaultFilePath.data());
        }
    }

    std::stringstream strStream;

    strStream << openFile.rdbuf();
    string configData = strStream.str();
    openFile.close();

    rapidjson::ParseResult result = doc.Parse(configData.c_str());

    if (!result)
    {
        return (int)POS_EVENT_ID::CONFIG_FILE_FORMAT_ERROR;
    }

    read = true;
    return (int)POS_EVENT_ID::CONFIG_FILE_READ_DONE;
}

int
ConfigManager::GetValue(string module, string key,
    void* value, ConfigType type)
{
    if (read == false)
    {
        int ret = ReadFile();
        if ((int)POS_EVENT_ID::CONFIG_FILE_READ_DONE != ret)
        {
            return ret;
        }
    }

    if (!doc.IsObject())
    {
        return (int)POS_EVENT_ID::CONFIG_JSON_DOC_IS_NOT_OBJECT;
    }

    if (false == doc.HasMember(module.c_str()))
    {
        return (int)POS_EVENT_ID::CONFIG_REQUEST_MODULE_ERROR;
    }

    if (false == doc[module.c_str()].HasMember(key.c_str()))
    {
        return (int)POS_EVENT_ID::CONFIG_REQUEST_KEY_ERROR;
    }

    switch (type)
    {
        case CONFIG_TYPE_STRING:
            if (doc[module.c_str()][key.c_str()].IsString() == true)
            {
                *(string*)value = doc[module.c_str()][key.c_str()].GetString();
                return EID(SUCCESS);
            }
            return (int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR;

        case CONFIG_TYPE_INT:
            if (doc[module.c_str()][key.c_str()].IsInt() == true)
            {
                *(int*)value = doc[module.c_str()][key.c_str()].GetInt();
                return EID(SUCCESS);
            }
            return (int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR;

        case CONFIG_TYPE_UINT32:
            if (doc[module.c_str()][key.c_str()].IsUint() == true)
            {
                *(uint32_t*)value = doc[module.c_str()][key.c_str()].GetUint();
                return EID(SUCCESS);
            }
            return (int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR;

        case CONFIG_TYPE_UINT64:
            if (doc[module.c_str()][key.c_str()].IsUint64() == true)
            {
                *(uint64_t*)value = doc[module.c_str()][key.c_str()].GetUint64();
                return EID(SUCCESS);
            }
            return (int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR;

        case CONFIG_TYPE_BOOL:
            if (doc[module.c_str()][key.c_str()].IsBool() == true)
            {
                *(bool*)value = doc[module.c_str()][key.c_str()].GetBool();
                return EID(SUCCESS);
            }
            return (int)POS_EVENT_ID::CONFIG_VALUE_TYPE_ERROR;

        default:
            return (int)POS_EVENT_ID::CONFIG_REQUEST_CONFIG_TYPE_ERROR;
            break;
    }
}
} // namespace pos
