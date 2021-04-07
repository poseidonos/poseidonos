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

#include "src/gc/copier_write_completion.h"

#include "src/array/free_buffer_pool.h"
#include "src/logger/logger.h"

namespace ibofos
{
GcChunkWriteCompletion::GcChunkWriteCompletion(void* buffer,
    uint32_t blkCnts,
    CopierMeta* meta,
    StripeId stripeId)
: Callback(false),
  buffer(buffer),
  blkCnts(blkCnts),
  meta(meta),
  stripeId(stripeId)
{
}

GcChunkWriteCompletion::~GcChunkWriteCompletion(void)
{
}

bool
GcChunkWriteCompletion::_DoSpecificJob(void)
{
    if (_GetErrorCount() != 0)
    {
        // TODO : chunk write fail handling
    }
    meta->SetDoneCopyBlks(blkCnts);
    meta->ReturnBuffer(stripeId, buffer);

    return true;
}

GcBlkWriteCompletion::GcBlkWriteCompletion(VolumeIoSmartPtr volumeIo)
: Callback(false),
  volumeIo(volumeIo)
{
#if defined QOS_ENABLED_BE
    SetFrontEnd(false);
    SetEventType(BackendEvent_GC);
#endif
}

GcBlkWriteCompletion::~GcBlkWriteCompletion(void)
{
}

bool
GcBlkWriteCompletion::_DoSpecificJob(void)
{
    if (_GetErrorCount() != 0)
    {
        // TODO(jg121.lim) : block write fail handling
    }
    volumeIo = nullptr;
    return true;
}

} // namespace ibofos
