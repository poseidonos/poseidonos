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

#include "written_logs.h"

#include "../log/log_handler.h"

template<typename T>
static void
ResetList(T& inputList)
{
    for (auto element : inputList)
    {
        delete element;
    }
    inputList.clear();
}

WrittenLogs::WrittenLogs(void)
{
    numJournalIssued = 0;
    numJournalAdded = 0;
}

WrittenLogs::~WrittenLogs(void)
{
    Reset();
}

void
WrittenLogs::Reset(void)
{
    ResetList(writeLogList);
    ResetList(stripeIos);
}

void
WrittenLogs::_AddToList(LogHandlerInterface* entry)
{
    std::lock_guard<std::mutex> lock(logListLock);
    writeLogList.push_back(entry);
}

void
WrittenLogs::AddToWriteList(VolumeIoSmartPtr volumeIo)
{
    volumeIos.push_back(volumeIo);
    numJournalIssued++;

    int volumeId = volumeIo->GetVolumeId();
    BlkAddr rba = ChangeSectorToBlock(volumeIo->GetRba());
    uint64_t numBlks = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    VirtualBlkAddr startVsa = volumeIo->GetVsa();
    StripeAddr writeBufferStripe = volumeIo->GetLsidEntry();
    ActiveStripeTailArrIdxInfo indexFinder = ActiveStripeTailArrIdxInfo(volumeId, false);
    int index = indexFinder.GetActiveStripeTailArrIdx();
    VirtualBlkAddr oldVsa = volumeIo->GetOldVsa();
    bool isGC = volumeIo->IsGc();

    LogHandlerInterface* entry = new BlockWriteDoneLogHandler(volumeId,
        rba, numBlks, startVsa, index, writeBufferStripe, oldVsa, isGC);

    _AddToList(entry);
}

void
WrittenLogs::AddToWriteList(Stripe* stripe, StripeAddr oldAddr)
{
    stripeIos.push_back(stripe);
    numJournalIssued++;

    StripeAddr newAddr = {
        .stripeLoc = IN_USER_AREA,
        .stripeId = stripe->GetUserLsid()};
    LogHandlerInterface* entry = new StripeMapUpdatedLogHandler(stripe->GetVsid(), oldAddr, newAddr);

    _AddToList(entry);
}

bool
WrittenLogs::CheckLogInTheList(LogHandlerInterface* log)
{
    bool exist = false;
    for (auto it = writeLogList.begin(); it != writeLogList.end(); it++)
    {
        if ((*it)->GetType() == log->GetType())
        {
            if (log->GetType() == LogType::BLOCK_WRITE_DONE)
            {
                BlockWriteDoneLogHandler* cmp1 = reinterpret_cast<BlockWriteDoneLogHandler*>(*it);
                BlockWriteDoneLogHandler* cmp2 = reinterpret_cast<BlockWriteDoneLogHandler*>(log);

                exist = (*cmp1 == *cmp2);
            }
            else if (log->GetType() == LogType::STRIPE_MAP_UPDATED)
            {
                StripeMapUpdatedLogHandler* cmp1 = reinterpret_cast<StripeMapUpdatedLogHandler*>(*it);
                StripeMapUpdatedLogHandler* cmp2 = reinterpret_cast<StripeMapUpdatedLogHandler*>(log);

                exist = (*cmp1 == *cmp2);
            }
            else if (log->GetType() == LogType::VOLUME_DELETED)
            {
                VolumeDeletedLogEntry* cmp1 = reinterpret_cast<VolumeDeletedLogEntry*>(*it);
                VolumeDeletedLogEntry* cmp2 = reinterpret_cast<VolumeDeletedLogEntry*>(log);

                exist = (*cmp1 == *cmp2);
            }

            if (exist == true)
            {
                break;
            }
        }
    }
    return exist;
}

void
WrittenLogs::JournalWriteDone(void)
{
    numJournalAdded++;
}

void
WrittenLogs::WaitForAllLogWriteDone(void)
{
    while (numJournalIssued != numJournalAdded)
    {
    }
}

bool
WrittenLogs::DoesAllJournalWriteDone(void)
{
    return (numJournalIssued == numJournalAdded);
}

uint32_t
WrittenLogs::GetNumLogsInTesting(void)
{
    return numJournalIssued;
}
