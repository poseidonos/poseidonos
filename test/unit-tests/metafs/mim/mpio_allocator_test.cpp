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

#include "src/metafs/mim/mpio_allocator.h"

#include <gtest/gtest.h>

#include <unordered_set>

#include "test/unit-tests/metafs/config/metafs_config_manager_mock.h"

using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class MpioAllocatorFixture : public ::testing::Test
{
public:
    MpioAllocatorFixture(void)
    {
        config = new NiceMock<MockMetaFsConfigManager>(nullptr);
        EXPECT_CALL(*config, GetMpioPoolCapacity).WillRepeatedly(Return(COUNT));
        EXPECT_CALL(*config, GetWriteMpioCacheCapacity).WillRepeatedly(Return(COUNT));
        EXPECT_CALL(*config, IsDirectAccessEnabled).WillRepeatedly(Return(false));

        allocator = new MpioAllocator(config);
    }

    virtual ~MpioAllocatorFixture()
    {
        delete allocator;
        delete config;
    }

    virtual void SetUp(void) override
    {
    }

    virtual void TearDown(void) override
    {
    }

    virtual void BuildIoInfo(MpioIoInfo& io, const MetaIoOpcode opcode,
        const int arrayId, const MetaLpnType lpn) const
    {
        io.opcode = opcode;
        io.arrayId = arrayId;
        io.metaLpn = lpn;
    }

protected:
    NiceMock<MockMetaFsConfigManager>* config;
    MpioAllocator* allocator;

    const uint32_t COUNT = 10;
    const int arrayId = 0;
    Mpio* mpioList[10] = {
        0,
    };
    Mpio* mpioList_r[10] = {
        0,
    };
    Mpio* mpioList_w[10] = {
        0,
    };
};

TEST_F(MpioAllocatorFixture, AllocAndReleaseForSsd)
{
    EXPECT_EQ(allocator->GetCapacity(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetCapacity(MpioType::Write), COUNT);

    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = allocator->TryAlloc(static_cast<MpioType>(i), MetaStorageType::SSD, index, true, arrayId);
        }

        // check read mpio list
        EXPECT_TRUE(allocator->IsEmpty(static_cast<MpioType>(i)));

        // check free read mpio
        Mpio* temp = allocator->TryAlloc(static_cast<MpioType>(i), MetaStorageType::SSD, COUNT + 1, true, arrayId);
        EXPECT_EQ(temp, nullptr);

        for (uint32_t index = 0; index < COUNT; index++)
        {
            allocator->Release(mpioList[index]);
        }
    }
}

TEST_F(MpioAllocatorFixture, AllocAndReleaseForNvRam)
{
    for (int i = 0; i < static_cast<int>(MpioType::Max); i++)
    {
        for (uint32_t index = 0; index < COUNT; index++)
        {
            mpioList[index] = allocator->TryAlloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, index, false, arrayId);
        }

        // check read mpio list
        EXPECT_TRUE(allocator->IsEmpty(static_cast<MpioType>(i)));

        // check free read mpio
        Mpio* temp = allocator->TryAlloc(static_cast<MpioType>(i), MetaStorageType::NVRAM, COUNT + 1, false, arrayId);
        EXPECT_EQ(temp, nullptr);

        for (uint32_t index = 0; index < COUNT; index++)
        {
            allocator->Release(mpioList[index]);
        }
    }
}

TEST_F(MpioAllocatorFixture, CheckCounter)
{
    EXPECT_EQ(allocator->GetCapacity(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetCapacity(MpioType::Write), COUNT);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Write), COUNT);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Read), 0);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Write), 0);

    uint32_t popCount = 4;

    for (uint32_t index = 0; index < popCount; index++)
    {
        mpioList_r[index] = allocator->TryAlloc(MpioType::Read, MetaStorageType::SSD, index, true, arrayId);
        ASSERT_NE(mpioList_r[index], nullptr);
        mpioList_w[index] = allocator->TryAlloc(MpioType::Write, MetaStorageType::SSD, index, true, arrayId);
        ASSERT_NE(mpioList_w[index], nullptr);
    }

    EXPECT_EQ(allocator->GetCapacity(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetCapacity(MpioType::Write), COUNT);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Read), COUNT - popCount);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Write), COUNT - popCount);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Read), popCount);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Write), popCount);

    for (uint32_t index = 0; index < popCount; index++)
    {
        allocator->Release(mpioList_r[index]);
        allocator->Release(mpioList_w[index]);
    }

    EXPECT_EQ(allocator->GetCapacity(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetCapacity(MpioType::Write), COUNT);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Write), COUNT);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Read), 0);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Write), 0);
}

TEST_F(MpioAllocatorFixture, AllocAndReleaseForNvRamCache)
{
    std::unordered_set<Mpio*> mpioSet;

    EXPECT_EQ(allocator->GetCapacity(MpioType::Read), COUNT);
    EXPECT_EQ(allocator->GetCapacity(MpioType::Write), COUNT);

    for (uint32_t idx = 0; idx < COUNT; idx++)
    {
        Mpio* m = allocator->TryAlloc(MpioType::Write, MetaStorageType::NVRAM, 0, true, arrayId);
        BuildIoInfo(m->io, MetaIoOpcode::Write, arrayId, 0);
        EXPECT_NE(m, nullptr);

        if (mpioSet.find(m) == mpioSet.end())
            mpioSet.insert(m);
    }

    EXPECT_EQ(mpioSet.size(), 1);
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Write), 1);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Write), COUNT - 1);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Read), COUNT);

    for (auto& i : mpioSet)
        allocator->Release(i);
    mpioSet.clear();

    EXPECT_EQ(allocator->GetUsedCount(MpioType::Write), 1);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Write), COUNT - 1);

    allocator->ReleaseAllCache();
    EXPECT_EQ(allocator->GetUsedCount(MpioType::Write), 0);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Write), COUNT);
    EXPECT_EQ(allocator->GetFreeCount(MpioType::Read), COUNT);
}

} // namespace pos
