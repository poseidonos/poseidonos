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
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
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

namespace ibofos
{
struct deviceInfo
{
    unsigned int device_type;
    uint32_t pad4_0[3];
    char device_uid[32];
    unsigned int device_state;
    uint32_t pad4_1[3];
};

struct arrayBootRecord
{
    char array_name[64];
    unsigned int abr_version;
    uint32_t pad4_0[7];
    char meta_raid_type[16];
    char data_raid_type[16];
    unsigned int total_dev_num;
    uint32_t pad4_1[3];
    unsigned int nvm_dev_num;
    uint32_t pad4_2[3];
    unsigned int data_dev_num;
    uint32_t pad4_3[3];
    unsigned int spare_dev_num;
    uint32_t pad4_4[2];

    unsigned int mfsInit;
    struct deviceInfo dev_info[256];
};

struct systemBootRecord
{
    char pos_version[16];
    uint32_t pad4_0[4];
    int mbr_version;
    uint32_t pad4_1[7];
    char system_uuid[64];
    unsigned int arrayNum;
    uint32_t pad4[7];
    char arrayBit[16];
    uint32_t pad5[4];
    char arrayDevBit[16][32];
    uint32_t pad6[80];

    struct arrayBootRecord array_info[15];
    uint32_t reserved[3119];
    uint32_t mbrParity;
};

const unsigned int POS_VERSION_SIZE = 16;
const unsigned int MBR_VERSION_SIZE = 16;
const unsigned int SYSTEM_UUID_SIZE = 64;

const unsigned int ARRAY_NAME_SIZE = 64;
const unsigned int META_RAID_TYPE_SIZE = 16;
const unsigned int DATA_RAID_TYPE_SIZE = 16;
const unsigned int ARRAY_DEVICE_BIT_SIZE = 32;

const unsigned int DEVICE_UID_SIZE = 32;

} // namespace ibofos
