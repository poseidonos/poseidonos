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
#include "meta_region_proto_content.h"
#include "metafs_config.h"
#include "metafs_log.h"
#include "src/metafs/storage/mss.h"

namespace pos
{
template<typename MetaRegionT, typename MetaContentT>
class MetaRegionProto
{
public:
    MetaRegionProto(void);     // Ctor for UT code
    MetaRegionProto(MetaStorageType mediaType, MetaRegionT regionType, MetaLpnType baseLpn, uint32_t mirrorCount = 0);
    virtual ~MetaRegionProto(void);
    virtual MetaContentT* GetContent(void);
    virtual void SetContent(MetaContentT* content_);
    virtual const size_t GetSizeOfContent(void);
    virtual const MetaLpnType GetBaseLpn(void);
    virtual void* GetDataBuf(void);
    virtual void* GetDataBuf(MetaLpnType pageOffset);
    virtual void* GetProtoBuf(MetaLpnType pageOffset);
    virtual const MetaLpnType GetLpnCntOfRegion(void);
    virtual void ResetContent(void);
    virtual void SetMss(MetaStorageSubsystem* mss);
    virtual bool Load(void);
    virtual bool Load(MetaStorageType targetMedia, MetaLpnType baseLPN, uint32_t idx, MetaLpnType pageCNT);

    virtual bool Store(void);
    virtual bool Store(MetaStorageType targetMedia, MetaLpnType startLPN, uint32_t idx, MetaLpnType pageCNT);

    virtual const MetaLpnType GetLpnCntOfContent(void);
    virtual const MetaLpnType GetOnSsdLpnCntOfContent(void);

protected:
    MetaContentT* content;
    MetaRegionT regionType;
    MetaContentT* protoBuf;

private:
    MetaStorageType mediaType;
    MetaLpnType startLpn;
    MetaLpnType totalLpnCnt;
    MetaLpnType mirrorCount;

