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
#include "src/device/unvme/unvme_ssd.h"
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
    DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
    vector<DeviceProperty> list = deviceMgr->ListDevs();

    // SPDK_NVME_LOG_HEALTH_INFORMATION
    for (auto device : list)
    {
        struct spdk_nvme_ctrlr* ctrlr = deviceMgr->GetNvmeCtrlr(device.name);
        if (nullptr == ctrlr)
        {
            // NVRAM case. ListDevs() includes SSD and NVRAM. But GetNvmeCtrlr() return only SSD.
            continue;
        }

        struct spdk_nvme_health_information_page* payload = new struct spdk_nvme_health_information_page();
        if (0 == CollectGetLogPage(payload, ctrlr, device.name, SmartReqId::NVME_HEALTH_INFO))
        {
            POS_TRACE_DEBUG(EID(SMART_COLLECTOR_EXT_CMD_DBG), "Device Model name:{}, FW REV:{}", device.mn, device.fr);
            PublishSmartTelemetry(payload, device.name);

            DevName dev(device.name);
            UnvmeSsdSharedPtr ssdDev = dynamic_pointer_cast<UnvmeSsd>(deviceMgr->GetDev(dev));

            if (ssdDev->IsSupportedExtSmart())
            {
                struct spdk_nvme_log_samsung_extended_information_entry* extPayload =
                    new struct spdk_nvme_log_samsung_extended_information_entry();
                if (0 == CollectGetLogPage(extPayload, ctrlr, device.name, SmartReqId::EXTENDED_SMART_INFO))
                {
                    PublishExtSmartTelemetry(extPayload, device.name);
                }
                delete extPayload;
            }
            else
            {
                POS_TRACE_DEBUG(EID(SMART_COLLECTOR_EXT_CMD_EXEC_SKIP), "Skip extended smart for only general");
            }
        }
        delete payload;
    }
}

int
SmartCollector::CollectGetLogPage(void* payload, spdk_nvme_ctrlr* ctrlr, std::string deviceName, SmartReqId reqId)
{
    uint16_t lid = 0;
    uint32_t payloadSize = 0;
    switch (reqId)
    {
        case SmartReqId::NVME_HEALTH_INFO:
            payloadSize = sizeof(spdk_nvme_health_information_page);
            lid = SPDK_NVME_LOG_HEALTH_INFORMATION;
            break;

        case SmartReqId::EXTENDED_SMART_INFO:
            payloadSize = sizeof(spdk_nvme_log_samsung_extended_information_entry);
            lid = SPDK_NVME_LOG_EXTENDED_SMART;
            break;

        case SmartReqId::OCP_INFO:
            payloadSize = sizeof(spdk_nvme_log_ocp_information_entry);
            lid = SPDK_NVME_LOG_OCP;
            break;

        default:
            assert(false);
    }

    IIODispatcher* dispatcher = IODispatcherSingleton::Instance();
    DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();

    GetLogPageContext* smartLogPageContext = new GetLogPageContext(payload, lid);
    UbioSmartPtr ubio(new Ubio((void*)smartLogPageContext, payloadSize, 0 /*array index*/));
    DevName dev(deviceName);
    UblockSharedPtr targetDevice = deviceMgr->GetDev(dev);

    ubio->dir = UbioDir::GetLogPage;
    ubio->SetUblock(targetDevice);

    int result = dispatcher->Submit(ubio, true);
    if (result < 0 || ubio->GetError() != IOErrorType::SUCCESS)
    {
        result = -1;
        POS_TRACE_INFO(EID(SMART_COLLECTOR_CMD_EXEC_ERR), "result:{} ubio->GetError():{}", result, ubio->GetError());
    }

    delete smartLogPageContext;
    return result;
}

