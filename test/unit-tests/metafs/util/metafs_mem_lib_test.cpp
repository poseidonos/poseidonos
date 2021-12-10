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

#include "src/metafs/util/metafs_mem_lib.h"

#include <gtest/gtest.h>

namespace pos
{
class MetaFsMemLibTester
{
public:
    MetaFsMemLibTester(void)
    {
    }

    static void CallbackTest(void* obj)
    {
    }
};

TEST(MetaFsMemLib, CheckAvailable)
{
    EXPECT_FALSE(MetaFsMemLib::IsResourceAvailable());
}

TEST(MetaFsMemLib, SetResourceAvailable)
{
    MetaFsMemLib::EnableResourceUse();
    EXPECT_TRUE(MetaFsMemLib::IsResourceAvailable());
}

TEST(MetaFsMemLib, MemoryCopyAsync)
{
    MetaFsMemLibTester tester;
    MetaFsMemLib::EnableResourceUse();

    uint64_t src = 0x12121212F0F0F0F0;
    uint64_t dst = 0;

    MetaFsMemLib::MemCpyAsync((void*)&dst, (void*)&src, sizeof(src), &(tester.CallbackTest), &tester);

#if (1 == METAFS_INTEL_IOAT_EN)
    EXPECT_EQ(src, dst);
#else
    EXPECT_NE(src, dst);
#endif
}

TEST(MetaFsMemLib, MemorySetZero)
{
    MetaFsMemLibTester tester;
    MetaFsMemLib::EnableResourceUse();

    uint64_t src = 0x12121212F0F0F0F0;

    MetaFsMemLib::MemSetZero((void*)&src, sizeof(src), &(tester.CallbackTest), &tester);

#if (1 == METAFS_INTEL_IOAT_EN)
    EXPECT_EQ(src, 0);
#else
    EXPECT_NE(src, 0);
#endif
}
} // namespace pos
