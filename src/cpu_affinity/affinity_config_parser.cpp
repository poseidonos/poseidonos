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

#include "affinity_config_parser.h"

#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

namespace pos
{
// If DEFAULT_IS_STRING_DESCRIPTED is true,
// string based expression for DEFAULT_CORE_DESCRIPTIONS will be used ("3-5"), ("0xff")
// Otherwise, core count for each num node (please refer CoreCountArray) will be configured to affinity.
// CoreCountArray will automatically assigns the cores in each node.

const bool AffinityConfigParser::DEFAULT_IS_STRING_DESCRIPTED = true;

const CoreDescriptionArray AffinityConfigParser::DEFAULT_CORE_DESCRIPTIONS =
    {
        CoreDescription{CoreType::REACTOR, {1, 0}, "0"},
        CoreDescription{CoreType::UDD_IO_WORKER, {1, 0}, "1"},
        CoreDescription{CoreType::EVENT_SCHEDULER, {1, 0}, "2"},
        CoreDescription{CoreType::EVENT_WORKER, {3, 0}, "3-5"},
        CoreDescription{CoreType::GENERAL_USAGE, {1, 0}, "6"},
        CoreDescription{CoreType::QOS, {1, 0}, "7"},
        CoreDescription{CoreType::META_SCHEDULER, {1, 0}, "8"},
        CoreDescription{CoreType::META_IO, {2, 0}, "9-10"},
        CoreDescription{CoreType::AIR, {1, 0}, "11"},
        CoreDescription{CoreType::EVENT_REACTOR, {3, 0}, "3-5"},
        CoreDescription{CoreType::DEBUG, {3, 0}, "6"},
};

const AffinityConfigParser::ConfigKeyAndCoreTypes
    AffinityConfigParser::CONFIG_KEY_AND_CORE_TYPES =
        {
            ConfigKeyAndCoreType{CoreType::REACTOR, "reactor"},
            ConfigKeyAndCoreType{CoreType::UDD_IO_WORKER, "udd_io_worker"},
            ConfigKeyAndCoreType{CoreType::EVENT_SCHEDULER, "event_scheduler"},
            ConfigKeyAndCoreType{CoreType::EVENT_WORKER, "event_worker"},
            ConfigKeyAndCoreType{CoreType::GENERAL_USAGE, "general_usage"},
            ConfigKeyAndCoreType{CoreType::QOS, "qos"},
            ConfigKeyAndCoreType{CoreType::META_SCHEDULER, "meta_scheduler"},
            ConfigKeyAndCoreType{CoreType::META_IO, "meta_io"},
            ConfigKeyAndCoreType{CoreType::AIR, "air"},
            ConfigKeyAndCoreType{CoreType::EVENT_REACTOR, "event_reactor"},
            ConfigKeyAndCoreType{CoreType::DEBUG, "debug"},
};

AffinityConfigParser::AffinityConfigParser(ConfigManager& configManager_)
: selectedDescs(DEFAULT_CORE_DESCRIPTIONS),
  isStringDescripted(DEFAULT_IS_STRING_DESCRIPTED),
  useEventReactor(false)
{
    ConfigManager& configManager = configManager_;
    std::string module("affinity_manager");

    bool useConfig = false;
    int ret = configManager.GetValue(module, "use_config", &useConfig,
        CONFIG_TYPE_BOOL);
    bool configExist =
        (ret == EID(SUCCESS));

    if (configExist == false || (useConfig == false))
    {
        POS_EVENT_ID eventId = EID(AFTMGR_NO_USE_CONFIG);
        POS_TRACE_INFO(static_cast<uint32_t>(eventId),
            "Use core description from default value");
        return;
    }

    POS_EVENT_ID eventId = EID(AFTMGR_USE_CONFIG);
    POS_TRACE_INFO(static_cast<uint32_t>(eventId),
        "Use core description from config file");

    CoreDescriptionArray parsedDescs{};
    bool useReactorConfig = false;
    ret = configManager.GetValue(module, "use_reactor_only", &useReactorConfig,
        CONFIG_TYPE_BOOL);
    configExist =
        (ret == EID(SUCCESS));
    if (configExist == false || useReactorConfig == false)
    {
        POS_EVENT_ID eventId = EID(AFTMGR_USE_CONFIG);
        POS_TRACE_INFO(static_cast<uint32_t>(eventId),
            "Use EventWorker and IOWorker");
    }
    else
    {
        useEventReactor = true;
        POS_EVENT_ID eventId = EID(AFTMGR_USE_CONFIG);
        POS_TRACE_INFO(static_cast<uint32_t>(eventId),
            "Use reactors for backend events and IOs.");
        POS_TRACE_INFO(static_cast<uint32_t>(eventId),
            "Event Reactor is not implemented yet, fallback to legacy code");
    }
    std::vector<bool> mandatoryAffinityVector;
    for (auto& iter : CONFIG_KEY_AND_CORE_TYPES)
    {
        bool useAffinity = true;
        if (useReactorConfig == true &&
            ((iter.type == CoreType::UDD_IO_WORKER)||
            (iter.type == CoreType::EVENT_SCHEDULER) ||
            (iter.type == CoreType::EVENT_WORKER)))
        {
            useAffinity = false;
        }
        if (useReactorConfig == false &&
            ((iter.type == CoreType::EVENT_REACTOR)))
        {
            useAffinity = false;
        }
        mandatoryAffinityVector.push_back(useAffinity);
    }
    for (auto& iter : CONFIG_KEY_AND_CORE_TYPES)
    {
        CoreType type = iter.type;
        CoreDescription& targetDesc = parsedDescs[static_cast<uint32_t>(type)];
        std::string key = iter.key;
        std::string coreRange = "";
        targetDesc.type = type;
        int ret = configManager.GetValue(module, key, &coreRange,
            CONFIG_TYPE_STRING);
        bool mandatory = (mandatoryAffinityVector.at(static_cast<uint32_t>(type)) == true);
        if (ret != EID(SUCCESS))
        {
            if (mandatory == true)
            {
                POS_TRACE_WARN(ret,
                    "Core description from config is not valid. Config \"{}\" is properly not set. Use default description.",
                    iter.key);
                return;
            }
            coreRange = "";
        }
        // If not necessary affinity is set by user in config file,
        // We print out warning message that affinity is not effective.
        if (ret == EID(SUCCESS))
        {
            if (mandatory == false)
            {
                POS_TRACE_WARN(ret, "We will ignore \"{}\" affinity config, use_reactor_only : {}",
                    iter.key, useReactorConfig);
                coreRange = "";
            }
        }
        targetDesc.coreRange = coreRange;
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

bool
AffinityConfigParser::UseEventReactor(void)
{
    return useEventReactor;
}

} // namespace pos
