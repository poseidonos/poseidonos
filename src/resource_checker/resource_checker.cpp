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

#include "resource_checker.h"

#include <stdio.h>
#include <string.h>

#include "src/cpu_affinity/affinity_manager.h"
#include "src/logger/logger.h"
#include "src/resource_checker/environment_checker.h"
#include "src/resource_checker/smart_collector.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"

using namespace std;

namespace pos
{
ResourceChecker::ResourceChecker(void)
{
    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_CONSTRUCTOR_IN), "try constructor execution");

    if (nullptr == affinityManager)
    {
        affinityManager = AffinityManagerSingleton::Instance();
    }

    if (nullptr == envChecker)
    {
        envChecker = new EnvironmentChecker();
    }

    if (nullptr == publisher)
    {
        publisher = new TelemetryPublisher("Resource_Monitoring");
    }

    if (nullptr == telemetryClient)
    {
        telemetryClient = TelemetryClientSingleton::Instance();
        telemetryClient->RegisterPublisher(publisher);
    }

    sleepSecTime = SLEEP_TIME_SEC;
    runningCnt = 0;
    enable = false;

    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_CONSTRUCTOR_OUT), "complete constructor execution");
}

ResourceChecker::~ResourceChecker(void)
{
    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_DESTRUCTOR_IN), "try destructor execution");

    if (nullptr != envChecker)
    {
        POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_DELETE_EVENT_CHECKER),
            "running_count:{}", runningCnt);
        delete envChecker;
        envChecker = nullptr;
    }

    if (nullptr != publisher)
    {
        telemetryClient->DeregisterPublisher(publisher->GetName());
        delete publisher;
        publisher = nullptr;
    }

    if (nullptr != th && th->joinable())
    {
        POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_DELETE_THREAD_IN),
            "Thread_id:{}", std::hash<std::thread::id>{}(th->get_id()));

        th->join();
        delete th;
        th = nullptr;

        POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_DELETE_THREAD_OUT),
            "running_count:{}", runningCnt);
    }

    affinityManager = nullptr;
    telemetryClient = nullptr;

    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_DESTRUCTOR_OUT), "complete destructor execution");
}

uint64_t
ResourceChecker::GetIterationCount(void)
{
    return runningCnt;
}

void
ResourceChecker::SetSleepTime(uint32_t time)
{
    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_SET_SLEEP_TIME),
        "sleep_time_value:{}", time);

    sleepSecTime = time;
}

void
ResourceChecker::Enable(void)
{
    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_ENABLE_THREAD_IN),
        "enable:{}", enable);

    if (false == enable)
    {
        if (nullptr == th)
        {
            th = new std::thread(&ResourceChecker::Execute, this);
            POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_MAKE_THREAD),
                "thread_id:{}", std::hash<std::thread::id>{}(th->get_id()));
            enable = true;
        }
    }

    POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_ENABLE_THREAD_OUT),
        "thread_id:{}, enable:{}",
        std::hash<std::thread::id>{}(th->get_id()), enable);
}

void
ResourceChecker::Execute(void)
{
    POS_TRACE_INFO(EID(RESOURCE_CHECKER_EXECUTE_IN),
        "running_count:{}", runningCnt);

    if (nullptr != affinityManager)
    {
        cpu_set_t cpuSet = affinityManager->GetCpuSet(CoreType::GENERAL_USAGE);
        sched_setaffinity(0, sizeof(cpuSet), &cpuSet);
        pthread_setname_np(pthread_self(), "ResourceChecker");
    }

    uint32_t availableMemorySizeInKByte = envChecker->GetAvailableMemorySize();

    if (nullptr != publisher && nullptr != envChecker)
    {
        while (true)
        {
            if (nullptr != envChecker)
            {
                runningCnt++;
                if (nullptr != publisher)
                {
                    POSMetric metric(TEL100000_RESOURCE_CHECKER_AVAILABLE_MEMORY, POSMetricTypes::MT_GAUGE);
                    metric.AddLabel("Node", "single");
                    metric.SetGaugeValue(static_cast<uint64_t>(availableMemorySizeInKByte));
                    publisher->PublishMetric(metric);
                }

                // sleep 60 sec
                CollectSmartLogPage();
                usleep(sleepSecTime * 1000 * 1000);
            }
            else
            {
                POS_TRACE_DEBUG(EID(RESOURCE_CHECKER_EXECUTE_EVENT_DELETED),
                    "stopped periodic checker");
                break;
            }
        }
    }

    POS_TRACE_INFO(EID(RESOURCE_CHECKER_EXECUTE_OUT),
        "running_count:{}", runningCnt);
}

void
ResourceChecker::CollectSmartLogPage(void)
{
    SmartCollector* smartCollector = SmartCollectorSingleton::Instance();
    smartCollector->PublishSmartDataToTelemetryAllCtrl();
}
} // namespace pos
