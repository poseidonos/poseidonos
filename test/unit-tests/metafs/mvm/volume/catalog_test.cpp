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

#include "src/metafs/mvm/volume/catalog.h"

#include <gtest/gtest.h>
 // namespace pos
namespace pos
{
class CatalogTester : public Catalog
{
public:
    CatalogTester(MetaVolumeType volumeType, MetaLpnType baseLpn)
    : Catalog(volumeType, baseLpn)
    {
    }

    uint64_t GetSignature(void)
    {
        return VOLUME_CATALOG_SIGNATURE;
    }
};

static void CheckAndAssertIfNotEqual(CatalogContent& left, CatalogContent& right)
{   
    // table1 data and newTable data must be same.
    EXPECT_EQ(left.signature, right.signature);
    EXPECT_EQ(left.volumeInfo.maxFileNumSupport, right.volumeInfo.maxFileNumSupport);
    for(int i = 0; i < (int)MetaRegionType::Max; ++i)
    {
        EXPECT_EQ(left.regionMap[i].baseLpn, right.regionMap[i].baseLpn);
        EXPECT_EQ(left.regionMap[i].maxLpn, right.regionMap[i].maxLpn);
    }
}

TEST(Catalog, CreateObject)
{
    MetaLpnType maxVolumeLpn = 1024;
    uint32_t maxFileNumSupport = 30;

    CatalogTester* catalog = new CatalogTester(MetaVolumeType::SsdVolume, 0);
    CatalogContent* content = catalog->GetContent();

    catalog->Create(maxVolumeLpn, maxFileNumSupport);

    EXPECT_EQ(content->volumeInfo.maxVolPageNum, maxVolumeLpn);
    EXPECT_EQ(content->volumeInfo.maxFileNumSupport, maxFileNumSupport);
    EXPECT_EQ(content->signature, catalog->GetSignature());

    EXPECT_TRUE(catalog->CheckValidity());

    delete catalog;
}

TEST(Catalog, RegisterRegion)
{
    MetaLpnType maxVolumeLpn = 1024;
    uint32_t maxFileNumSupport = 30;

    CatalogTester* catalog = new CatalogTester(MetaVolumeType::SsdVolume, 0);
    CatalogContent* content = catalog->GetContent();

    catalog->Create(maxVolumeLpn, maxFileNumSupport);

    for (int i = 0; i < (int)MetaRegionType::Max; ++i)
    {
        catalog->RegisterRegionInfo((MetaRegionType)i, i * 5 + 1, i * 5 + 5);
    }

    for (int i = 0; i < (int)MetaRegionType::Max; ++i)
    {
        EXPECT_EQ(content->regionMap[i].baseLpn, i * 5 + 1);
        EXPECT_EQ(content->regionMap[i].maxLpn,  i * 5 + 5);
    }

    delete catalog;
}

TEST(Catalog, ToBytes_testIfDeserializedObjContainsOriginalData)
{
    MetaLpnType maxVolumeLpn = 1024;
    uint32_t maxFileNumSupport = 30;

    CatalogTester* catalog = new CatalogTester(MetaVolumeType::SsdVolume, 0);
    CatalogContent* content = catalog->GetContent();

    catalog->Create(maxVolumeLpn, maxFileNumSupport);

    for (int i = 0; i < (int)MetaRegionType::Max; ++i)
    {
        catalog->RegisterRegionInfo((MetaRegionType)i, i * 5 + 1, i * 5 + 5);
    }

    int protoBufSize = CatalogContentOnSsdSize;
    char ioBuffer[protoBufSize];

    // Write Catalog data to buffer.
    content->ToBytes(ioBuffer);

    CatalogTester* newCatalog = new CatalogTester(MetaVolumeType::SsdVolume, 0);
    CatalogContent* newContent = newCatalog->GetContent();
    
    // Read Catalog data from buffer.
    newContent->FromBytes(ioBuffer);

    // Catalog content and newContent data must be same. 
    CheckAndAssertIfNotEqual(*content, *newContent);

    delete catalog;
    delete newCatalog;
}

} // namespace pos
