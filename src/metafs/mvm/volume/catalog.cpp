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

#include "catalog.h"

#include "metafs_config.h"
#include "src/logger/logger.h"

namespace pos
{
Catalog::Catalog(MetaVolumeType volumeType, MetaLpnType baseLpn)
: OnVolumeMetaRegion<MetaRegionType, CatalogContent>(volumeType, MetaRegionType::VolCatalog, baseLpn)
{
}

Catalog::~Catalog(void)
{
    delete content;
}

void
Catalog::Create(MetaLpnType maxVolumeLpn, uint32_t maxFileNumSupport)
{
    ResetContent();

    _InitVolumeRegionInfo(maxVolumeLpn, maxFileNumSupport);

    content->signature = VOLUME_CATALOG_SIGNATURE;

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Volume catalog has been initialized...");
}

void
Catalog::_InitVolumeRegionInfo(MetaLpnType maxVolumeLpn, uint32_t maxFileNumSupport)
{
    content->volumeInfo.maxVolPageNum = maxVolumeLpn;
    content->volumeInfo.maxFileNumSupport = maxFileNumSupport;
}

void
Catalog::RegisterRegionInfo(MetaRegionType regionType, MetaLpnType baseLpn, MetaLpnType maxLpn)
{
    MetaRegionMap regionMap(baseLpn, maxLpn);

    content->regionMap[(int)regionType] = regionMap;

    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Volume information regiesterd: <regionType={}, {}, {}>",
        (int)regionType, baseLpn, maxLpn);
}

bool
Catalog::CheckValidity(void)
{
    if (Catalog::VOLUME_CATALOG_SIGNATURE == content->signature)
        return true;

    MFS_TRACE_ERROR(EID(MFS_META_VOLUME_CATALOG_INVALID),
        "The signature({}) in the buffer does not match for VOLUME_CATALOG_SIGNATURE.",
        content->signature);
    return false;
}
} // namespace pos
