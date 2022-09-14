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

#include <yaml-cpp/yaml.h>

#include <string>

#include "src/include/pos_event_id.h"
#include "src/include/pos_event_id.hpp"
#include "src/logger/logger.h"

std::unordered_map<int, EventManager::EventInfoEntry> EventManager::EventInfo =
    EventManager::_LoadPosEvent();
std::unordered_map<std::string, int> EventManager::EventNameToIdMap =
    EventManager::_LoadEventNameToIdMap();

EventManager::EventManager()
{
}

EventManager::~EventManager()
{
}

int
EventManager::GetEventIdFromMap(std::string eventName)
{
    if (eventName == "SUCCESS")
    {
        return 0;
    }

    std::unordered_map<std::string, int>::const_iterator it =
        eventManager.EventNameToIdMap.find(eventName);

    if (it == eventManager.EventNameToIdMap.end())
    {
        return UNKNOWN_EVENT_ID;
    }

    return it->second;
}

std::unordered_map<int, EventManager::EventInfoEntry>
EventManager::_LoadPosEvent()
{
    std::unordered_map<int, EventInfoEntry> result;
    YAML::Node events;
    try
    {
        events = YAML::LoadFile(POS_EVENT_FILE_PATH)["Root"]["Event"];

        int id;
        std::string name, message, cause, solution;
        for (size_t i = 0; i < events.size(); ++i)
        {
            YAML::Node event = events[i];
            id = event["Id"].IsNull() ? -1 : event["Id"].as<int>();
            name = event["Name"].IsNull() ? "NONE" : event["Name"].as<std::string>();
            message = event["Message"].IsNull() ? "NONE" : event["Message"].as<std::string>();
            cause = event["Cause"].IsNull() ? "NONE" : event["Cause"].as<std::string>();
            solution = event["Solution"].IsNull() ? "NONE" : event["Solution"].as<std::string>();

            result.insert({id, EventInfoEntry{name, message, cause, solution}});
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return result;
}

std::unordered_map<std::string, int>
EventManager::_LoadEventNameToIdMap()
{
    std::unordered_map<std::string, int> result;

    for (auto& it : EventInfo)
    {
        int id = it.first;
        std::string name = it.second.GetEventName();

        result.insert(std::make_pair(name, id));
    }

    return result;
}

std::unordered_map<int, EventManager::EventInfoEntry>*
EventManager::GetEventInfo()
{
    return &EventInfo;
}