/*
 *   BSD LICENSE
 *   Copyright (c) 20` Samsung Electronics Corporation
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

#include "smart_collector.h"

#include <stdio.h>
#include <string.h>

#include <tuple>
#include <vector>

#include "spdk/include/spdk/nvme_spec.h"
#include "src/admin/disk_query_manager.h"
#include "src/array_models/dto/device_set.h"
#include "src/include/smart_ptr_type.h"
#include "src/logger/logger.h"

using namespace std;

namespace pos
{
SmartCollector::SmartCollector(void)
{
    if (nullptr == publisher)
    {
        publisher = new TelemetryPublisher("Disk_Monitoring");
    }

    if (nullptr == telemetryClient)
    {
        telemetryClient = TelemetryClientSingleton::Instance();
        telemetryClient->RegisterPublisher(publisher);
    }

    POS_TRACE_DEBUG(EID(SMART_COLLECTOR_CONSTRUCTOR_EXECUTION), "execute constructor");
}

SmartCollector::~SmartCollector(void)
{
    if (nullptr != publisher)
    {
        telemetryClient->DeregisterPublisher(publisher->GetName());
        delete publisher;
        publisher = nullptr;
    }

    telemetryClient = nullptr;

    POS_TRACE_DEBUG(EID(SMART_COLLECTOR_DESTRUCTOR_EXECUTION), "execute destructor");
}

void
SmartCollector::PublishSmartDataToTelemetryAllCtrl(void)
{
    struct spdk_nvme_ctrlr* ctrlr;
    IIODispatcher* dispatcher = IODispatcherSingleton::Instance();
    DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();

    struct spdk_nvme_health_information_page* payload = new struct spdk_nvme_health_information_page();
    uint16_t lid = SPDK_NVME_LOG_HEALTH_INFORMATION;
    GetLogPageContext* smartLogPageContext = new GetLogPageContext(payload, lid);

    vector<DeviceProperty> list = deviceMgr->ListDevs();
    for (auto device : list)
    {
        ctrlr = deviceMgr->GetNvmeCtrlr(device.name);
        if (nullptr == ctrlr)
        {
            // NVRAM case. ListDevs() includes SSD and NVRAM. But GetNvmeCtrlr() return only SSD.
            continue;
        }

        // re-use payload & smartLogPageContext
        UbioSmartPtr ubio(new Ubio((void*)smartLogPageContext, sizeof(struct spdk_nvme_health_information_page), 0 /*array index*/));
        DevName dev(device.name);
        UblockSharedPtr targetDevice = deviceMgr->GetDev(dev);

        ubio->dir = UbioDir::GetLogPage;
        ubio->SetUblock(targetDevice);

        int result = dispatcher->Submit(ubio, true);
        if (result < 0 || ubio->GetError() != IOErrorType::SUCCESS)
        {
            POS_TRACE_INFO(EID(SMART_COLLECTOR_CMD_EXEC_ERR), "result:{}", result);
        }
        else
        {
            PublishTelemetry(payload, device.name);
        }
    }

    delete payload;
    delete smartLogPageContext;
}

void
SmartCollector::PublishTelemetry(spdk_nvme_health_information_page* payload, string deviceName)
{
    POSMetricVector* metricList = new POSMetricVector();
    vector<pair<string, uint64_t>> smartEntries;

    smartEntries.push_back(make_pair(TEL110000_MEDIA_ERROR_COUNT_LOW, payload->media_errors[0]));
    smartEntries.push_back(make_pair(TEL110001_MEDIA_ERROR_COUNT_HIGH, payload->media_errors[1]));
    smartEntries.push_back(make_pair(TEL110002_POWER_CYCLE_LOW, payload->power_cycles[0]));
    smartEntries.push_back(make_pair(TEL110003_POWER_CYCLE_HIGH, payload->power_cycles[1]));
    smartEntries.push_back(make_pair(TEL110004_POWER_ON_HOUR_LOW, payload->power_on_hours[0]));
    smartEntries.push_back(make_pair(TEL110005_POWER_ON_HOUR_HIGH, payload->power_on_hours[1]));
    smartEntries.push_back(make_pair(TEL110006_UNSAFE_SHUTDOWNS_LOW, payload->unsafe_shutdowns[0]));
    smartEntries.push_back(make_pair(TEL110007_UNSAFE_SHUTDOWNS_HIGH, payload->unsafe_shutdowns[1]));
    smartEntries.push_back(make_pair(TEL110008_TEMPERATURE, (uint64_t)payload->temperature));
    smartEntries.push_back(make_pair(TEL110009_AVAILABLE_SPARE, payload->available_spare));
    smartEntries.push_back(make_pair(TEL110010_AVAILABLE_SPARE_THRESHOLD, payload->available_spare_threshold));
    smartEntries.push_back(make_pair(TEL110011_PERCENTAGE_USED, payload->percentage_used));
    smartEntries.push_back(make_pair(TEL110012_CONTROLLER_BUSY_TIME_LOW, payload->controller_busy_time[0]));
    smartEntries.push_back(make_pair(TEL110013_CONTROLLER_BUSY_TIME_HIGH, payload->controller_busy_time[1]));
    smartEntries.push_back(make_pair(TEL110014_WARNING_TEMP_TIME, (uint64_t)payload->warning_temp_time));
    smartEntries.push_back(make_pair(TEL110015_CRITICAL_TEMP_TIME, (uint64_t)payload->critical_temp_time));

    for (auto entry : smartEntries)
    {
        POSMetric metric(entry.first, POSMetricTypes::MT_GAUGE);
        metric.AddLabel("nvme_ctrl_id", deviceName);
        metric.SetGaugeValue(static_cast<uint64_t>(entry.second));
        metricList->push_back(metric);
    }

    if (!metricList->empty() && nullptr != publisher)
    {
        publisher->PublishMetricList(metricList);
    }
}

SmartReturnType
SmartCollector::CollectPerCtrl(spdk_nvme_health_information_page* payload, spdk_nvme_ctrlr* ctrlr)
{
    SmartReturnType result = SmartReturnType::SUCCESS;
    if (nullptr != ctrlr)
    {
        if (spdk_nvme_ctrlr_cmd_get_log_page(ctrlr, SPDK_NVME_LOG_HEALTH_INFORMATION, SPDK_NVME_GLOBAL_NS_TAG,
                payload, sizeof(spdk_nvme_health_information_page), 0, &CompleteSmartLogPage, NULL))
        {
            result = SmartReturnType::SEND_ERR;
        }

        int ret;
        while ((ret = spdk_nvme_ctrlr_process_admin_completions(ctrlr)) <= 0)
        {
            if (ret < 0)
            {
                result = SmartReturnType::RESPONSE_ERR;
            }

            // sleep 10ms to avoid excessive spdk occupancy
            usleep(10000);
        }
    }
    else
    {
        result = SmartReturnType::CTRL_NOT_EXIST;
    }

    return result;
}

void
SmartCollector::CompleteSmartLogPage(void* arg, const spdk_nvme_cpl* cpl)
{
    // do nothing
}
} // namespace pos
