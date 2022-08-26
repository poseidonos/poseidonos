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

#include "src/metafs/mai/metafs_wbt_api.h"
#include "src/metafs/include/meta_volume_type.h"

#include <string>

#include "test/integration-tests/metafs/lib/metafs_test_fixture.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Matcher;
using ::testing::Return;
using ::testing::ReturnRef;

using namespace std;

namespace pos
{
class MetaFsWbtIntegrationTest : public MetaFsTestFixture, public ::testing::Test
{
public:
    MetaFsWbtIntegrationTest(void)
    : MetaFsTestFixture(),
      arrayId(0),
      isNpor(true),
      fileName("TestFile"),
      fileSize(2048),
      volumeType(MetaVolumeType::SsdVolume)
    {
    }

    virtual ~MetaFsWbtIntegrationTest(void)
    {
    }

    virtual void SetUp(void)
    {
        // mount array
        EXPECT_EQ(0, GetMetaFs(arrayId)->Init());

        // create meta file
        POS_EVENT_ID rc_mgmt;
        MetaFilePropertySet prop;
        rc_mgmt = GetMetaFs(arrayId)->ctrl->Create(fileName, fileSize, prop, volumeType);
        EXPECT_EQ(rc_mgmt, EID(SUCCESS));
    }

    virtual void TearDown(void)
    {
        // unmount array
        GetMetaFs(arrayId)->Dispose();
    }

protected:
    int arrayId;
    bool isNpor;
    std::string fileName;
    uint64_t fileSize;
    MetaVolumeType volumeType;
};

TEST_F(MetaFsWbtIntegrationTest, GetTheListOfMetaFilesInTheArray)
{
    std::vector<pos::MetaFileInfoDumpCxt> fileList;

    // positive
    EXPECT_EQ(true, GetMetaFs(arrayId)->wbt->GetMetaFileList(fileList, MetaVolumeType::SsdVolume));

    EXPECT_EQ(1, fileList.size());

    for (auto item : fileList)
    {
        EXPECT_EQ(item.fileName, fileName);
        EXPECT_EQ(item.size, fileSize);
    }

    // negative
    EXPECT_EQ(false, GetMetaFs(arrayId)->wbt->GetMetaFileList(fileList, MetaVolumeType::NvRamVolume));
}

TEST_F(MetaFsWbtIntegrationTest, GetMetaFileInfo)
{
    pos::MetaFileInodeDumpCxt fileInfo;

    // positive
    EXPECT_EQ(true, GetMetaFs(arrayId)->wbt->GetMetaFileInode(fileName, fileInfo, MetaVolumeType::SsdVolume));

    EXPECT_EQ(fileInfo.inodeInfo.data.field.fileName, fileName);
    EXPECT_EQ(fileInfo.inodeInfo.data.field.fileByteSize, fileSize);

    // negative
    EXPECT_EQ(false, GetMetaFs(arrayId)->wbt->GetMetaFileInode(fileName, fileInfo, MetaVolumeType::NvRamVolume));
}
} // namespace pos
