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

#include "smart_log_update_request.h"

#include <sstream>

#include "src/admin/disk_smart_complete_handler.h"
#include "src/event_scheduler/event_scheduler.h"
#include "src/spdk_wrapper/event_framework_api.h"
namespace pos
{
SmartLogUpdateRequest::SmartLogUpdateRequest(struct spdk_nvme_health_information_page* resultPage, struct spdk_nvme_health_information_page* page, pos_io* io, uint32_t originCore)
: Callback(false, CallbackType_SmartLogUpdateRequest),
  resultPage(resultPage),
  page(page),
  io(io),
  originCore(originCore)
{
}
SmartLogUpdateRequest::~SmartLogUpdateRequest(void)
{
}

void
SmartLogUpdateRequest::_ClearMemory(void)
{
    delete page;
}

bool
SmartLogUpdateRequest::_DoSpecificJob(void)
{
    _CalculateVarBasedVal();
    _ClearMemory();
    return true;
}

void
SmartLogUpdateRequest::_CalculateVarBasedVal(void)
{
    // this is the first completion coming from disk. Assign the values
    std::unique_lock<std::mutex> lock(updateMutex);
    if (resultPage->temperature == 0 && page->temperature != 0)
    {
        resultPage->data_units_read[0] = page->data_units_read[0];
        resultPage->data_units_read[1] = page->data_units_read[1];
        resultPage->temperature = page->temperature;
        resultPage->available_spare = page->available_spare;
        resultPage->available_spare_threshold = page->available_spare_threshold;
        resultPage->percentage_used = page->percentage_used;
        resultPage->warning_temp_time = page->warning_temp_time;
        resultPage->critical_temp_time = page->critical_temp_time;

        for (int i = 0; i < NO_OF_TEMP_SENSORS; i++)
            resultPage->temp_sensor[i] = page->temp_sensor[i];
    }
    else if (page->temperature != 0)
    {
        // calculate max temperature
        if (page->temperature > resultPage->temperature)
        {
            resultPage->temperature = page->temperature;
        }
        // calculate min available spare
        if (page->available_spare < resultPage->available_spare)
        {
            resultPage->available_spare = page->available_spare;
        }
        // calculate min available spare threhold
        if (page->available_spare_threshold < resultPage->available_spare_threshold)
        {
            resultPage->available_spare_threshold = page->available_spare_threshold;
        }
        // calculate max percentage used
        if (page->percentage_used > resultPage->percentage_used)
        {
            resultPage->percentage_used = page->percentage_used;
        }
        // calculate max temperature time
        if (page->warning_temp_time > resultPage->warning_temp_time)
        {
            resultPage->warning_temp_time = page->warning_temp_time;
        }
        // calculate max temperature time
        if (page->critical_temp_time > resultPage->critical_temp_time)
        {
            resultPage->critical_temp_time = page->critical_temp_time;
        }

        // calculate max value for all temperature sensors
        for (int i = 0; i < NO_OF_TEMP_SENSORS; i++)
            if (page->temp_sensor[i] > resultPage->temp_sensor[i])
                resultPage->temp_sensor[i] = page->temp_sensor[i];
    }
    return;
}

} // namespace pos
