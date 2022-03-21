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

#include "src/io/frontend_io/write_for_parity.h"
#include "src/io/general_io/io_submit_handler.h"
#include "src/bio/volume_io.h"
#include "src/logger/logger.h"

namespace pos
{

WriteForParity::WriteForParity(VolumeIoSmartPtr inputVolumeIo, bool isFrontEnd)
: Callback(isFrontEnd),
  volumeIo(inputVolumeIo)
{
}

bool
WriteForParity::_DoSpecificJob(void)
{
    LogicalBlkAddr blkAddr = {
        .stripeId = volumeIo->GetLsidEntry().stripeId,
        .offset = volumeIo->GetVsa().offset
    };
    uint32_t blockCount = DivideUp(volumeIo->GetSize(), BLOCK_SIZE);
    LogicalByteAddr byteAddr = {
        .blkAddr = blkAddr,
        .byteOffset = 0,
        .byteSize =(uint32_t)ChangeBlockToByte(blockCount)
    };

    IOSubmitHandlerStatus ioStatus =
        IIOSubmitHandler::GetInstance()->SubmitAsyncByteIO(
            IODirection::WRITE, volumeIo->GetBuffer(), byteAddr,
            PartitionType::WRITE_BUFFER, nullptr, volumeIo->GetArrayId());

    if (IOSubmitHandlerStatus::SUCCESS != ioStatus)
    {
        return false;
    }
    return true;
}

} // namespace pos
