/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/metafs/include/metafs_service.h"

#include <gtest/gtest.h>
#include <sched.h>

#include "test/unit-tests/metafs/config/metafs_config_manager_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_scheduler_mock.h"
#include "test/unit-tests/metafs/mim/metafs_io_scheduler_factory_mock.h"
#include "test/unit-tests/telemetry/telemetry_client/telemetry_publisher_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MetaFsServiceTester : public MetaFsService
{
public:
    using MetaFsService::MetaFsService;
    void SetNumaId(const uint32_t numaId)
    {
        numaId_.push_back(numaId);
    }

private:
    // private virtual method
    virtual uint32_t _GetNumaId(const uint32_t coreId) override
    {
        if (count > numaId_.size())
        {
            EXPECT_TRUE(false) << "The count of calling this method is exceeded";
            return 0;
        }
        return numaId_[count++];
    }

    std::vector<uint32_t> numaId_;
    size_t count = 0;
};

class MetaFsServiceFixture : public ::testing::Test
{
public:
    MetaFsServiceFixture(void)
    {
    }

    virtual ~MetaFsServiceFixture(void)
    {
    }

    virtual void SetUp(void)
    {
        totalCoreCount = 96;
        CPU_ZERO(&schedSet);
        CPU_ZERO(&mioSet);
        CPU_SET(0, &mioSet);

        for (int i = 0; i < COUNT_OF_SCHEDULER; ++i)
        {
            scheduler[i] = new NiceMock<MockMetaFsIoScheduler>;
            ON_CALL(*scheduler[i], StartThread).WillByDefault(Return());
            ON_CALL(*scheduler[i], ExitThread).WillByDefault(Return());
        }

        config = new NiceMock<MockMetaFsConfigManager>;
        schedulerFactory = new NiceMock<MockMetaFsIoSchedulerFactory>;

        ON_CALL(*config, Init).WillByDefault(Return(true));
        ON_CALL(*config, GetTimeIntervalInMillisecondsForMetric).WillByDefault(Return(0));
        ON_CALL(*config, GetWrrWeight).WillByDefault(Return(weight));

        tp = new NiceMock<TelemetryPublisher>;

        service = new MetaFsServiceTester(config, schedulerFactory, COUNT_OF_SCHEDULER);
    }

    virtual void TearDown(void)
    {
        delete service;
    }

    void SetNumaId(const uint32_t numaId)
    {
        ((MetaFsServiceTester*)service)->SetNumaId(numaId);
    }

protected:
    static const int COUNT_OF_SCHEDULER = 2;
    NiceMock<MockMetaFsIoScheduler>* scheduler[COUNT_OF_SCHEDULER];
    NiceMock<MockMetaFsIoSchedulerFactory>* schedulerFactory;
    NiceMock<MockMetaFsConfigManager>* config;
    NiceMock<TelemetryPublisher>* tp;
    std::vector<int> weight;

    uint32_t totalCoreCount = 96;
    cpu_set_t schedSet;
    cpu_set_t mioSet;

    MetaFsService* service;
};

TEST_F(MetaFsServiceFixture, Initialize_testIfOneSchedulerWithNuma0CanBeCreatedWhenSchedulerCoreSettingIsSetToACoreAndNumaDedicatedSchedulingIsOff)
{
    EXPECT_CALL(*schedulerFactory, CreateMetaFsIoScheduler).WillOnce(Return(scheduler[0]));
    ON_CALL(*config, IsSupportingNumaDedicatedScheduling).WillByDefault(Return(false));

    CPU_SET(1, &schedSet);

    SetNumaId(0);

    service->Initialize(totalCoreCount, schedSet, mioSet, tp);

    size_t countOfSchedulerExpected = 1;
    SchedulerMap schedulers = service->GetScheduler();
    EXPECT_EQ(schedulers.size(), countOfSchedulerExpected);
    EXPECT_NE(schedulers.find(0), schedulers.end());
    EXPECT_EQ(schedulers.find(1), schedulers.end());

    delete scheduler[1];
}

