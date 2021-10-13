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

#include "mfs_wbt_test.h"

#include "metafs_wbt_api.h"

namespace pos
{
TEST_F(UtMetaFsWBT, TestAPI_GetMetaFileList)
{
    std::string fileName = std::string("wbt_" + std::to_string(GetTimestampUs()));
    size_t fileSize = MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * 50;
    CreateFileAndOpen(fileName, fileSize);

    MetaFsReturnCode<MetaFsStatusCodeWBTSpcf, std::vector<MetaFileInfoDumpCxt>> rc;
    rc = metaFs.wbt.GetMetaFileList();

    for (auto it = rc.returnData.begin(); it != rc.returnData.end(); it++)
    {
        std::cout << "-------------------------" << std::endl;
        MetaFileInfoDumpCxt& data = *it;
        std::cout << data.fd << std::endl;
        std::cout << data.fileName << std::endl;
        std::cout << data.size << std::endl;
        std::cout << data.ctime << std::endl;
        std::cout << data.location << std::endl;
    }
}

TEST_F(UtMetaFsWBT, GetMetaFileInode)
{
}
} // namespace pos
