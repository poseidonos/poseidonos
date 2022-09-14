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

#ifndef __EVENTM_MANAGER_H__
#define __EVENTM_MANAGER_H__

#include <string>
#include <unordered_map>

typedef int POS_EVENT_ID;

class EventManager
{
public:
    EventManager();
    ~EventManager();

    class EventInfoEntry
    {
    public:
        EventInfoEntry(std::string eventName, std::string message,
            std::string cause, std::string solution)
        {
            this->eventName = eventName;
            this->message = message;
            this->cause = cause;
            this->solution = solution;
        }
        std::string GetEventName()
        {
            return eventName;
        }
        std::string GetMessage()
        {
            return message;
        }
        std::string GetCause()
        {
            return cause;
        }
        std::string GetSolution()
        {
            return solution;
        }

    private:
        std::string eventName = "";
        std::string message = "";
        // mj: Fill in cause and solution for erroneous events only.
        std::string cause = "";
        std::string solution = "";
    };
    std::unordered_map<int, EventInfoEntry>* GetEventInfo();
    static std::unordered_map<int, EventInfoEntry> EventInfo;
    static std::unordered_map<std::string, int> EventNameToIdMap;
    static std::unordered_map<int, EventInfoEntry> _LoadPosEvent();
    static std::unordered_map<std::string, int> _LoadEventNameToIdMap();
    int GetEventIdFromMap(std::string eventName);

private:
};

static EventManager eventManager;

#endif // __EVENTM_MANAGER_H__
