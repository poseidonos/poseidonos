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

#include "metafs_common.h"
#include "os_header.h"

namespace pos
{
class MDPageControlInfo
{
public:
    static const uint32_t MDPAGE_CTRL_INFO_SIG = 0x00C0FFEE;
    static const uint32_t CONTROL_INFO_SIZE = MetaFsIoConfig::META_PAGE_CONTROL_INFO_SIZE;
    union {
        struct
        {
            /* total 64 byte = CONTROL_INFO_SIZE */
            uint32_t mfsSignature;      //  4byte   0..3
            uint32_t reserved;          //  4byte   4..7
            uint64_t epochSignature;    //  8byte   8..15
            uint32_t version;           //  4byte   16..19
            FileDescriptorType fd;      //  4byte   20..23
            MetaLpnType metaLpn;        //  8byte   24..31
            uint8_t reserved2[28];      // 28byte   32..59
            uint32_t crc;               //  4byte   60..63, have to be at the end
        };

        uint8_t all[CONTROL_INFO_SIZE];
    };
};
} // namespace pos
