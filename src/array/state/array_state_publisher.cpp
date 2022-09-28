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

#include "array_state_publisher.h"
#include "src/logger/logger.h"

namespace pos
{
ArrayStatePublisher::~ArrayStatePublisher()
{
    if (publisher != nullptr)
    {
        TelemetryClientSingleton::Instance()->DeregisterPublisher(publisher->GetName());
    }
    delete publisher;
}

void
ArrayStatePublisher::Register(string uuid)
{
    _RegisterTelemetry(uuid);
}

void
ArrayStatePublisher::PublishArrayState(const ArrayStateType& state)
{
    POSMetricValue v;
    v.gauge = static_cast<uint64_t>(state.ToEnum());
    publisher->PublishData(TEL60001_ARRAY_STATUS, v, POSMetricTypes::MT_GAUGE);
}

void
ArrayStatePublisher::_RegisterTelemetry(string uuid)
{
    if (publisher == nullptr)
    {
        publisher = new TelemetryPublisher("ArrayStatePublisher");
        publisher->AddDefaultLabel("array_unique_id", uuid);
        TelemetryClientSingleton::Instance()->RegisterPublisher(publisher);
    }
}

} // namespace pos
