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

#include <string>
#include "meta_region_content.h"
#include "metafs_config.h"
#include "metafs_log.h"
#include "src/metafs/storage/mss.h"

namespace pos
{
template<typename MetaRegionT, typename MetaContentT>
class MetaRegion
{
public:
    MetaRegion(void);     // Ctor for UT code
    MetaRegion(MetaStorageType mediaType, MetaRegionT regionType, MetaLpnType baseLpn, uint32_t mirrorCount = 0);
    virtual ~MetaRegion(void);
    virtual MetaContentT* GetContent(void);
    virtual void SetContent(MetaContentT* content_);
    virtual const size_t GetSizeOfContent(void);
    virtual const MetaLpnType GetBaseLpn(void);
    virtual void* GetDataBuf(void);
    virtual void* GetDataBuf(MetaLpnType pageOffset);
    virtual const MetaLpnType GetLpnCntOfRegion(void);
    virtual void ResetContent(void);
    virtual void SetMss(MetaStorageSubsystem* mss);
    virtual bool Load(void);
    virtual bool Load(MetaStorageType targetMedia, MetaLpnType baseLPN, uint32_t idx, MetaLpnType pageCNT);

    virtual bool Store(void);
    virtual bool Store(MetaStorageType targetMedia, MetaLpnType startLPN, uint32_t idx, MetaLpnType pageCNT);

    virtual const MetaLpnType GetLpnCntOfContent(void);

protected:
    MetaContentT* content;
    MetaRegionT regionType;

private:
    MetaStorageType mediaType;
    MetaLpnType startLpn;
    MetaLpnType totalLpnCnt;
    MetaLpnType mirrorCount;

    MetaStorageSubsystem* mssIntf;
};

template<typename MetaRegionT, typename MetaContentT>
MetaRegion<MetaRegionT, MetaContentT>::MetaRegion(void)
: content(nullptr)
{
}

template<typename MetaRegionT, typename MetaContentT>
MetaRegion<MetaRegionT, MetaContentT>::MetaRegion(MetaStorageType mediaType, MetaRegionT regionType, MetaLpnType baseLpn, uint32_t mirrorCount)
: content(new (GetLpnCntOfContent()) MetaContentT),
  regionType(regionType),
  mediaType(mediaType),
  startLpn(baseLpn),
  totalLpnCnt(GetLpnCntOfContent()),
  mirrorCount(mirrorCount),
  mssIntf(nullptr)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "MetaRegion(constructed): media={}, region={}, sizeof={}, baseLpn={}, lpn count={}",
        (int)mediaType, (int)regionType, GetSizeOfContent(),
        baseLpn, GetLpnCntOfContent());
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
MetaRegion<MetaRegionT, MetaContentT>::~MetaRegion(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "MetaRegion(destructed): media={}, region={}",
        (int)mediaType, (int)regionType);
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
MetaContentT*
MetaRegion<MetaRegionT, MetaContentT>::GetContent(void)
{
    return content;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void
MetaRegion<MetaRegionT, MetaContentT>::SetContent(MetaContentT* content_)
{
    content = content_;
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
const size_t
MetaRegion<MetaRegionT, MetaContentT>::GetSizeOfContent(void)
{
    return sizeof(MetaContentT);
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType
MetaRegion<MetaRegionT, MetaContentT>::GetBaseLpn(void)
{
    return startLpn;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void*
MetaRegion<MetaRegionT, MetaContentT>::GetDataBuf(void)
{
    return content;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void*
MetaRegion<MetaRegionT, MetaContentT>::GetDataBuf(MetaLpnType pageOffset)
{
    uint8_t* buf = (uint8_t*)GetDataBuf() + (MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * pageOffset);
    return buf;
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType
MetaRegion<MetaRegionT, MetaContentT>::GetLpnCntOfRegion(void)
{
    return totalLpnCnt;
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void
MetaRegion<MetaRegionT, MetaContentT>::ResetContent(void)
{
    memset((void*)content, 0, totalLpnCnt * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Reset all content...");
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
void
MetaRegion<MetaRegionT, MetaContentT>::SetMss(MetaStorageSubsystem* mss)
{
    mssIntf = mss;
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegion<MetaRegionT, MetaContentT>::Load(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta load <mediaType, startLpn, totalLpn>={}, {}, {}",
        (int)mediaType, startLpn, totalLpnCnt);

    // The Data in GetDataBuf() is contents of each regions in the  mediaType.
    POS_EVENT_ID rc = mssIntf->ReadPage(mediaType, startLpn, GetDataBuf(), totalLpnCnt);
    return (rc == EID(SUCCESS)) ? true : false;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegion<MetaRegionT, MetaContentT>::Load(MetaStorageType media, MetaLpnType baseLPN, uint32_t idx, MetaLpnType pageCNT)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta load<mediaTyp, startLpn, totalLpn>={}, {}, {},",
        (int)media, baseLPN, pageCNT);

    POS_EVENT_ID rc = mssIntf->ReadPage(media, baseLPN + idx, GetDataBuf(idx), pageCNT);
    return (rc == EID(SUCCESS)) ? true : false;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegion<MetaRegionT, MetaContentT>::Store(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta store <mediaType, startLpn, totalLpn>={}, {}, {}",
        (int)mediaType, startLpn, totalLpnCnt);
    POS_EVENT_ID rc = mssIntf->WritePage(mediaType, startLpn, GetDataBuf(), totalLpnCnt);
    return (rc == EID(SUCCESS)) ? true : false;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegion<MetaRegionT, MetaContentT>::Store(MetaStorageType media, MetaLpnType baseLPN, uint32_t idx, MetaLpnType pageCNT)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta store <mediaTyp, startLpn, totalLpn>={}, {}, {},",
        (int)media, baseLPN, pageCNT);

    POS_EVENT_ID rc = mssIntf->WritePage(media, baseLPN + idx, GetDataBuf(idx), pageCNT);
    return (rc == EID(SUCCESS)) ? true : false;
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType
MetaRegion<MetaRegionT, MetaContentT>::GetLpnCntOfContent(void)
{
    size_t contentSize = GetSizeOfContent();
    MetaLpnType lpnCntOfContent = (contentSize + MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES - 1) / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    return lpnCntOfContent;
}
} // namespace pos
