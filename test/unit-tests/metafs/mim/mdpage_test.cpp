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

#include "src/metafs/mim/mdpage.h"

#include <gtest/gtest.h>

#include "src/include/memory.h"
#include "src/metafs/include/metafs_service.h"
#include "test/unit-tests/array_models/interface/i_array_info_mock.h"
#include "test/unit-tests/metafs/include/metafs_mock.h"

namespace pos
{
TEST(MDPage, Mdpage_Normal)
{
    const size_t TEST_PAGE_SIZE = 4096;
    const MetaLpnType lpn = 10;
    const FileDescriptorType fd = 9;

    void* buf = pos::Memory<TEST_PAGE_SIZE>::Alloc(1);
    bool result = false;

    MockIArrayInfo* info = new MockIArrayInfo();
    EXPECT_CALL(*info, GetName()).Times(1);
    EXPECT_CALL(*info, GetIndex()).Times(2);

    MockMetaFs* metaFs = new MockMetaFs(info, false);
    EXPECT_CALL(*metaFs, GetEpochSignature()).Times(2);

    MDPage* page = new MDPage(buf);
    EXPECT_NE(page->GetDataBuf(), nullptr);

    int arrayId = info->GetIndex();
    page->AttachControlInfo();
    page->Make(lpn, fd, arrayId, metaFs->GetEpochSignature());

    uint32_t signature = page->GetMfsSignature();
    EXPECT_NE(signature, 0);

    result = page->CheckFileMismatch(fd);
    EXPECT_EQ(result, true);

    result = page->CheckLpnMismatch(lpn);
    EXPECT_EQ(result, true);

    result = page->CheckValid(arrayId, metaFs->GetEpochSignature());
    EXPECT_EQ(result, true);

    // null check
    page->AttachControlInfo();
    page->ClearCtrlInfo();

    delete metaFs;
    delete info;
    delete page;

    pos::Memory<TEST_PAGE_SIZE>::Free(buf);
}

} // namespace pos
