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

#include "active_wb_stripe_replayer.h"

#include <iomanip>
#include <iostream>

#include "src/logger/logger.h"

namespace pos
{
ActiveWBStripeReplayer::ActiveWBStripeReplayer(PendingStripeList& pendingStripeList)
: pendingStripes(pendingStripeList),
  contextReplayer(nullptr),
  wbStripeAllocator(nullptr),
  stripeMap(nullptr),
  arrayInfo(nullptr)
{
}

ActiveWBStripeReplayer::ActiveWBStripeReplayer(IContextReplayer* ctxReplayer,
    IWBStripeAllocator* wbstripeAllocator, IStripeMap* stripeMap,
    PendingStripeList& pendingStripeList, IArrayInfo* aInfo)
: pendingStripes(pendingStripeList),
  contextReplayer(ctxReplayer),
  wbStripeAllocator(wbstripeAllocator),
  stripeMap(stripeMap),
  arrayInfo(aInfo)
{
    readTails = contextReplayer->GetAllActiveStripeTail();
    foundActiveStripes.resize(readTails.size());
}

ActiveWBStripeReplayer::~ActiveWBStripeReplayer(void)
{
}

void
ActiveWBStripeReplayer::Update(StripeInfo stripeInfo)
{
    int index = _FindWbufIndex(stripeInfo);

    if (_IsFlushedStripe(stripeInfo))
    {
        if (index != INDEX_NOT_FOUND)
        {
            _AddResetTailToFoundList(index);
        }
    }
    else
    {
        VirtualBlkAddr tailVsa = {
            .stripeId = stripeInfo.GetVsid(),
            .offset = stripeInfo.GetLastOffset() + 1};

        ActiveStripeAddr addr(stripeInfo.GetVolumeId(), tailVsa,
            stripeInfo.GetWbLsid());

        POS_TRACE_DEBUG(EID(JOURNAL_REPLAY_WB_STRIPE),
            "Update active wb stripe replayer, index {} volumeId {}, tail offset {}, wbLsid {}",
            index, stripeInfo.GetVolumeId(), tailVsa.offset, stripeInfo.GetWbLsid());

        if (index == INDEX_NOT_FOUND)
        {
            pendingActiveStripes.push_back(addr);
        }
        else
        {
            _AddWbufTailToFoundList(index, addr);
        }
    }
}

bool
ActiveWBStripeReplayer::_IsFlushedStripe(StripeInfo stripeInfo)
{
    return (stripeInfo.IsLastOffsetValid() == false);
}

int
ActiveWBStripeReplayer::_FindWbufIndex(StripeInfo stripeInfo)
{
    if (stripeInfo.IsWbIndexValid() == true)
    {
        return stripeInfo.GetWbIndex();
    }
    else
    {
        for (auto it = readTails.begin(); it != readTails.end(); it++)
        {
            if (it->stripeId == stripeInfo.GetVsid())
            {
                return (it - readTails.begin());
            }
        }
    }
    return INDEX_NOT_FOUND;
}

void
ActiveWBStripeReplayer::_AddResetTailToFoundList(int index)
{
    ActiveStripeAddr resetAddr;
    resetAddr.Reset();

    foundActiveStripes[index].push_back(resetAddr);
}

void
ActiveWBStripeReplayer::_AddWbufTailToFoundList(int index, ActiveStripeAddr newAddr)
{
    foundActiveStripes[index].push_back(newAddr);
}

void
ActiveWBStripeReplayer::UpdateRevMaps(int volId, StripeId vsid, uint64_t offset, BlkAddr startRba)
{
    ActiveStripeAddr* stripe = _FindStripe(volId, vsid);
    // TODO(cheolho.kang): Change assert code to error code
    assert(stripe != nullptr);
    stripe->UpdateRevMap(offset, startRba);
}

ActiveStripeAddr*
ActiveWBStripeReplayer::_FindStripe(int index, StripeId vsid)
{
    for (auto stripe = foundActiveStripes[index].begin(); stripe != foundActiveStripes[index].end(); stripe++)
    {
        if (stripe->IsValid() == true && stripe->GetTail().stripeId == vsid)
        {
            return &(*stripe);
        }
    }

    for (auto stripe = pendingActiveStripes.begin(); stripe != pendingActiveStripes.end(); stripe++)
    {
        if (stripe->IsValid() == true && stripe->GetTail().stripeId == vsid)
        {
            return &(*stripe);
        }
    }

    return nullptr;
}

int
ActiveWBStripeReplayer::Replay(void)
{
    int ret = _RestoreActiveStripes();
    if (ret < 0)
    {
        return ret;
    }

    ret = _RestorePendingStripes();
    return ret;
}

int
ActiveWBStripeReplayer::_RestoreActiveStripes(void)
{
    for (uint32_t index = 0; index < foundActiveStripes.size(); index++)
    {
        if ((foundActiveStripes[index].size() == 0) &&
            (IsUnMapVsa(readTails[index]) == false))
        {
            _AddActiveStripeToRestore(index);
        }

        if (foundActiveStripes[index].size() != 0)
        {
            ActiveStripeAddr current = _FindTargetActiveStripeAndRestore(index);
            if (current.IsValid() == true)
            {
                _SetActiveStripeTail(index, current);
            }
            else
            {
                _ResetActiveStripeTail(index);
            }
        }
    }
    return 0;
}

void
ActiveWBStripeReplayer::_AddActiveStripeToRestore(int index)
{
    StripeAddr stripeAddr = stripeMap->GetLSA(readTails[index].stripeId);
    if (_IsStripeFull(readTails[index]) && (stripeAddr.stripeLoc == IN_USER_AREA))
    {
        POS_TRACE_DEBUG(EID(JOURNAL_REPLAY_WB_STRIPE),
            "Reconstructing flushed active stripe will be skipped, wbIndex {}, vsid {}",
            index, readTails[index].stripeId);
    }
    else
    {
        assert(stripeAddr.stripeLoc == IN_WRITE_BUFFER_AREA);

        ActiveStripeAddr currentAddr(index, readTails[index], stripeAddr.stripeId);
        foundActiveStripes[index].push_back(currentAddr);
    }
}

bool
ActiveWBStripeReplayer::_IsStripeFull(VirtualBlkAddr vsa)
{
    assert(vsa.offset <= _GetNumBlksPerStripe());
    return vsa.offset == _GetNumBlksPerStripe();
}

uint32_t
ActiveWBStripeReplayer::_GetNumBlksPerStripe(void)
{
    const PartitionLogicalSize* udSize = arrayInfo->GetSizeInfo(PartitionType::USER_DATA);
    return udSize->blksPerStripe;
}

void
ActiveWBStripeReplayer::_SetActiveStripeTail(int index, ActiveStripeAddr addr)
{
    contextReplayer->SetActiveStripeTail(index,
        addr.GetTail(), addr.GetWbLsid());

    std::ostringstream logString;
    logString << "[Replay] Restore active stripe (index " << index
              << ", vsid " << addr.GetTail().stripeId
              << ", wbLsid " << addr.GetWbLsid()
              << ", offset " << addr.GetTail().offset << ")";
    POS_TRACE_DEBUG(EID(JOURNAL_REPLAY_WB_STRIPE), logString.str());
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, EID(JOURNAL_REPLAY_WB_STRIPE), logString.str());
}