TEST_F(MetaFsServiceFixture, Initialize_testIfOneSchedulerWithNuma1CanBeCreatedWhenSchedulerCoreSettingIsSetToACoreAndNumaDedicatedSchedulingIsOff)
{
    EXPECT_CALL(*schedulerFactory, CreateMetaFsIoScheduler).WillOnce(Return(scheduler[0]));
    ON_CALL(*config, IsSupportingNumaDedicatedScheduling).WillByDefault(Return(false));

    CPU_SET(1, &schedSet);

    SetNumaId(1);

    service->Initialize(totalCoreCount, schedSet, mioSet, tp);

    size_t countOfSchedulerExpected = 1;
    SchedulerMap schedulers = service->GetScheduler();
    EXPECT_EQ(schedulers.size(), countOfSchedulerExpected);
    EXPECT_NE(schedulers.find(0), schedulers.end());
    EXPECT_EQ(schedulers.find(1), schedulers.end());

    delete scheduler[1];
}

TEST_F(MetaFsServiceFixture, Initialize_testIfAnySchedulerCannotBeCreatedWhenSchedulerCoreSettingIsSetToTwoCoresNumaDedicatedSchedulingIsOff)
{
    ON_CALL(*config, IsSupportingNumaDedicatedScheduling).WillByDefault(Return(false));

    CPU_SET(1, &schedSet);
    CPU_SET(2, &schedSet);

    SetNumaId(0);
    SetNumaId(0);

    EXPECT_DEATH(service->Initialize(totalCoreCount, schedSet, mioSet, tp), "");

    delete scheduler[0];
    delete scheduler[1];
}

TEST_F(MetaFsServiceFixture, Initialize_testIfOneSchedulerCanBeCreatedWhenSchedulerCoreSettingIsSetToACoreInTheSameNumaAndNumaDedicatedSchedulingIsOn)
{
    EXPECT_CALL(*schedulerFactory, CreateMetaFsIoScheduler).WillOnce(Return(scheduler[0]));
    ON_CALL(*config, IsSupportingNumaDedicatedScheduling).WillByDefault(Return(true));

    CPU_SET(1, &schedSet);

    SetNumaId(0);

    service->Initialize(totalCoreCount, schedSet, mioSet, tp);

    size_t countOfSchedulerExpected = 1;
    SchedulerMap schedulers = service->GetScheduler();
    EXPECT_EQ(schedulers.size(), countOfSchedulerExpected);
    EXPECT_NE(schedulers.find(0), schedulers.end());
    EXPECT_EQ(schedulers.find(1), schedulers.end());

    delete scheduler[1];
}

TEST_F(MetaFsServiceFixture, Initialize_testIfOneSchedulerCannotBeCreatedMoreThenOneNumaEachNumaWhenNumaDedicatedSchedulingIsOn)
{
    ON_CALL(*config, IsSupportingNumaDedicatedScheduling).WillByDefault(Return(true));

    CPU_SET(1, &schedSet);
    CPU_SET(2, &schedSet);

    SetNumaId(0);
    SetNumaId(0);

    EXPECT_DEATH(service->Initialize(totalCoreCount, schedSet, mioSet, tp), "");

    delete scheduler[0];
    delete scheduler[1];
}

TEST_F(MetaFsServiceFixture, Initialize_testIfOneSchedulerCanBeCreatedEachNumaWhenNumaDedicatedSchedulingIsOn)
{
    EXPECT_CALL(*schedulerFactory, CreateMetaFsIoScheduler).WillOnce(Return(scheduler[0]))
        .WillOnce(Return(scheduler[1]));
    ON_CALL(*config, IsSupportingNumaDedicatedScheduling).WillByDefault(Return(true));

    CPU_SET(1, &schedSet);
    CPU_SET(2, &schedSet);

    SetNumaId(0);
    SetNumaId(1);

    service->Initialize(totalCoreCount, schedSet, mioSet, tp);

    size_t countOfSchedulerExpected = 2;
    SchedulerMap schedulers = service->GetScheduler();
    EXPECT_EQ(schedulers.size(), countOfSchedulerExpected);
    EXPECT_NE(schedulers.find(0), schedulers.end());
    EXPECT_NE(schedulers.find(1), schedulers.end());
}
} // namespace pos
