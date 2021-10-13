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

#pragma once

#include "meta_region_map_info.h"
#include "meta_vol_basic_info.h"
#include "mf_inode.h"
#include "on_volume_meta_region.h"
#include "os_header.h"

namespace pos
{
// Meta volume catalog contains baseline information of corresponding volume which are base lpn and size of each regions
// the contents of this class are buffered in a particular memory space directly, hence please do not define any STL variables here
class CatalogContent : public MetaRegionContent
{
public:
    uint64_t signature;
    VolumeBasicInfo volumeInfo;
    MetaRegionMap regionMap[(int)MetaRegionType::Max];
};

class Catalog : public OnVolumeMetaRegion<MetaRegionType, CatalogContent>
{
public:
    explicit Catalog(MetaVolumeType volumeType, MetaLpnType baseLpn);
    virtual ~Catalog(void);

    virtual void Create(MetaLpnType maxVolumeLpn, uint32_t maxFileNumSupport);
    virtual void RegisterRegionInfo(MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn);

    virtual bool CheckValidity(void);

protected:
    friend class CatalogManager;

    void _InitVolumeRegionInfo(MetaLpnType maxVolumeLpn, uint32_t maxFileNumSupport);

    static const uint64_t VOLUME_CATALOG_SIGNATURE = 0x1B0F198502041B0F;
};
} // namespace pos
