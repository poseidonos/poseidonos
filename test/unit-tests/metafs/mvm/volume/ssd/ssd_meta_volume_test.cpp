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

#include "src/metafs/mvm/volume/ssd/ssd_meta_volume.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "test/unit-tests/metafs/mvm/volume/catalog_manager_mock.h"
#include "test/unit-tests/metafs/mvm/volume/inode_manager_mock.h"
#include "test/unit-tests/metafs/storage/mss_mock.h"

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class SsdMetaVolumeTestFixture : public ::testing::Test
{
public:
    SsdMetaVolumeTestFixture(void)
    : inodeMgr(nullptr),
      catalogMgr(nullptr)
    {
    }

    virtual ~SsdMetaVolumeTestFixture()
    {
    }

    virtual void
    SetUp(void)
    {
        inodeMgr = new NiceMock<InodeManager>(arrayId);
        catalogMgr = new NiceMock<CatalogManager>(arrayId);

        volume = new SsdMetaVolume(arrayId, MetaVolumeType::SsdVolume, maxLpn, inodeMgr, catalogMgr);
        volume->Init(metaStorage);
        volume->InitVolumeBaseLpn();
    }

    virtual void
    TearDown(void)
    {
        delete volume;
    }

protected:
    SsdMetaVolume* volume;

    NiceMock<InodeManager>* inodeMgr;
    NiceMock<CatalogManager>* catalogMgr;

    int arrayId = 0;
    MetaLpnType maxLpn = 8192;
    MockMetaStorageSubsystem* metaStorage;
    MetaFilePropertySet prop;
};

TEST_F(SsdMetaVolumeTestFixture, SSD_Meta_Volume_Normal)
{
    uint64_t chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;

    // then
    // the maximum size
    EXPECT_TRUE(volume->IsOkayToStore(4087 * chunkSize, prop));
    // expected count of the free lpn: 4088
    EXPECT_TRUE(volume->IsOkayToStore(4088 * chunkSize, prop));
    // more than possible
    EXPECT_FALSE(volume->IsOkayToStore(4089 * chunkSize, prop));
}

TEST(SsdMetaVolume, CreateDefault)
{
    SsdMetaVolume* metaVolume = new SsdMetaVolume(0, MetaVolumeType::SsdVolume, 0);
    delete metaVolume;
}

TEST(SsdMetaVolume, IsOkayToStore_Negative)
{
    NiceMock<MockInodeManager>* inodeMgr = new NiceMock<MockInodeManager>(0);
    NiceMock<MockCatalogManager>* catalogMgr = new NiceMock<MockCatalogManager>(0);
    SsdMetaVolume* volume = new SsdMetaVolume(0, MetaVolumeType::SsdVolume, 8192, inodeMgr, catalogMgr);
    uint64_t chunkSize = MetaFsIoConfig::DEFAULT_META_PAGE_DATA_CHUNK_SIZE;
    MetaFilePropertySet prop;

    EXPECT_CALL(*inodeMgr, GetAvailableLpnCount).WillOnce(Return(8192));

    // not enough space
    EXPECT_FALSE(volume->IsOkayToStore(4087 * chunkSize, prop));

    delete volume;
}

} // namespace pos
