/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/include/smart_ptr_type.h"
#include "src/mapper/include/mapper_const.h"
#include "src/mapper/include/mpage_info.h"

namespace pos
{
class LogHandlerInterface;

class LogWriteContext
{
public:
    LogWriteContext(void);
    LogWriteContext(LogHandlerInterface* inputLog, EventSmartPtr callbackEvent);
    LogWriteContext(LogHandlerInterface* inputLog, MapList inputMapList, EventSmartPtr callbackEvent);
    virtual ~LogWriteContext(void);

    virtual void SetLogAllocated(int logGroupId, uint64_t sequenceNumber);

    virtual MapList& GetDirtyMapList(void);
    virtual int GetLogGroupId(void);

    virtual uint64_t GetLogSize(void);
    virtual char* GetBuffer(void);
    virtual EventSmartPtr GetCallback(void);

    virtual LogHandlerInterface* GetLog(void);

private:
    LogHandlerInterface* log;
    MapList dirtyMap;

    int logGroupId;
    EventSmartPtr callback;
};

} // namespace pos
