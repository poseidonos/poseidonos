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

#ifndef __IBOF_EVENT_ID_HPP__
#define __IBOF_EVENT_ID_HPP__

#include <string>

#include "pos_event_id.h"

namespace pos
{
enum class EventLevel
{
    CRITICAL,
    ERROR,
    WARNING,
    INFO,
    DEBUG,
};

class PosEventId
{
public:
    static const char*& GetString(POS_EVENT_ID eventId);
    static std::string GetJsonLogMsg(int eventId);
    static void Print(POS_EVENT_ID id, EventLevel level);
    static void Print(POS_EVENT_ID id, EventLevel level,
        std::string& additionalMessage);

private:
    struct PosEventIdEntry
    {
        POS_EVENT_ID eventId;
        const char* message = "";
        const char* cause = "";
    };

    struct PosEventInfoEntry
    {
        POS_EVENT_ID eventId;
        const char* eventName = "";
        const char* message = "";
        const char* cause = "";
    };

    static PosEventIdEntry RESERVED_EVENT_ENTRY;
    static PosEventIdEntry SYSTEM_EVENT_ENTRY[(int)POS_EVENT_ID::SYSTEM_COUNT];
    static PosEventIdEntry ARRAY_EVENT_ENTRY[(int)POS_EVENT_ID::ARRAY_COUNT];
    static PosEventIdEntry QOS_EVENT_ENTRY[(int)POS_EVENT_ID::QOS_COUNT];
    static PosEventIdEntry IOPATH_NVMF_EVENT_ENTRY[(int)POS_EVENT_ID::IONVMF_COUNT];
    static PosEventIdEntry IOPATH_FRONTEND_EVENT_ENTRY[(int)POS_EVENT_ID::IOFRONTEND_COUNT];
    static PosEventIdEntry IOPATH_BACKEND_EVENT_ENTRY[(int)POS_EVENT_ID::IOBACKEND_COUNT];

    PosEventId(void) = delete;
    ~PosEventId(void);
};

} // namespace pos
#endif // __IBOF_EVENT_ID_HPP__
