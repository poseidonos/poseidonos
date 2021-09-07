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
#include "src/telemetry/telemetry_air/telemetry_air_delegator.h"

#include <exception>
#include <iostream>
#include <sstream>
#include <string>

#include "Air.h"
#include "src/telemetry/telemetry_id.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

namespace pos
{
TelemetryAirDelegator::TelemetryAirDelegator(TelemetryClient* telClient, TelemetryPublisher* telPub)
: telClient(telClient),
  telPub(telPub)
{
}

TelemetryAirDelegator::~TelemetryAirDelegator(void)
{
}

void
TelemetryAirDelegator::StartDelegation(void)
{
    returnState = stateRun;
    air_request_data({"PERF_ARR_VOL"},
        [this](const air::JSONdoc& data) -> int {
            const std::lock_guard<std::mutex> lock(this->mutex);
            try
            {
                if (data.HasKey("PERF_ARR_VOL"))
                {
                    auto& objs = data["PERF_ARR_VOL"]["objs"];
                    for (auto& obj_it: objs)
                    {
                        auto& obj = objs[obj_it.first];
                        std::stringstream stream_filter;
                        stream_filter << obj["filter"];
                        std::string str_filter = stream_filter.str();
                        if (0 == str_filter.compare("\"AIR_READ\""))
                        {
                            std::stringstream stream_iops;
                            stream_iops << obj["iops"];
                            uint32_t iops = 0;
                            stream_iops >> iops;
                            this->telPub->PublishData(TEL_VOLUME_IOPS_READ, iops);
                            /* TODO: Need more params in PublishData not only iops
                            but thread id, array id and volume id
                            obj["index"] => 0x0104 (array id = 1, volume id = 4)
                            obj["target_id"] => thread id
                            obj["target_name"] => thread name
                            */
                        }
                    }
                }
            }
            catch (std::exception& e)
            {
                std::cout << e.what() << std::endl;
                this->returnState = this->stateEnd;
            }
            return this->returnState;
        });
}

void
TelemetryAirDelegator::StopDelegation(void)
{
    const std::lock_guard<std::mutex> lock(mutex);
    returnState = stateEnd;
}

} // namespace pos
