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

#include "src/bio/flush_io.h"
#include "src/volume/volume_list.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
FlushIo::FlushIo(int arrayId)
: Ubio(nullptr, 0, arrayId),
  volumeId(MAX_VOLUME_COUNT),
  originCore(EventFrameworkApiSingleton::Instance()->GetCurrentReactor()),
  state(FLUSH__BLOCKING_ALLOCATION),
  isInternalFlush(false),
  stripeCnt(0),
  stripesScanComplete(false),
  stripesFlushComplete(false),
  vsaMapFlushComplete(false),
  stripeMapFlushComplete(false),
  allocatorFlushComplete(false),
  metaFlushComplete(false)
{
}

FlushIo::~FlushIo(void)
{
}

uint32_t
FlushIo::GetVolumeId(void)
{
    return volumeId;
}

void
FlushIo::SetVolumeId(uint32_t volumeId)
{
    this->volumeId = volumeId;
}

uint32_t
FlushIo::GetOriginCore(void)
{
    return originCore;
}

FlushState
FlushIo::GetState(void)
{
    return state;
}

void
FlushIo::SetState(FlushState state)
{
    this->state = state;
}

bool
FlushIo::IsVsaMapFlushComplete(void)
{
    return vsaMapFlushComplete;
}

void
FlushIo:: SetVsaMapFlushComplete(bool vsaMapFlushComplete)
{
    this->vsaMapFlushComplete = vsaMapFlushComplete;
}

bool
FlushIo::IsStripeMapFlushComplete(void)
{
    return stripeMapFlushComplete;
}

void
FlushIo::SetStripeMapFlushComplete(bool stripeMapFlushComplete)
{
    this->stripeMapFlushComplete = stripeMapFlushComplete;
}

bool
FlushIo::IsAllocatorFlushComplete(void)
{
    return allocatorFlushComplete;
}

void
FlushIo::SetAllocatorFlushComplete(bool allocatorFlushComplete)
{
    this->allocatorFlushComplete = allocatorFlushComplete;
}

bool
FlushIo::IsStripeMapAllocatorFlushComplete(void)
{
    return metaFlushComplete;
}

void
FlushIo::SetStripeMapAllocatorFlushComplete(bool metaFlushComplete)
{
    this->metaFlushComplete = metaFlushComplete;
}

void
FlushIo::IncreaseStripeCnt(void)
{
    stripeCnt++;
}

void
FlushIo::DecreaseStripeCnt(void)
{
    stripeCnt--;

    if (stripeCnt == 0 && stripesScanComplete == true)
    {
        stripesFlushComplete = true;
    }
}

void
FlushIo::StripesScanComplete(void)
{
    stripesScanComplete = true;

    if (stripeCnt == 0)
    {
        stripesFlushComplete = true;
    }
}

bool
FlushIo::IsStripesFlushComplete(void)
{
    return stripesFlushComplete;
}

bool
FlushIo::IsInternalFlush(void)
{
    return isInternalFlush;
}

void
FlushIo::SetInternalFlush(bool isInternalFlush)
{
    this->isInternalFlush = isInternalFlush;
}

} // namespace pos