void
SmartCollector::PublishSmartTelemetry(spdk_nvme_health_information_page* payload, string deviceName)
{
    POSMetricVector* metricList = new POSMetricVector();
    vector<pair<string, int64_t>> entries;

    ConvertMaxInt64Value(&payload->media_errors[1], &payload->media_errors[0]);
    entries.push_back(make_pair(TEL110000_MEDIA_ERROR_COUNT_LOWER, payload->media_errors[0]));
    entries.push_back(make_pair(TEL110001_MEDIA_ERROR_COUNT_UPPER, payload->media_errors[1]));

    ConvertMaxInt64Value(&payload->power_cycles[1], &payload->power_cycles[0]);
    entries.push_back(make_pair(TEL110002_POWER_CYCLE_LOWER, payload->power_cycles[0]));
    entries.push_back(make_pair(TEL110003_POWER_CYCLE_UPPER, payload->power_cycles[1]));

    ConvertMaxInt64Value(&payload->power_on_hours[1], &payload->power_on_hours[0]);
    entries.push_back(make_pair(TEL110004_POWER_ON_HOUR_LOWER, payload->power_on_hours[0]));
    entries.push_back(make_pair(TEL110005_POWER_ON_HOUR_UPPER, payload->power_on_hours[1]));

    ConvertMaxInt64Value(&payload->unsafe_shutdowns[1], &payload->unsafe_shutdowns[0]);
    entries.push_back(make_pair(TEL110006_UNSAFE_SHUTDOWNS_LOWER, payload->unsafe_shutdowns[0]));
    entries.push_back(make_pair(TEL110007_UNSAFE_SHUTDOWNS_UPPER, payload->unsafe_shutdowns[1]));

    entries.push_back(make_pair(TEL110008_TEMPERATURE, static_cast<int64_t>(payload->temperature)));
    entries.push_back(make_pair(TEL110009_AVAILABLE_SPARE, payload->available_spare));
    entries.push_back(make_pair(TEL110010_AVAILABLE_SPARE_THRESHOLD, payload->available_spare_threshold));
    entries.push_back(make_pair(TEL110011_PERCENTAGE_USED, payload->percentage_used));

    ConvertMaxInt64Value(&payload->controller_busy_time[1], &payload->controller_busy_time[0]);
    entries.push_back(make_pair(TEL110012_CONTROLLER_BUSY_TIME_LOWER, payload->controller_busy_time[0]));
    entries.push_back(make_pair(TEL110013_CONTROLLER_BUSY_TIME_UPPER, payload->controller_busy_time[1]));

    entries.push_back(make_pair(TEL110014_WARNING_TEMP_TIME, static_cast<int64_t>(payload->warning_temp_time)));
    entries.push_back(make_pair(TEL110015_CRITICAL_TEMP_TIME, static_cast<int64_t>(payload->critical_temp_time)));

    for (auto entry : entries)
    {
        POSMetric metric(entry.first, POSMetricTypes::MT_GAUGE);
        metric.AddLabel("nvme_ctrl_id", deviceName);
        metric.SetGaugeValue(entry.second);
        metricList->push_back(metric);
    }

    if (!metricList->empty() && nullptr != publisher)
    {
        publisher->PublishMetricList(metricList);
    }
}

