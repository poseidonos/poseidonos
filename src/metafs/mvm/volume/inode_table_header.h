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

#include <array>
#include <queue>
#include <vector>

#include "file_descriptor_in_use_map.h"
#include "metafs_common.h"

#include "src/metafs/common/meta_region_content.h"
#include "on_volume_meta_region_proto.h"
#include "src/metafs/storage/mss.h"

#include "src/metafs/include/meta_file_extent.h"
#include "proto/generated/cpp/pos_bc.pb.h"


namespace pos
{
class InodeInUseBitmap
{
public:
    InodeInUseBitmap&
    operator=(const InodeInUseBitmap& src)
    {
        bits = src.bits;
        return *this;
    }

    std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> bits;
    uint32_t allocatedInodeCnt;
};

// The fixed size for a single InodeTableHeaderContent
const int InodeTableHeaderOnSsdSize = 12288; // == 3LPN
class InodeTableHeaderContent : public MetaRegionProtoContent
{
public:
    uint32_t totalInodeNum;
    size_t inodeEntryByteSize;
    uint32_t totalFileCreated;
    InodeInUseBitmap inodeInUseBitmap;
    std::array<MetaFileExtent, MetaFsConfig::MAX_VOLUME_CNT> allocExtentsList;
    
    void ToBytes(char* destBuf)
    {
        pos_bc::InodeTableHeaderContentProto proto; 
        
        pos_bc::InodeInUseBitmap inodeInUseBitmapProto;
        inodeInUseBitmapProto.set_bits(this->inodeInUseBitmap.bits.to_string());
        inodeInUseBitmapProto.set_allocatedinodecnt(this->inodeInUseBitmap.allocatedInodeCnt);    
        
        proto.set_totalinodenum(this->totalInodeNum);
        proto.set_inodeentrybytesize(this->inodeEntryByteSize);
        proto.set_totalfilecreated(this->totalFileCreated);
        *proto.mutable_inodeinusebitmap() = inodeInUseBitmapProto;

        for (int i = 0 ;i < (int)this->allocExtentsList.size(); ++i)
        {
            pos_bc::MetaFileExtent* metaFileExtentProto = proto.add_allocextentslist();
            metaFileExtentProto->set_startlpn(this->allocExtentsList[i].GetStartLpn());
            metaFileExtentProto->set_count(this->allocExtentsList[i].GetCount());
        }
    
        size_t effectiveSize = proto.ByteSizeLong();
        for(unsigned int bytePos = effectiveSize; bytePos < InodeTableHeaderOnSsdSize; bytePos ++)
        {
            destBuf[bytePos] = 0;
        }
        
        int ret = proto.SerializeToArray(destBuf, InodeTableHeaderOnSsdSize);    
        if(ret==0){
            // ret == 0 means ParseFromArray ran into an error. It's because we have passed in 
            // a fixed-size length, i.e., ONSSD_SIZE, regardless of the actual message size.
            // The current implementation has a risk of not detecting a parse error when there has been
            // real data corruption, but such data corruption would have put the system in an irrecoverable state anyway, 
            // so I'd keep the current impl until there's a good way to handle fixed-size message with protobuf.
        }
    }

    
    void FromBytes(char* srcBuf)
    {
        pos_bc::InodeTableHeaderContentProto deserializedProto;
        int ret = deserializedProto.ParseFromArray(srcBuf, InodeTableHeaderOnSsdSize);
        if(ret == 0){
            // ret == 0 means ParseFromArray ran into an error. It's because we have passed in 
            // a fixed-size length, i.e., ONSSD_SIZE, regardless of the actual message size.
            // The current implementation has a risk of not detecting a parse error when there has been
            // real data corruption, but such data corruption would have put the system in an irrecoverable state anyway, 
            // so I'd keep the current impl until there's a good way to handle fixed-size message with protobuf. 
        }

        this->totalInodeNum = deserializedProto.totalinodenum();
        this->inodeEntryByteSize = deserializedProto.inodeentrybytesize();
        this->totalFileCreated = deserializedProto.totalfilecreated();

        pos_bc::InodeInUseBitmap deserializedInodeInUseBitmapProto;
        deserializedInodeInUseBitmapProto = deserializedProto.inodeinusebitmap();
        bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT> tmpbitset(deserializedInodeInUseBitmapProto.bits());

        this->inodeInUseBitmap.bits = tmpbitset;
        this->inodeInUseBitmap.allocatedInodeCnt = deserializedInodeInUseBitmapProto.allocatedinodecnt();
        for(int i = 0;i<deserializedProto.allocextentslist_size(); ++i)
        {
            this->allocExtentsList[i].SetStartLpn(deserializedProto.allocextentslist(i).startlpn());
            this->allocExtentsList[i].SetCount(deserializedProto.allocextentslist(i).count());
        }
    }

    void ToBytesByIndex(char* destBuf, int idx)
    {
        ToBytes(destBuf);
    }

    void FromBytesByIndex(char* srcBuf, int idx)
    {
        FromBytes(srcBuf);
    }

    size_t GetOnSsdSize()
    {
        return InodeTableHeaderOnSsdSize; // 3 LPN
    }

    size_t GetSizeOfEntry()
    {
        return 4096; //Not used in InodeTableHeader, must be equal or larger than 4096
    }
};

class InodeTableHeader :  public OnVolumeMetaRegionProto<MetaRegionType, InodeTableHeaderContent>
{
public:
    explicit InodeTableHeader(MetaVolumeType volumeType, MetaLpnType baseLpn);
    virtual ~InodeTableHeader(void);

    virtual void Create(uint32_t totalFileNum);
    virtual void SetInodeInUse(uint32_t idx);
    virtual void ClearInodeInUse(uint32_t idx);
    virtual bool IsFileInodeInUse(uint32_t idx);
    virtual uint32_t GetTotalAllocatedInodeCnt(void);
    virtual std::vector<MetaFileExtent> GetFileExtentContent(void);
    virtual void SetFileExtentContent(std::vector<MetaFileExtent>& extents);
    virtual size_t GetFileExtentContentSize(void);
    virtual void BuildFreeInodeEntryMap(void);
    virtual uint32_t GetFreeInodeEntryIdx(void);
    virtual std::bitset<MetaFsConfig::MAX_META_FILE_NUM_SUPPORT>& GetInodeInUseBitmap(void);
    virtual bool Load(void);
    virtual bool Load(MetaStorageType media, MetaLpnType baseLPN,
        uint32_t idx, MetaLpnType pageCNT);
    virtual bool Store(void);
    virtual bool Store(MetaStorageType media, MetaLpnType baseLPN,
        uint32_t idx, MetaLpnType pageCNT);

private:
    void _PrintLog(void) const;
    std::queue<uint32_t>* freeInodeEntryIdxQ;
};
} // namespace pos
