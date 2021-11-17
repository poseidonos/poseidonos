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

#include "src/metafs/mvm/volume/catalog_manager.h"
#include "test/unit-tests/metafs/mvm/volume/catalog_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::Return;

namespace pos
{
class CatalogManagerFixture : public ::testing::Test
{
public:
    CatalogManagerFixture(void)
    : catalogMgr(nullptr),
      catalog(nullptr)
    {
    }

    virtual ~CatalogManagerFixture(void)
    {
    }

    virtual void
    SetUp(void)
    {
        catalog = new NiceMock<MockCatalog>(volumeType, baseLpn);

        catalogMgr = new CatalogManager(catalog, arrayId);

        catalogMgr->Init(MetaVolumeType::SsdVolume, 0, 100);
    }

    virtual void
    TearDown(void)
    {
        delete catalogMgr;
    }

protected:
    CatalogManager* catalogMgr;

    NiceMock<MockCatalog>* catalog;

    int arrayId = 0;
    MetaVolumeType volumeType = MetaVolumeType::SsdVolume;
    MetaLpnType baseLpn = 0;
};

TEST_F(CatalogManagerFixture, UnusedMethod)
{
    catalogMgr->Bringup();
    catalogMgr->Finalize();
}

TEST_F(CatalogManagerFixture, CreateObject_New)
{
    EXPECT_EQ(catalogMgr->GetRegionSizeInLpn(), 1);
}

TEST_F(CatalogManagerFixture, SaveContent_Positive)
{
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(true));
    EXPECT_CALL(*catalog, Store()).WillOnce(Return(true));

    EXPECT_EQ(catalogMgr->SaveContent(), true);
}

TEST_F(CatalogManagerFixture, SaveContent_Negative)
{
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(true));
    EXPECT_CALL(*catalog, Store()).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->SaveContent(), false);
}

TEST_F(CatalogManagerFixture, BackupContent_Positive)
{
    EXPECT_CALL(*catalog, Store(_, _, _, _)).WillOnce(Return(true));

    EXPECT_EQ(catalogMgr->BackupContent(MetaVolumeType::SsdVolume, 0, 0), true);
}

TEST_F(CatalogManagerFixture, BackupContent_Negative)
{
    EXPECT_CALL(*catalog, Store(_, _, _, _)).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->BackupContent(MetaVolumeType::SsdVolume, 0, 0), false);
}

TEST_F(CatalogManagerFixture, CreateCatalog_Positive)
{
    EXPECT_CALL(*catalog, Create).Times(2);
    EXPECT_CALL(*catalog, RegisterRegionInfo).Times(2);
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(true));
    EXPECT_CALL(*catalog, Store()).WillOnce(Return(true));

    EXPECT_EQ(catalogMgr->CreateCatalog(0, 0, true), true);

    EXPECT_EQ(catalogMgr->CreateCatalog(0, 0, false), true);
}

TEST_F(CatalogManagerFixture, CreateCatalog_Negative)
{
    EXPECT_CALL(*catalog, Create).Times(2);
    EXPECT_CALL(*catalog, RegisterRegionInfo).Times(2);
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(true));
    EXPECT_CALL(*catalog, Store()).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->CreateCatalog(0, 0, true), false);

    EXPECT_EQ(catalogMgr->CreateCatalog(0, 0, false), true);
}

TEST_F(CatalogManagerFixture, LoadCatalog_Positive)
{
    EXPECT_CALL(*catalog, Load()).WillOnce(Return(true));
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(true));

    EXPECT_EQ(catalogMgr->LoadVolCatalog(), true);
}

TEST_F(CatalogManagerFixture, LoadCatalog_Negative0)
{
    EXPECT_CALL(*catalog, Load()).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->LoadVolCatalog(), false);
}

TEST_F(CatalogManagerFixture, LoadCatalog_Negative1)
{
    EXPECT_CALL(*catalog, Load()).WillOnce(Return(true));
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->LoadVolCatalog(), false);
}

TEST_F(CatalogManagerFixture, RestoreContent_Positive)
{
    EXPECT_CALL(*catalog, Load(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(true));

    EXPECT_EQ(catalogMgr->RestoreContent(MetaVolumeType::SsdVolume, 0, 0), true);
}

TEST_F(CatalogManagerFixture, RestoreContent_Negative0)
{
    EXPECT_CALL(*catalog, Load(_, _, _, _)).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->RestoreContent(MetaVolumeType::SsdVolume, 0, 0), false);
}

TEST_F(CatalogManagerFixture, RestoreContent_Negative1)
{
    EXPECT_CALL(*catalog, Load(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*catalog, CheckValidity).WillOnce(Return(false));

    EXPECT_EQ(catalogMgr->RestoreContent(MetaVolumeType::SsdVolume, 0, 0), false);
}
} // namespace pos
