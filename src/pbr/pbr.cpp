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

#include "pbr.h"
#include "revision.h"
#include "src/pbr/load/pbr_loader.h"
#include "src/pbr/update/pbr_updater.h"

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pbr
{
int
Pbr::Load(vector<AteData*>& ateListOut, vector<pos::UblockSharedPtr> devs)
{
    IPbrLoader* loader = new PbrLoader(devs);
    int ret = loader->Load(ateListOut);
    return ret;
}

int
Pbr::Reset(vector<pos::UblockSharedPtr> devs)
{
    IPbrUpdater* updater = new PbrUpdater(REVISION, devs);
    int ret = updater->Clear();
    return ret;
}

int
Pbr::Update(vector<pos::UblockSharedPtr> devs, AteData* ateData)
{
    IPbrUpdater* updater = new PbrUpdater(REVISION, devs);
    int ret = updater->Update(ateData);
    return ret;
}
} // namespace pbr
