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
#include "meta_storage_specific.h"
#include "metafs_common.h"

namespace pos
{
// A interface class for meta region management: contains common API set
class MetaRegionManager
{
public:
    explicit MetaRegionManager(int arrayId);
    virtual ~MetaRegionManager(void);

    // Init: used to initialize internal context based on given paramters
    virtual void Init(MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn) = 0;

    // Bringup: used to build internal context and prepare internal to handle requests
    virtual void Bringup(void) = 0;
    virtual bool SaveContent(void) = 0;
    virtual MetaLpnType GetRegionSizeInLpn(void) = 0;
    virtual void Finalize(void) = 0;

    void SetRegionInfo(MetaStorageType mediaType, MetaLpnType baseLpn, MetaLpnType maxLpn);

protected:
    MetaStorageType mediaType;
    MetaLpnType baseLpn;
    MetaLpnType maxLpn;
    int arrayId;
};
} // namespace pos
