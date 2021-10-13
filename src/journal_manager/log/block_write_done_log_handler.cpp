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

#include "src/journal_manager/log/block_write_done_log_handler.h"

namespace pos
{
BlockWriteDoneLogHandler::BlockWriteDoneLogHandler(int volId, BlkAddr startRba,
    uint32_t numBlks, VirtualBlkAddr startVsa, int wbIndex,
    StripeAddr stripeAddr)
{
    dat.type = LogType::BLOCK_WRITE_DONE;
    dat.volId = volId;
    dat.startRba = startRba;
    dat.numBlks = numBlks;
    dat.startVsa = startVsa;
    dat.wbIndex = wbIndex;
    dat.writeBufferStripeAddress = stripeAddr;
}

BlockWriteDoneLogHandler::BlockWriteDoneLogHandler(BlockWriteDoneLog& log)
{
    dat = log;
}

bool
BlockWriteDoneLogHandler::operator==(const  BlockWriteDoneLogHandler log)
{
    return ((dat.volId == log.dat.volId) && (dat.startRba == log.dat.startRba)
        && (dat.numBlks == log.dat.numBlks) && (dat.startVsa == log.dat.startVsa)
        && (dat.wbIndex == log.dat.wbIndex) && (dat.writeBufferStripeAddress == log.dat.writeBufferStripeAddress));
}

LogType
BlockWriteDoneLogHandler::GetType(void)
{
    return dat.type;
}

uint32_t
BlockWriteDoneLogHandler::GetSize(void)
{
    return sizeof(BlockWriteDoneLog);
}

char*
BlockWriteDoneLogHandler::GetData(void)
{
    return (char*)&dat;
}

StripeId
BlockWriteDoneLogHandler::GetVsid(void)
{
    return dat.startVsa.stripeId;
}

uint32_t
BlockWriteDoneLogHandler::GetSeqNum(void)
{
    return dat.seqNum;
}

void
BlockWriteDoneLogHandler::SetSeqNum(uint32_t num)
{
    dat.seqNum = num;
}

} // namespace pos