void
SmartCollector::PublishExtSmartTelemetry(spdk_nvme_log_samsung_extended_information_entry* payload, std::string deviceName)
{
    vector<pair<string, int64_t>> entries;
    entries.push_back(make_pair(TEL110020_LIFETIME_WAF, static_cast<int64_t>(payload->lifetime_write_amplification_factor)));
    entries.push_back(make_pair(TEL110021_HOUR_WAF, static_cast<int64_t>(payload->hour_write_amplification_factor)));

    ConvertMaxInt64Value(&payload->trim_sector_count[1], &payload->trim_sector_count[0]);
    entries.push_back(make_pair(TEL110022_TRIM_SECTOR_COUNT_LOWER, payload->trim_sector_count[0]));
    entries.push_back(make_pair(TEL110023_TRIM_SECTOR_COUNT_UPPER, payload->trim_sector_count[1]));

    ConvertMaxInt64Value(&payload->lifetime_user_write_count[1], &payload->lifetime_user_write_count[0]);
    entries.push_back(make_pair(TEL110024_HOST_WRITTEN_COUNT_LOWER, payload->lifetime_user_write_count[0]));
    entries.push_back(make_pair(TEL110025_HOST_WRITTEN_COUNT_UPPER, payload->lifetime_user_write_count[1]));

    ConvertMaxInt64Value(&payload->lifetime_nand_write_count[1], &payload->lifetime_nand_write_count[0]);
    entries.push_back(make_pair(TEL110026_NAND_WRITTEN_COUNT_LOWER, payload->lifetime_nand_write_count[0]));
    entries.push_back(make_pair(TEL110027_NAND_WRITTEN_COUNT_UPPER, payload->lifetime_nand_write_count[1]));

    entries.push_back(make_pair(TEL110028_THERMAL_THROTTLE_EVNET_COUNT, static_cast<int64_t>(payload->thermal_throttle_info.event_count)));
    entries.push_back(make_pair(TEL110029_HIGHEST_TEMPERATURE, static_cast<int64_t>(payload->highest_temperature)));
    entries.push_back(make_pair(TEL110030_LOWEST_TEMPERATURE, static_cast<int64_t>(payload->lowest_temeperature)));
    entries.push_back(make_pair(TEL110031_OVER_TEMPERATURE_COUNT, static_cast<int64_t>(payload->over_temperature_count)));
    entries.push_back(make_pair(TEL110032_UNDER_TEMPERATURE_COUNT, static_cast<int64_t>(payload->under_temperature_count)));

    POSMetricVector* metricList = new POSMetricVector();
    for (auto entry : entries)
    {
        POSMetric metric(entry.first, POSMetricTypes::MT_GAUGE);
        metric.AddLabel("nvme_ctrl_id", deviceName);
        metric.SetGaugeValue(entry.second);
        metricList->push_back(metric);
    }

    if (!metricList->empty() && nullptr != publisher)
    {
        publisher->PublishMetricList(metricList);
    }
}

SmartReturnType
SmartCollector::CollectPerCtrl(void* payload, spdk_nvme_ctrlr* ctrlr, SmartReqId reqId)
{
    uint8_t lid = 0;
    uint32_t payloadSize = 0;
    SmartReturnType result = SmartReturnType::SUCCESS;

    switch (reqId)
    {
        case SmartReqId::NVME_HEALTH_INFO:
            payload = static_cast<spdk_nvme_health_information_page*>(payload);
            payloadSize = sizeof(spdk_nvme_health_information_page);
            lid = SPDK_NVME_LOG_HEALTH_INFORMATION;
            break;

        case SmartReqId::EXTENDED_SMART_INFO:
        case SmartReqId::OCP_INFO:
            // fall through
        default:
            result = SmartReturnType::LID_NOT_SUPPORTED;
            return result;
    }

    if (nullptr != ctrlr)
    {
        if (spdk_nvme_ctrlr_cmd_get_log_page(ctrlr, lid, SPDK_NVME_GLOBAL_NS_TAG,
                payload, payloadSize, 0, &CompleteSmartLogPage, NULL))
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

            // sleep 10us to avoid excessive spdk occupancy
            usleep(10);
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

void
SmartCollector::ConvertMaxInt64Value(uint64_t* upper, uint64_t* lower)
{
    bool overMax = false;
    overMax = (*upper & SIGN_BIT_INT_64_MASK) ? true : false;
    if (!overMax)
    {
        *upper <<= 1;
        if (*lower & SIGN_BIT_INT_64_MASK)
        {
            *lower &= ~SIGN_BIT_INT_64_MASK;
            *upper |= 1;
        }
        overMax = (*upper & SIGN_BIT_INT_64_MASK) ? true : false;
    }
    if (overMax)
    {
        *lower = INT_64_MAX_VALUE;
        *upper = INT_64_MAX_VALUE;
    }
}
} // namespace pos
