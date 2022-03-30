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

#include "src/metafs/storage/pstore/mss_on_disk.h"
#include "test/unit-tests/metafs/storage/pstore/mss_disk_inplace_mock.h"

#include <gtest/gtest.h>

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
TEST(MssOnDisk, CheckCapacity_DefaultValue)
{
    int arrayId = 0;
    MssOnDisk* mss = new MssOnDisk(arrayId);

    EXPECT_EQ(mss->GetCapacity(MetaStorageType::SSD), 0);

    delete mss;
}

TEST(MssOnDisk, TranslateAddr_DefaultValue)
{
    int arrayId = 0;
    MssOnDisk* mss = new MssOnDisk(arrayId);
    std::vector<MssDiskPlace*>& map = mss->GetMssDiskPlace();
    MockMssDiskInplace* disk = new MockMssDiskInplace();
    map.clear();
    map.push_back(disk);

    LogicalBlkAddr addr;
    addr.stripeId = 1;
    addr.offset = 2;

    EXPECT_CALL(*disk, CalculateOnDiskAddress)
        .Times(2)
        .WillRepeatedly(Return(addr));

    EXPECT_EQ(mss->TranslateAddress(MetaStorageType::SSD, 0).stripeId, 1);
    EXPECT_EQ(mss->TranslateAddress(MetaStorageType::SSD, 0).offset, 2);

    delete mss;
    // delete disk;
}
} // namespace pos
