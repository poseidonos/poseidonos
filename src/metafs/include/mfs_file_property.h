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

#pragma once

#include "mfs_file_prop_integrity_def.h"

// definitions below indicate io characteristics of the corresponding meta file access
// meta filesystem stores metadata and service io efficiently according to the setting given by user
enum class MDFilePropIoAccessPattern
{
    ByteIntensive,
    SmallSizeBlockIO,
    NoSpecific,

    Max,

    Default = NoSpecific
};

enum class MDFilePropIoOpType
{
    ReadDominant = 0,
    WriteDominant = 1,
    NoSpecific = 2,

    Max,

    Default = NoSpecific
};

class MetaFilePropertySet
{
public:
    MetaFilePropertySet(void)
    : ioAccPattern(MDFilePropIoAccessPattern::Default),
      ioOpType(MDFilePropIoOpType::Default),
      integrity(MDFilePropIntegrity::Default)
    {
    }
    MDFilePropIoAccessPattern ioAccPattern;
    MDFilePropIoOpType ioOpType;
    MDFilePropIntegrity integrity;
};