void
ActiveWBStripeReplayer::_ResetActiveStripeTail(int index)
{
    contextReplayer->ResetActiveStripeTail(index);

    std::ostringstream logString;
    logString << "[Replay] Active stripe tail index " << index << " is reset";
    POS_TRACE_DEBUG(EID(JOURNAL_REPLAY_WB_STRIPE), logString.str());
    POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, EID(JOURNAL_REPLAY_WB_STRIPE), logString.str());
}

ActiveStripeAddr
ActiveWBStripeReplayer::_FindTargetActiveStripeAndRestore(int index)
{
    ActiveStripeAddr lastActiveStripe;

    for (auto it = foundActiveStripes[index].begin();
         it != foundActiveStripes[index].end();)
    {
        if (it->IsValid() == false)
        {
            it = foundActiveStripes[index].erase(it);
        }
        else
        {
            POS_TRACE_DEBUG(EID(JOURNAL_REPLAY_WB_STRIPE),
                "Start reconstructing stripe, vol {}, wbLsid {}", it->GetVolumeId(), it->GetWbLsid());

            int reconstructResult = wbStripeAllocator->ReconstructActiveStripe(it->GetVolumeId(),
                it->GetWbLsid(), it->GetTail(), it->GetRevMap());
            if (reconstructResult < 0)
            {
                int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STRIPE_FLUSH_FAILED));
                std::ostringstream os;
                os << "Failed to reconstruct active stripe, wb lsid " << it->GetWbLsid()
                   << ", tail offset " << it->GetTail().offset;

                POS_TRACE_ERROR(eventId, os.str());
                POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
            }
            else
            {
                if (lastActiveStripe.IsValid() == true)
                {
                    pendingStripes.push_back(new PendingStripe(lastActiveStripe.GetVolumeId(),
                        lastActiveStripe.GetWbLsid(), lastActiveStripe.GetTail()));
                }
                lastActiveStripe = *it;
            }
            ++it;
        }
    }

    return lastActiveStripe;
}

int
ActiveWBStripeReplayer::_RestorePendingStripes(void)
{
    for (auto it = pendingActiveStripes.begin();
         it != pendingActiveStripes.end();)
    {
        int reconstructResult = wbStripeAllocator->ReconstructActiveStripe(it->GetVolumeId(),
            it->GetWbLsid(), it->GetTail(), it->GetRevMap());
        if (reconstructResult < 0)
        {
            int eventId = static_cast<int>(EID(JOURNAL_REPLAY_STRIPE_FLUSH_FAILED));
            std::ostringstream os;
            os << "Failed to reconstruct active stripe, wb lsid " << it->GetWbLsid()
               << ", tail offset " << it->GetTail().offset;

            POS_TRACE_ERROR(eventId, os.str());
            POS_TRACE_DEBUG_IN_MEMORY(ModuleInDebugLogDump::JOURNAL, eventId, os.str());
        }
        else
        {
            pendingStripes.push_back(new PendingStripe(it->GetVolumeId(),
                it->GetWbLsid(), it->GetTail()));
        }
        it = pendingActiveStripes.erase(it);
    }
    return 0;
}
} // namespace pos
