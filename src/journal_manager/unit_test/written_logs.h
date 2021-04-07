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

#pragma once

#include <atomic>
#include <list>

#include "../log/log_handler.h"
#include "src/allocator/stripe.h"
#include "src/include/address_type.h"
#include "src/io/general_io/volume_io.h"

using namespace ibofos;

class WrittenLogs
{
public:
    WrittenLogs(void);
    ~WrittenLogs(void);

    void Reset(void);

    void AddToWriteList(VolumeIoSmartPtr volumeIo);
    void AddToWriteList(Stripe* stripe, StripeAddr oldAddr);

    bool CheckLogInTheList(LogHandlerInterface* log);
    void JournalWriteDone(void);
    void WaitForAllLogWriteDone(void);

    bool DoesAllJournalWriteDone(void);
    uint32_t GetNumLogsInTesting(void);

    void CompareLogList(LogList& input);

private:
    void _AddToList(LogHandlerInterface* entry);

    std::mutex logListLock;
    LogList writeLogList;

    std::atomic<uint32_t> numJournalIssued;
    std::atomic<uint32_t> numJournalAdded;

    std::list<VolumeIoSmartPtr> volumeIos;
    std::list<Stripe*> stripeIos;
};
