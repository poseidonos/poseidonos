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

#include "meta_file_inode.h"
#include "on_volume_meta_region_proto.h"
#include "meta_region_type.h"
#include "proto/generated/cpp/pos_bc.pb.h"

namespace pos
{
const int InodeTableContentOnSsdSize = INODE_DATA_BYTE_SIZE;  
class InodeTableContent : public MetaRegionProtoContent
{
public:
    static const uint32_t MAX_META_FILE_INODE_NUM = MetaFsConfig::MAX_META_FILE_NUM_SUPPORT;
    std::array<MetaFileInode, InodeTableContent::MAX_META_FILE_INODE_NUM> entries;

    void ToBytes(char* destBuf)
    {
        // Write to Buffer
        for (int i = 0; i < (int)InodeTableContent::MAX_META_FILE_INODE_NUM; ++i)
        {
            ToBytesByIndex(destBuf, i);
        }        
    }

    void FromBytes(char* srcBuf)
    {        
        // Read From Buffer
        for (int i = 0; i < (int)InodeTableContent::MAX_META_FILE_INODE_NUM; ++i)
        {
            FromBytesByIndex(srcBuf, i);
        }
    }

    void ToBytesByIndex(char* destBuf, int entryIdx)
    {    
        pos_bc::MetaFileInodeDataProto metaFileInodeDataProto;
        metaFileInodeDataProto.set_inuse(entries[entryIdx].data.basic.field.inUse);
        metaFileInodeDataProto.set_age(entries[entryIdx].data.basic.field.age);
        metaFileInodeDataProto.set_ctime(entries[entryIdx].data.basic.field.ctime);
        metaFileInodeDataProto.set_referencecnt(entries[entryIdx].data.basic.field.referenceCnt);
        metaFileInodeDataProto.set_filedescriptortype(entries[entryIdx].data.basic.field.fd);
        metaFileInodeDataProto.set_filename(entries[entryIdx].data.basic.field.fileName.ToString());
        metaFileInodeDataProto.set_filebytesize(entries[entryIdx].data.basic.field.fileByteSize);
        metaFileInodeDataProto.set_datachunksize(entries[entryIdx].data.basic.field.dataChunkSize);
        
        //Set IoAtrribute
        pos_bc::MetaStorageIoProperty metaStorageIoProperty;
        metaStorageIoProperty.set_media((pos_bc::MetaStorageIoProperty::MetaStorageType)entries[entryIdx].data.basic.field.ioAttribute.media);

        pos_bc::MetaFilePropertySet metaFilePropertySet;
        metaFilePropertySet.set_integrity((pos_bc::MetaFilePropertySet::MetaFileIntegrityType) entries[entryIdx].data.basic.field.ioAttribute.ioSpecfic.integrity);
        metaFilePropertySet.set_type((pos_bc::MetaFilePropertySet::MetaFileType) entries[entryIdx].data.basic.field.ioAttribute.ioSpecfic.type);
        *metaStorageIoProperty.mutable_iospecific() = metaFilePropertySet;

        *metaFileInodeDataProto.mutable_ioattribute() = metaStorageIoProperty;

        metaFileInodeDataProto.set_indexininodetable(entries[entryIdx].data.basic.field.indexInInodeTable);
        metaFileInodeDataProto.set_versionsignature(entries[entryIdx].data.basic.field.versionSignature);
        metaFileInodeDataProto.set_version(entries[entryIdx].data.basic.field.version);
        metaFileInodeDataProto.set_pagemapcnt(entries[entryIdx].data.basic.field.pagemapCnt);
        
        for (int j = 0; j < (int)entries[entryIdx].data.basic.field.pagemapCnt ; ++j)
        {
            pos_bc::MetaFileExtent* MetaFileExtentProto = metaFileInodeDataProto.add_pagemap();
            MetaFileExtentProto->set_startlpn(entries[entryIdx].data.basic.field.pagemap[j].GetStartLpn());
            MetaFileExtentProto->set_count(entries[entryIdx].data.basic.field.pagemap[j].GetCount());
        }

        metaFileInodeDataProto.set_ctimecopy(entries[entryIdx].data.basic.field.ctimeCopy);
        metaFileInodeDataProto.set_agecopy(entries[entryIdx].data.basic.field.ageCopy);

        int ret = metaFileInodeDataProto.SerializeToArray(destBuf + entryIdx * InodeTableContentOnSsdSize, InodeTableContentOnSsdSize);
        if (ret == 0){
            // ret == 0 means ParseFromArray ran into an error. It's because we have passed in 
            // a fixed-size length, i.e., ONSSD_SIZE, regardless of the actual message size.
            // The current implementation has a risk of not detecting a parse error when there has been
            // real data corruption, but such data corruption would have put the system in an irrecoverable state anyway, 
            // so I'd keep the current impl until there's a good way to handle fixed-size message with protobuf.
        }
    }

