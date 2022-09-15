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

#include <rapidjson/document.h>

#include <string>
#include <vector>

#include "src/lib/singleton.h"
#include "src/master_context/default_configuration.h"

using namespace std;

namespace pos
{
enum ConfigType
{
    CONFIG_TYPE_START = 0,
    CONFIG_TYPE_STRING = CONFIG_TYPE_START,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_UINT32,
    CONFIG_TYPE_UINT64,
    CONFIG_TYPE_BOOL,
    CONFIG_TYPE_END,
    CONFIG_TYPE_COUNT = CONFIG_TYPE_END - CONFIG_TYPE_START
};

static const string CONFIG_TYPE_STR[(int)ConfigType::CONFIG_TYPE_COUNT]
{
    "STRING",
    "INT",
    "UINT32",
    "UINT64",
    "BOOL"
};

class ConfigManager
{
friend class UpdateConfigWbtCommand;

public:
    ConfigManager(void);
    virtual ~ConfigManager(void)
    {
    }

    virtual int ReadFile(void);
    virtual int GetValue(string module, string key, void* value, ConfigType type);
    string RawData() { return configData; }

private:
    //for wbt only
    virtual int _SetValue(string module, string key, void* value, ConfigType type);

    bool read = false;
    string configData = "";
    rapidjson::Document doc;
    DefaultConfiguration defaultConfig;
    std::recursive_mutex configManagerMutex;
};

using ConfigManagerSingleton = Singleton<ConfigManager>;

} // namespace pos
