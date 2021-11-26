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

#include "src/master_context/default_configuration.h"

#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "src/include/pos_event_id.h"
#include "src/helper/json/json_helper.h"
#include "src/helper/file/directory.h"

namespace pos
{
DefaultConfiguration::DefaultConfiguration(void)
{
}

void
DefaultConfiguration::Restore(void)
{
    if (DirExists(ConfigurationDir()) == false)
    {
        MakeDir(ConfigurationDir());
    }
    string filePath = DefaultConfigurationFilePath();
    std::ofstream outfile(filePath.data());
    JsonElement defaultConfigElem("");

    for (auto& moduleIter : defaultConfig)
    {
        string moduleName = moduleIter.moduleName;
        JsonElement moduleElem(moduleName);

        for (auto& dataIter : moduleIter.keyAndValueList)
        {
            string key = dataIter.key;
            string value = dataIter.value;
            JsonAttribute attribute(key, value);
            moduleElem.SetAttribute(attribute);
        }
        defaultConfigElem.SetElement(moduleElem);
    }
    outfile <<
        defaultConfigElem.ToJson(JsonFormatType::JSON_FORMAT_TYPE_READABLE)
            << std::endl;
    outfile.close();
}

} // namespace pos
