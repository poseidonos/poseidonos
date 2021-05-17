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

#include "meta_mount_sequence.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#ifdef _ADMIN_ENABLED
#include "src/admin/smart_log_mgr.h"
#endif

namespace pos
{
MetaMountSequence::MetaMountSequence(std::string arrayName,
    IMountSequence* mapper, IMountSequence* allocator, IMountSequence* journal)
: arrayName(arrayName),
  mapper(mapper),
  allocator(allocator),
  journal(journal)
{
}

MetaMountSequence::~MetaMountSequence(void)
{
}

int
MetaMountSequence::Init(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::ARRAY_MOUNTING);
    int result = 0;

    POS_TRACE_INFO(eventId, "Start initializing mapper of array {}", arrayName);
    result = mapper->Init();
    if (result != 0)
    {
        mapper->Dispose();
        return result;
    }

    POS_TRACE_INFO(eventId, "Start initializing allocator of array {}", arrayName);
    result = allocator->Init();
    if (result != 0)
    {
        allocator->Dispose();
        mapper->Dispose();
        return result;
    }

    POS_TRACE_INFO(eventId, "Start initializing journal of array {}", arrayName);
    result = journal->Init();
    if (result != 0)
    {
        journal->Dispose();
        allocator->Dispose();
        mapper->Dispose();
        return result;
    }
#ifdef _ADMIN_ENABLED
    string fileName = "SmartLogPage.bin";
    MetaFileIntf* smartLogFile = new FILESTORE(fileName, "");
    SmartLogMgrSingleton::Instance()->Init(smartLogFile);
#endif
    return result;
}

void
MetaMountSequence::Dispose(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::ARRAY_UNMOUNTING);

    POS_TRACE_INFO(eventId, "Start disposing allocator of array {}", arrayName);
    allocator->Dispose();

    POS_TRACE_INFO(eventId, "Start disposing mapper of array {}", arrayName);
    mapper->Dispose();

    POS_TRACE_INFO(eventId, "Start disposing journal of array {}", arrayName);
    journal->Dispose();
}

void
MetaMountSequence::Shutdown(void)
{
    int eventId = static_cast<int>(POS_EVENT_ID::ARRAY_UNMOUNTING);

    POS_TRACE_INFO(eventId, "Start shutdown allocator of array {}", arrayName);
    allocator->Shutdown();

    POS_TRACE_INFO(eventId, "Start shutdown mapper of array {}", arrayName);
    mapper->Shutdown();

    POS_TRACE_INFO(eventId, "Start shutdown journal of array {}", arrayName);
    journal->Shutdown();
}

} // namespace pos
