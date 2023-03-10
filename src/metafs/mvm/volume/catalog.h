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
#include "on_volume_meta_region_proto.h"
#include "os_header.h"

#include "proto/generated/cpp/pos_bc.pb.h"

namespace pos
{
// Meta volume catalog contains baseline information of corresponding volume which are base lpn and size of each regions
// the contents of this class are buffered in a particular memory space directly, hence please do not define any STL variables here

// The fixed size for a single CatalogContent
const int CatalogContentOnSsdSize = 4096; // == 1 LPN
class CatalogContent : public MetaRegionProtoContent
{
public:
    uint64_t signature;
    VolumeBasicInfo volumeInfo;
    MetaRegionMap regionMap[(int)MetaRegionType::Max];

    // Note: if you want to add any field, add variables here and pos_bc.proto::CatalogContentProto

    void ToBytes(char* destBuf)
    {
        pos_bc::CatalogContentProto proto;

        // Set Signature value to proto
        proto.set_signature(this->signature);

        // Set volumeInfo value to proto
        pos_bc::VolumeBasicInfo volumeInfoProto;
        volumeInfoProto.set_maxvolpagenum(this->volumeInfo.maxVolPageNum);
        volumeInfoProto.set_maxfilenumsupport(this->volumeInfo.maxFileNumSupport);

        *proto.mutable_volumeinfo() = volumeInfoProto;

        for (int i = 0 ;i < (int) MetaRegionType::Max; ++i)
        {
            pos_bc::MetaRegionMap* metaRegionMapProto = proto.add_regionmap();
            metaRegionMapProto->set_baselpn(this->regionMap[i].baseLpn);
            metaRegionMapProto->set_maxlpn(this->regionMap[i].maxLpn);
        }
        
        int ret = proto.SerializeToArray(destBuf, CatalogContentOnSsdSize);    
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
        pos_bc::CatalogContentProto deserializedProto;
        int ret = deserializedProto.ParseFromArray(srcBuf, CatalogContentOnSsdSize);
        if(ret == 0){
            // ret == 0 means ParseFromArray ran into an error. It's because we have passed in 
            // a fixed-size length, i.e., ONSSD_SIZE, regardless of the actual message size.
            // The current implementation has a risk of not detecting a parse error when there has been
            // real data corruption, but such data corruption would have put the system in an irrecoverable state anyway, 
            // so I'd keep the current impl until there's a good way to handle fixed-size message with protobuf. 
        }

        // Read signature, volumeInfo values from buffer
        this->signature = deserializedProto.signature();
        this->volumeInfo.maxFileNumSupport = deserializedProto.volumeinfo().maxfilenumsupport();
        this->volumeInfo.maxVolPageNum = deserializedProto.volumeinfo().maxvolpagenum();


        // Read region map values from buffer
        for(int i = 0; i < deserializedProto.regionmap_size(); ++i)
        {
            this->regionMap[i].baseLpn = deserializedProto.regionmap(i).baselpn();
            this->regionMap[i].maxLpn = deserializedProto.regionmap(i).maxlpn();
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
        return CatalogContentOnSsdSize; // 1 LPN
    }

    size_t GetSizeOfEntry()
    {
        return 4096; // Not used in CatalogContent, must be equal or larger than 4096
    }
};

class Catalog : public OnVolumeMetaRegionProto<MetaRegionType, CatalogContent>
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
