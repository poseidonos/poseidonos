/*
 *   BSD LICENSE
 *   Copyright (c) 2023 Samsung Electronics Corporation
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

#include "src/metafs/mvm/volume/active_file_list.h"

#include <gtest/gtest.h>

namespace pos
{
TEST(ActiveFileList, CheckFileInActive)
{
    ActiveFileList activeFiles;
    std::unordered_set<FileDescriptorType>& set = activeFiles.GetActiveFiles();
    set.insert(0);

    EXPECT_EQ(activeFiles.CheckFileInActive(0), true);
    EXPECT_EQ(activeFiles.CheckFileInActive(1), false);
}

TEST(ActiveFileList, CheckAddedFileInActive)
{
    ActiveFileList activeFiles;
    EXPECT_EQ(activeFiles.AddFileInActiveList(0), EID(SUCCESS));
    EXPECT_EQ(activeFiles.AddFileInActiveList(0), EID(MFS_FILE_OPEN_REPETITIONARY));
}

TEST(ActiveFileList, RemoveFileInActive)
{
    ActiveFileList activeFiles;
    std::unordered_set<FileDescriptorType>& set = activeFiles.GetActiveFiles();
    set.insert(0);

    EXPECT_EQ(set.size(), 1);

    activeFiles.RemoveFileFromActiveList(0);
}

TEST(ActiveFileList, GetFileCountInActive)
{
    ActiveFileList activeFiles;
    std::unordered_set<FileDescriptorType>& set = activeFiles.GetActiveFiles();
    set.insert(0);

    EXPECT_EQ(activeFiles.GetFileCountInActive(), 1);

    set.erase(0);

    EXPECT_EQ(activeFiles.GetFileCountInActive(), 0);
}
} // namespace pos
