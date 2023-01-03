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

#include "pbr_tester.h"
#include "src/pbr/dto/ate_data.h"
#include "src/pbr/update/pbr_file_updater.h"
#include "src/pbr/revision.h"
#include "src/helper/time/time_helper.h"

namespace pbr
{
PbrTester::PbrTester(void)
{
    fileList.push_back("/root/pbr1.pbr");
    fileList.push_back("/root/pbr2.pbr");
    fileList.push_back("/root/pbr3.pbr");
}

int
PbrTester::Create(void)
{
    _Create(4, 4);
    _Verify(arrayname, arrayuuid);
    return 0;
}

int
PbrTester::Update(void)
{
    AteData* ate = new AteData();
    ate->nodeUuid = nodeuuid;
    ate->arrayUuid = newArrayuuid;
    ate->arrayName = newArrayname;
    ate->createdDateTime = _GetCurrentSecondsAsEpoch();
    ate->lastUpdatedDateTime =_GetCurrentSecondsAsEpoch();
    _AddAde(ate, 4);
    _AddPte(ate, 4);
    IPbrUpdater* updater = new PbrFileUpdater(REVISION, fileList);
    updater->Update(ate);
    _Verify(newArrayname, newArrayuuid);

    return 0;
}

void
PbrTester::_Create(uint32_t adeCnt, uint32_t pteCnt)
{
    AteData* ate = new AteData();
    ate->nodeUuid = nodeuuid;
    ate->arrayUuid = arrayuuid;
    ate->arrayName = arrayname;
    ate->createdDateTime = _GetCurrentSecondsAsEpoch();
    ate->lastUpdatedDateTime =_GetCurrentSecondsAsEpoch();
    _AddAde(ate, adeCnt);
    _AddPte(ate, pteCnt);
    IPbrUpdater* updater = new PbrFileUpdater(REVISION, fileList);
    updater->Update(ate);
}

vector<AteData*>
PbrTester::_Load()
{
    IPbr* iPbr = new Pbr();
    vector<AteData*> ateList;
    iPbr->Load(ateList, fileList);
    return ateList;
}

bool
PbrTester::_Verify(string expectedArrayName, string expectedArrayUuid)
{
    vector<AteData*> ateList =_Load();
    if (ateList.size() == 0)
    {
        return false;
    }

    for (auto ate : ateList)
    {
        if (ate->arrayName != expectedArrayName)
        {
            return false;
        }
        if (ate->arrayUuid != expectedArrayUuid)
        {
            return false;
        }
    }
    return true;
}

void
PbrTester::_AddAde(AteData* ateData, uint32_t adeCnt)
{
    for (uint32_t i = 0; i < adeCnt; i++)
    {
        AdeData* ad = new AdeData();
        ad->devIndex = i;
        ad->devTypeGuid = "DATA";
        ad->devStateGuid = "NORMAL";
        ad->devSn = "unvme-ns-" + to_string(i);
        ateData->adeList.push_back(ad);
    }
}

void
PbrTester::_AddPte(AteData* ateData, uint32_t pteCnt)
{
    uint64_t startLba = 0;
    uint64_t size = 1024;
    for (uint32_t i = 0; i < pteCnt; i++)
    {
        PteData* pd = new PteData();
        pd->partTypeGuid = "PART_TYPE_" + to_string(i);
        pd->raidTypeGuid = "RAID_TYPE_" + to_string(i);
        pd->startLba = startLba;
        pd->lastLba = startLba + size - 1;
        startLba = pd->lastLba + 1;
        ateData->pteList.push_back(pd);
    }
}

} // namespace pbr
