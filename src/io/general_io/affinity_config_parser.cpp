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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

#include "affinity_config_parser.h"

#include "src/include/ibof_event_id.hpp"
#include "src/logger/logger.h"
#include "src/master_context/config_manager.h"

namespace ibofos
{
const bool AffinityConfigParser::DEFAULT_IS_STRING_DESCRIPTED = true;

const CoreDescriptionArray AffinityConfigParser::DEFAULT_CORE_DESCRIPTIONS =
    {
        CoreDescription{CoreType::REACTOR, {1, 0}, "0"},
        CoreDescription{CoreType::UDD_IO_WORKER, {1, 0}, "1"},
        CoreDescription{CoreType::EVENT_SCHEDULER, {1, 0}, "2"},
        CoreDescription{CoreType::EVENT_WORKER, {3, 0}, "3-5"},
        CoreDescription{CoreType::GENERAL_USAGE, {1, 0}, "6"},
#if defined QOS_ENABLED_BE
        CoreDescription{CoreType::QOS, {1, 0}, "7"},
#endif
        CoreDescription{CoreType::META_SCHEDULER, {1, 0}, "8"},
        CoreDescription{CoreType::META_IO, {2, 0}, "9-10"},
};

const AffinityConfigParser::ConfigKeyAndCoreTypes
    AffinityConfigParser::CONFIG_KEY_AND_CORE_TYPES =
        {
            ConfigKeyAndCoreType{CoreType::REACTOR, "reactor"},
            ConfigKeyAndCoreType{CoreType::UDD_IO_WORKER, "udd_io_worker"},
            ConfigKeyAndCoreType{CoreType::EVENT_SCHEDULER, "event_scheduler"},
            ConfigKeyAndCoreType{CoreType::EVENT_WORKER, "event_worker"},
            ConfigKeyAndCoreType{CoreType::GENERAL_USAGE, "general_usage"},
#if defined QOS_ENABLED_BE
            ConfigKeyAndCoreType{CoreType::QOS, "qos"},
#endif
            ConfigKeyAndCoreType{CoreType::META_SCHEDULER, "meta_scheduler"},
            ConfigKeyAndCoreType{CoreType::META_IO, "meta_io"},
};

AffinityConfigParser::AffinityConfigParser(void)
: selectedDescs(DEFAULT_CORE_DESCRIPTIONS),
  isStringDescripted(DEFAULT_IS_STRING_DESCRIPTED)
{
    ConfigManager& configManager = *ConfigManagerSingleton::Instance();
    std::string module("affinity_manager");

    bool useConfig = false;
    int ret = configManager.GetValue(module, "use_config", &useConfig,
        CONFIG_TYPE_BOOL);
    bool configExist =
        (ret == static_cast<int>(IBOF_EVENT_ID::SUCCESS));

    if (configExist == false || (useConfig == false))
    {
        IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AFTMGR_NO_USE_CONFIG;
        IBOF_TRACE_INFO(static_cast<uint32_t>(eventId),
            IbofEventId::GetString(eventId));
        return;
    }

    IBOF_EVENT_ID eventId = IBOF_EVENT_ID::AFTMGR_USE_CONFIG;
    IBOF_TRACE_INFO(static_cast<uint32_t>(eventId),
        IbofEventId::GetString(eventId));

    CoreDescriptionArray parsedDescs{};

    for (auto& iter : CONFIG_KEY_AND_CORE_TYPES)
    {
        std::string key = iter.key;
        std::string coreRange;
        int ret = configManager.GetValue(module, key, &coreRange,
            CONFIG_TYPE_STRING);
        if (ret != static_cast<int>(IBOF_EVENT_ID::SUCCESS))
        {
            IBOF_TRACE_WARN(ret, "Core description from config is not valid. Use default description.");
            return;
        }

        CoreType type = iter.type;
        CoreDescription& targetDesc = parsedDescs[static_cast<uint32_t>(type)];
        targetDesc.coreRange = coreRange;
        targetDesc.type = type;
    }

    isStringDescripted = true;
    selectedDescs = parsedDescs;

    return;
}

AffinityConfigParser::~AffinityConfigParser(void)
{
}

const CoreDescriptionArray&
AffinityConfigParser::GetDescriptions(void)
{
    return selectedDescs;
}

bool
AffinityConfigParser::IsStringDescripted(void)
{
    return isStringDescripted;
}

} // namespace ibofos