    void FromBytesByIndex(char* srcBuf, int entryIdx)
    {
        pos_bc::MetaFileInodeDataProto deserializedProto;
        int ret = deserializedProto.ParseFromArray(srcBuf + entryIdx * InodeTableContentOnSsdSize, InodeTableContentOnSsdSize);
        if(ret == 0){
            // ret == 0 means ParseFromArray ran into an error. It's because we have passed in 
            // a fixed-size length, i.e., ONSSD_SIZE, regardless of the actual message size.
            // The current implementation has a risk of not detecting a parse error when there has been
            // real data corruption, but such data corruption would have put the system in an irrecoverable state anyway, 
            // so I'd keep the current impl until there's a good way to handle fixed-size message with protobuf. 
        }
        this->entries[entryIdx].data.basic.field.inUse = deserializedProto.inuse();
        this->entries[entryIdx].data.basic.field.age = deserializedProto.age();
        this->entries[entryIdx].data.basic.field.ctime = deserializedProto.ctime();
        this->entries[entryIdx].data.basic.field.referenceCnt = deserializedProto.referencecnt();

        this->entries[entryIdx].data.basic.field.fd = deserializedProto.filedescriptortype();
        this->entries[entryIdx].data.basic.field.fileName = deserializedProto.filename();
        this->entries[entryIdx].data.basic.field.fileByteSize = deserializedProto.filebytesize();
        this->entries[entryIdx].data.basic.field.dataChunkSize = deserializedProto.datachunksize();

        this->entries[entryIdx].data.basic.field.ioAttribute.ioSpecfic.integrity = (MetaFileIntegrityType) deserializedProto.ioattribute().iospecific().integrity();
        this->entries[entryIdx].data.basic.field.ioAttribute.ioSpecfic.type = (MetaFileType) deserializedProto.ioattribute().iospecific().type();
        this->entries[entryIdx].data.basic.field.ioAttribute.media = (MetaStorageType) deserializedProto.ioattribute().media();

        this->entries[entryIdx].data.basic.field.indexInInodeTable = deserializedProto.indexininodetable();


        this->entries[entryIdx].data.basic.field.versionSignature = deserializedProto.versionsignature();
        this->entries[entryIdx].data.basic.field.version = deserializedProto.version();
        this->entries[entryIdx].data.basic.field.pagemapCnt = deserializedProto.pagemapcnt();

        for(int j = 0; j < deserializedProto.pagemap_size(); ++j)
        {
            this->entries[entryIdx].data.basic.field.pagemap[j].SetStartLpn(deserializedProto.pagemap(j).startlpn());
            this->entries[entryIdx].data.basic.field.pagemap[j].SetCount(deserializedProto.pagemap(j).count());
        }

        this->entries[entryIdx].data.basic.field.ctimeCopy = deserializedProto.ctimecopy();
        this->entries[entryIdx].data.basic.field.ageCopy = deserializedProto.agecopy();
    }

    size_t GetOnSsdSize()
    {
        return InodeTableContentOnSsdSize * MAX_META_FILE_INODE_NUM; // 4 LPN * 1024 = 4096 LPN
    }

    size_t GetSizeOfEntry()
    {
        return InodeTableContentOnSsdSize;
    }
};
using MetaFileInodeArray = std::array<MetaFileInode, InodeTableContent::MAX_META_FILE_INODE_NUM>;

class InodeTable : public OnVolumeMetaRegionProto<MetaRegionType, InodeTableContent>
{
public:
    explicit InodeTable(MetaVolumeType volumeType, MetaLpnType startLpn);
    virtual ~InodeTable(void);

    virtual void Create(uint32_t maxInodeEntryNum);
    virtual FileDescriptorType GetFileDescriptor(uint32_t inodeIdx);
    virtual MetaFileInode& GetInode(uint32_t inodeIdx);
    virtual MetaFileInodeArray& GetInodeArray(void);
};
} // namespace pos
