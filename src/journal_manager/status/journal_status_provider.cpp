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

#include "journal_status_provider.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "i_checkpoint_status.h"
#include "i_log_buffer_status.h"
#include "src/journal_manager/config/journal_configuration.h"

namespace pos
{
JournalStatusProvider::JournalStatusProvider(void)
: bufferStatus(nullptr),
  checkpointStatus(nullptr),
  config(nullptr)
{
    logGroupStatusMap[LogGroupStatus::INIT] = "INIT";
    logGroupStatusMap[LogGroupStatus::ACTIVE] = "ACTIVE";
    logGroupStatusMap[LogGroupStatus::INVALID] = "INVALID";
    logGroupStatusMap[LogGroupStatus::FULL] = "FULL";

    checkpointStatusMap[CheckpointStatus::INIT] = "INIT";
    checkpointStatusMap[CheckpointStatus::STARTED] = "STARTED";
    checkpointStatusMap[CheckpointStatus::WAITING_FOR_FLUSH_DONE] = "WAITING_FOR_FLUSH_DONE";
    checkpointStatusMap[CheckpointStatus::COMPLETED] = "COMPLETED";
}

JournalStatusProvider::~JournalStatusProvider(void)
{
}

void
JournalStatusProvider::Init(ILogBufferStatus* bufferStatusProvider, JournalConfiguration* journalConfig, ICheckpointStatus* checkpointStatusProvider)
{
    bufferStatus = bufferStatusProvider;
    checkpointStatus = checkpointStatusProvider;
    config = journalConfig;
}

bool
JournalStatusProvider::IsJournalEnabled(void)
{
    // TODO (huijeong.kim) to initialize module when journal disabled
    if (config == nullptr)
    {
        return false;
    }
    else
    {
        return config->IsEnabled();
    }
}

ElementList
JournalStatusProvider::GetJournalStatus(void)
{
    ElementList statusList;

    JsonElement configElement = _CreateConfigElement();
    JsonElement bufferElement = _CreateBuferStatusElement();
    JsonElement checkpointElement = _CreateCheckpointStatusElement();

    statusList.push_back(configElement);
    statusList.push_back(bufferElement);
    statusList.push_back(checkpointElement);

    return statusList;
}

JsonElement
JournalStatusProvider::_CreateConfigElement(void)
{
    JsonElement configElement("config");
    configElement.SetAttribute(JsonAttribute("numLogGroups", std::to_string(config->GetNumLogGroups())));

    return configElement;
}

JsonElement
JournalStatusProvider::_CreateBuferStatusElement(void)
{
    JsonElement bufferElement("logBufferStatus");
    bufferElement.SetAttribute(JsonAttribute("logBufferSizeInBytes", std::to_string(config->GetLogBufferSize())));

    JsonArray logGroupElement("logGroups");
    for (int groupId = 0; groupId < config->GetNumLogGroups(); groupId++)
    {
        JsonElement logInfo("");
        logInfo.SetAttribute(JsonAttribute("seqNum", std::to_string(bufferStatus->GetSequenceNumber(groupId))));
        logInfo.SetAttribute(JsonAttribute("status", "\"" + logGroupStatusMap[bufferStatus->GetBufferStatus(groupId)] + "\""));
        logGroupElement.AddElement(logInfo);
    }
    bufferElement.SetArray(logGroupElement);

    return bufferElement;
}

JsonElement
JournalStatusProvider::_CreateCheckpointStatusElement(void)
{
    JsonElement checkpointElement("checkpointStatus");
    checkpointElement.SetAttribute(JsonAttribute("flushingLogGroupId", std::to_string(checkpointStatus->GetFlushingLogGroupId())));
    JsonArray fullGroupElement("fullLogGroups");
    for (auto it : checkpointStatus->GetFullLogGroups())
    {
        JsonElement logInfo("");
        logInfo.SetAttribute(JsonAttribute("ID", std::to_string(it)));
        fullGroupElement.AddElement(logInfo);
    }
    checkpointElement.SetArray(fullGroupElement);

    checkpointElement.SetAttribute(JsonAttribute("status", "\"" + checkpointStatusMap[checkpointStatus->GetStatus()] + "\""));

    return checkpointElement;
}

} // namespace pos
