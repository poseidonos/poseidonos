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

#include "src/metafs/storage/mss.h"

#include <gtest/gtest.h>

namespace pos
{
class MssTester : public MetaStorageSubsystem
{
public:
    explicit MssTester(int arrayId)
    : MetaStorageSubsystem(arrayId)
    {
    }
    ~MssTester(void)
    {
    }
    POS_EVENT_ID CreateMetaStore(int arrayId, MetaStorageType mediaType, uint64_t capacity, bool formatFlag = false)
    {
        return EID(SUCCESS);
    }
    POS_EVENT_ID Open(void)
    {
        return EID(SUCCESS);
    }
    POS_EVENT_ID Close(void)
    {
        return EID(SUCCESS);
    }
    uint64_t GetCapacity(MetaStorageType mediaType)
    {
        return 1024;
    }
    POS_EVENT_ID ReadPage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
    {
        return EID(MFS_ARRAY_ADD_FAILED);
    }
    POS_EVENT_ID WritePage(MetaStorageType mediaType, MetaLpnType metaLpn, void* buffer, MetaLpnType numPages)
    {
        return EID(MFS_ARRAY_DELETE_FAILED);
    }
    bool IsAIOSupport(void)
    {
        return true;
    }
    POS_EVENT_ID ReadPageAsync(MssAioCbCxt* cb)
    {
        return EID(MFS_SYSTEM_MOUNT_AGAIN);
    }
    POS_EVENT_ID WritePageAsync(MssAioCbCxt* cb)
    {
        return EID(MFS_SYSTEM_OPEN_FAILED);
    }
    POS_EVENT_ID TrimFileData(MetaStorageType mediaType, MetaLpnType startLpn, void* buffer, MetaLpnType numPages)
    {
        return EID(MFS_ARRAY_CREATE_FAILED);
    }
    LogicalBlkAddr TranslateAddress(MetaStorageType type, MetaLpnType theLpn)
    {
        return {0, 0};
    }
};

TEST(MssTester, CheckRead)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Read;
    MetaStorageType mediaType = MetaStorageType::SSD;
    MetaLpnType metaLpn = 0;
    MetaLpnType numPages = 0;
    uint32_t mpio_id = 0;
    uint32_t tagid = 0;

    EXPECT_EQ(mss->DoPageIO(opcode, mediaType, metaLpn, nullptr, numPages,
                mpio_id, tagid), EID(MFS_ARRAY_ADD_FAILED));

    delete mss;
}

TEST(MssTester, CheckWrite)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Write;
    MetaStorageType mediaType = MetaStorageType::SSD;
    MetaLpnType metaLpn = 0;
    MetaLpnType numPages = 0;
    uint32_t mpio_id = 0;
    uint32_t tagid = 0;

    EXPECT_EQ(mss->DoPageIO(opcode, mediaType, metaLpn, nullptr, numPages,
                mpio_id, tagid), EID(MFS_ARRAY_DELETE_FAILED));

    delete mss;
}

TEST(MssTester, CheckTrim)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Trim;
    MetaStorageType mediaType = MetaStorageType::SSD;
    MetaLpnType metaLpn = 0;
    MetaLpnType numPages = 0;
    uint32_t mpio_id = 0;
    uint32_t tagid = 0;

    EXPECT_EQ(mss->DoPageIO(opcode, mediaType, metaLpn, nullptr, numPages,
                mpio_id, tagid), EID(MFS_ARRAY_CREATE_FAILED));

    delete mss;
}

TEST(MssTester, CheckRead_Async)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Read;

    EXPECT_EQ(mss->DoPageIOAsync(opcode, nullptr), EID(MFS_SYSTEM_MOUNT_AGAIN));

    delete mss;
}

TEST(MssTester, CheckWrite_Async)
{
    MssTester* mss = new MssTester(0);

    MssOpcode opcode = MssOpcode::Write;

    EXPECT_EQ(mss->DoPageIOAsync(opcode, nullptr), EID(MFS_SYSTEM_OPEN_FAILED));

    delete mss;
}

TEST(MssTester, CheckTranslate)
{
    MssTester* mss = new MssTester(0);

    EXPECT_EQ(mss->TranslateAddress(MetaStorageType::SSD, 0).stripeId, 0);
    EXPECT_EQ(mss->TranslateAddress(MetaStorageType::SSD, 0).offset, 0);

    delete mss;
}

TEST(MssTester, CheckCapacity)
{
    MssTester* mss = new MssTester(0);

    EXPECT_EQ(mss->GetCapacity(MetaStorageType::SSD), 1024);

    delete mss;
}
} // namespace pos
