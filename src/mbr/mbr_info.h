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
#include <cstdint>
namespace pos
{
const unsigned int MAX_ARRAY_CNT = 8;
const unsigned int MAX_ARRAY_DEVICE_CNT = 128;
const unsigned int POS_VERSION_SIZE = 16;
const unsigned int SYSTEM_UUID_SIZE = 64;

const unsigned int ARRAY_NAME_SIZE = 64;
const unsigned int META_RAID_TYPE_SIZE = 16;
const unsigned int DATA_RAID_TYPE_SIZE = 16;
const unsigned int DEVICE_UID_SIZE = 32;
const unsigned int DATE_SIZE = 32;

const unsigned int DEVICE_TYPE_OFFSET = 0;
const unsigned int DEVICE_INFO_PADDING_0_OFFSET = DEVICE_TYPE_OFFSET + sizeof(unsigned int);
const unsigned int DEVICE_INFO_PADDING_0_NUM = 3;
const unsigned int DEVICE_UID_OFFSET = DEVICE_INFO_PADDING_0_OFFSET + sizeof(uint32_t) * DEVICE_INFO_PADDING_0_NUM;
const unsigned int DEVICE_STATE_OFFSET = DEVICE_UID_OFFSET + sizeof(char) * DEVICE_UID_SIZE;
const unsigned int DEVICE_INFO_SIZE = 128;
const unsigned int DEVICE_INFO_PADDING_1_OFFSET = DEVICE_STATE_OFFSET + sizeof(unsigned int);
const unsigned int DEVICE_INFO_PADDING_1_NUM = (DEVICE_INFO_SIZE - DEVICE_INFO_PADDING_1_OFFSET) / sizeof(uint32_t);

const unsigned int ARRAY_NAME_OFFSET = 0;
const unsigned int ABR_VERSION_OFFSET = ARRAY_NAME_OFFSET + sizeof(char) * ARRAY_NAME_SIZE;
const unsigned int ABR_PADDING_0_OFFSET = ABR_VERSION_OFFSET + sizeof(unsigned int);
const unsigned int ABR_PADDING_0_SIZE = 7;
const unsigned int META_RAID_TYPE_OFFSET = ABR_PADDING_0_OFFSET + sizeof(uint32_t) * ABR_PADDING_0_SIZE;
const unsigned int DATA_RAID_TYPE_OFFSET = META_RAID_TYPE_OFFSET + sizeof(char) * META_RAID_TYPE_SIZE;
const unsigned int TOTAL_DEV_NUM_OFFSET = DATA_RAID_TYPE_OFFSET + sizeof(char) * DATA_RAID_TYPE_SIZE;
const unsigned int NVM_DEV_NUM_OFFSET = TOTAL_DEV_NUM_OFFSET + sizeof(unsigned int);
const unsigned int DATA_DEV_NUM_OFFSET = NVM_DEV_NUM_OFFSET + sizeof(unsigned int);
const unsigned int SPARE_DEV_NUM_OFFSET = DATA_DEV_NUM_OFFSET + sizeof(unsigned int);
const unsigned int MFS_INIT_OFFSET = SPARE_DEV_NUM_OFFSET + sizeof(unsigned int);
const unsigned int CREATE_DATE_OFFSET = MFS_INIT_OFFSET + sizeof(unsigned int);
const unsigned int MODIFIED_DATE_OFFSET = CREATE_DATE_OFFSET + sizeof(char) * DATE_SIZE;
const unsigned int INSTANCE_ID_OFFSET = MODIFIED_DATE_OFFSET + sizeof(char) * DATE_SIZE;
const unsigned int ABR_PADDING_1_OFFSET = INSTANCE_ID_OFFSET + sizeof(unsigned int);
const unsigned int ABR_DEVICE_INFO_OFFSET = 1024;
const unsigned int ABR_PADDING_1_NUM = (ABR_DEVICE_INFO_OFFSET - ABR_PADDING_1_OFFSET) / sizeof(uint32_t);
const unsigned int ABR_RESERVED_OFFSET = ABR_DEVICE_INFO_OFFSET + DEVICE_INFO_SIZE * MAX_ARRAY_DEVICE_CNT;
const unsigned int ABR_RESERVED_NUM = 128;
const unsigned int ABR_SIZE = ABR_RESERVED_OFFSET + sizeof(uint32_t) * ABR_RESERVED_NUM;

const unsigned int POS_VERSION_OFFSET = 0;
const unsigned int MBR_PADDING_0_OFFSET = POS_VERSION_OFFSET + sizeof(char) * POS_VERSION_SIZE;
const unsigned int MBR_PADDING_0_NUM = 4;
const unsigned int MBR_VERSION_OFFSET = MBR_PADDING_0_OFFSET + sizeof(uint32_t) * MBR_PADDING_0_NUM;
const unsigned int MBR_PADDING_1_OFFSET = MBR_VERSION_OFFSET + sizeof(unsigned int);
const unsigned int MBR_PADDING_1_NUM = 7;
const unsigned int SYSTEM_UUID_OFFSET = MBR_PADDING_1_OFFSET + sizeof(uint32_t) * MBR_PADDING_1_NUM;
const unsigned int ARRAY_NUM_OFFSET = SYSTEM_UUID_OFFSET + sizeof(char) * SYSTEM_UUID_SIZE;
const unsigned int MBR_PADDING_2_OFFSET = ARRAY_NUM_OFFSET + sizeof(unsigned int);
const unsigned int MBR_PADDING_2_NUM = 7;
const unsigned int ARRAY_FLAG_OFFSET = MBR_PADDING_2_OFFSET + sizeof(uint32_t) * MBR_PADDING_2_NUM;
const unsigned int MBR_PADDING_3_OFFSET = ARRAY_FLAG_OFFSET + sizeof(unsigned int) * MAX_ARRAY_CNT;
const unsigned int MBR_PADDING_3_NUM = 4;
const unsigned int ARRAY_DEVICE_FLAG_OFFSET = MBR_PADDING_3_OFFSET + sizeof(uint32_t) * MBR_PADDING_3_NUM;
const unsigned int MBR_PADDING_4_OFFSET = ARRAY_DEVICE_FLAG_OFFSET + sizeof(unsigned int) * MAX_ARRAY_DEVICE_CNT;
const unsigned int MBR_ABR_OFFSET = 4096;
const unsigned int MBR_PADDING_4_NUM = (MBR_ABR_OFFSET - MBR_PADDING_4_OFFSET) / sizeof(uint32_t);
const unsigned int MBR_RESERVED_OFFSET = MBR_ABR_OFFSET + ABR_SIZE * MAX_ARRAY_CNT;
const unsigned int MBR_SIZE = 262144;
const unsigned int MBR_PARITY_SIZE = sizeof(uint32_t);
const unsigned int MBR_RESERVED_NUM = (MBR_SIZE - MBR_PARITY_SIZE - MBR_RESERVED_OFFSET) / sizeof(uint32_t);
const unsigned int MBR_PARITY_OFFSET = MBR_RESERVED_OFFSET + sizeof(uint32_t) * MBR_RESERVED_NUM;

struct deviceInfo
{
    unsigned int deviceType;
    uint32_t pad0[DEVICE_INFO_PADDING_0_NUM];
    char deviceUid[DEVICE_UID_SIZE];
    unsigned int deviceState;
    uint32_t pad1[DEVICE_INFO_PADDING_1_NUM];
};

struct ArrayBootRecord
{
    char arrayName[ARRAY_NAME_SIZE];
    unsigned int abrVersion;
    uint32_t pad0[ABR_PADDING_0_SIZE];
    char metaRaidType[META_RAID_TYPE_SIZE];
    char dataRaidType[DATA_RAID_TYPE_SIZE];
    unsigned int totalDevNum;
    unsigned int nvmDevNum;
    unsigned int dataDevNum;
    unsigned int spareDevNum;
    unsigned int mfsInit;
    char createDatetime[DATE_SIZE];
    char updateDatetime[DATE_SIZE];
    unsigned int uniqueId;
    uint32_t pad1[ABR_PADDING_1_NUM];
    struct deviceInfo devInfo[MAX_ARRAY_DEVICE_CNT];
    uint32_t reserved[ABR_RESERVED_NUM];
};

struct masterBootRecord
{
    char posVersion[POS_VERSION_SIZE];
    uint32_t pad0[MBR_PADDING_0_NUM];
    unsigned int mbrVersion;
    uint32_t pad1[MBR_PADDING_1_NUM];
    char systemUuid[SYSTEM_UUID_SIZE];
    unsigned int arrayNum;
    uint32_t pad2[MBR_PADDING_2_NUM];
    unsigned int arrayValidFlag[MAX_ARRAY_CNT];
    uint32_t pad3[MBR_PADDING_3_NUM];
    unsigned int arrayDevFlag[MAX_ARRAY_DEVICE_CNT];
    uint32_t pad4[MBR_PADDING_4_NUM];
    struct ArrayBootRecord arrayInfo[MAX_ARRAY_CNT];
    uint32_t reserved[MBR_RESERVED_NUM];
    uint32_t mbrParity;
};
} // namespace pos