    MetaStorageSubsystem* mssIntf;
};

template<typename MetaRegionT, typename MetaContentT>
MetaRegionProto<MetaRegionT, MetaContentT>::MetaRegionProto(void)
: content(nullptr)
{
}

template<typename MetaRegionT, typename MetaContentT>
MetaRegionProto<MetaRegionT, MetaContentT>::MetaRegionProto(MetaStorageType mediaType, MetaRegionT regionType, MetaLpnType baseLpn, uint32_t mirrorCount)
: content(new (GetLpnCntOfContent()) MetaContentT),
  regionType(regionType),
  protoBuf(new (GetLpnCntOfContent()) MetaContentT),
  mediaType(mediaType),
  startLpn(baseLpn),
  totalLpnCnt(GetLpnCntOfContent()),
  mirrorCount(mirrorCount),
  mssIntf(nullptr)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "MetaRegionProto(constructed): media={}, region={}, sizeof={}, baseLpn={}, lpn count={}",
        (int)mediaType, (int)regionType, GetSizeOfContent(),
        baseLpn, GetLpnCntOfContent());
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
MetaRegionProto<MetaRegionT, MetaContentT>::~MetaRegionProto(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "MetaRegionProto(destructed): media={}, region={}",
        (int)mediaType, (int)regionType);
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
MetaContentT*
MetaRegionProto<MetaRegionT, MetaContentT>::GetContent(void)
{
    return content;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void
MetaRegionProto<MetaRegionT, MetaContentT>::SetContent(MetaContentT* content_)
{
    content = content_;
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
const size_t
MetaRegionProto<MetaRegionT, MetaContentT>::GetSizeOfContent(void)
{
    return content->GetOnSsdSize();
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType
MetaRegionProto<MetaRegionT, MetaContentT>::GetBaseLpn(void)
{
    return startLpn;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void*
MetaRegionProto<MetaRegionT, MetaContentT>::GetDataBuf(void) 
{
    return content;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void*
MetaRegionProto<MetaRegionT, MetaContentT>::GetDataBuf(MetaLpnType pageOffset)
{
    uint8_t* buf = (uint8_t*)GetDataBuf() + (MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * pageOffset);
    return buf;
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
void* 
MetaRegionProto<MetaRegionT, MetaContentT>::GetProtoBuf(MetaLpnType pageOffset)
{
    uint8_t* buf = (uint8_t*)protoBuf + (MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES * pageOffset);
    return buf;
}

template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType
MetaRegionProto<MetaRegionT, MetaContentT>::GetLpnCntOfRegion(void)
{
    return totalLpnCnt;
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
void
MetaRegionProto<MetaRegionT, MetaContentT>::ResetContent(void)
{
    memset((void*)content, 0, totalLpnCnt * MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Reset all content...");
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
void
MetaRegionProto<MetaRegionT, MetaContentT>::SetMss(MetaStorageSubsystem* mss)
{
    mssIntf = mss;
}

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegionProto<MetaRegionT, MetaContentT>::Load(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta load <mediaType, startLpn, totalLpn>={}, {}, {}",
        (int)mediaType, startLpn, totalLpnCnt);
    
    POS_EVENT_ID rc = mssIntf->ReadPage(mediaType, startLpn, protoBuf, GetOnSsdLpnCntOfContent()); 
    if(rc == EID(SUCCESS)){
        content->FromBytes((char*)protoBuf);
        return true;
    }else{
        return false;
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegionProto<MetaRegionT, MetaContentT>::Load(MetaStorageType media, MetaLpnType baseLPN, uint32_t idx, MetaLpnType pageCNT)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta load by buf index <mediaType, startLpn, totalLpn>={}, {}, {}, and buf idx : {}",
        (int)media, baseLPN + idx, pageCNT, idx);

    POS_EVENT_ID rc = mssIntf->ReadPage(media, baseLPN + idx, GetProtoBuf(idx), pageCNT);
    if(rc == EID(SUCCESS)){
        int pageEntryIndex = idx / (content->GetSizeOfEntry() / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
        content->FromBytesByIndex((char*)protoBuf, pageEntryIndex /* This value is valid only for InodeTableContent */);
        return true;
    }else{
        return false;
    }
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegionProto<MetaRegionT, MetaContentT>::Store(void)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta store <mediaType, startLpn, totalLpn>={}, {}, {}",
        (int)mediaType, startLpn, totalLpnCnt);

    content->ToBytes((char*)protoBuf);
    POS_EVENT_ID rc = mssIntf->WritePage(mediaType, startLpn, protoBuf, GetOnSsdLpnCntOfContent());
    return (rc == EID(SUCCESS)) ? true : false;
}
// LCOV_EXCL_STOP

// LCOV_EXCL_START
template<typename MetaRegionT, typename MetaContentT>
bool
MetaRegionProto<MetaRegionT, MetaContentT>::Store(MetaStorageType media, MetaLpnType baseLPN, uint32_t idx, MetaLpnType pageCNT)
{
    MFS_TRACE_DEBUG(EID(MFS_DEBUG_MESSAGE),
        "Do meta store by buf index <mediaType, startLpn, totalLpn>={}, {}, {}, and buf LPN idx : {} ",
        (int)media, baseLPN + idx, pageCNT, idx);
    int pageEntryIndex = idx / (content->GetSizeOfEntry() / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES);
    content->ToBytesByIndex((char*)protoBuf, pageEntryIndex /* This value is valid only for InodeTableContent */);
    POS_EVENT_ID rc = mssIntf->WritePage(media, baseLPN + idx, GetProtoBuf(idx), pageCNT); 
    return (rc == EID(SUCCESS)) ? true : false;
}
// LCOV_EXCL_STOP

template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType
MetaRegionProto<MetaRegionT, MetaContentT>::GetLpnCntOfContent(void)
{
    size_t contentSize = GetSizeOfContent(); //ONSSD_SIZE
    MetaLpnType lpnCntOfContent = (contentSize + MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES - 1) / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    return lpnCntOfContent;
}

template<typename MetaRegionT, typename MetaContentT>
const MetaLpnType 
MetaRegionProto<MetaRegionT, MetaContentT>::GetOnSsdLpnCntOfContent(void)
{ 
    size_t contentSize = content->GetOnSsdSize();
    MetaLpnType lpnCntOfContent = (contentSize + MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES - 1) / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;
    return lpnCntOfContent;
}


} // namespace pos
