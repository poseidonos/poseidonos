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

#ifndef ABR_MANAGER_H_
#define ABR_MANAGER_H_

#include <string>
#include <vector>

#include "src/array/interface/i_abr_control.h"
#include "src/array/meta/array_meta.h"
#include "src/mbr/mbr_info.h"
#include "src/mbr/mbr_manager.h"

using namespace std;

namespace pos
{
class AbrManager : public IAbrControl
{
public:
    AbrManager(void);
    explicit AbrManager(MbrManager* mbrMgr);
    ~AbrManager(void) override;

    virtual int CreateAbr(ArrayMeta& meta) override;
    virtual int DeleteAbr(string arrayName) override;
    virtual int LoadAbr(ArrayMeta& meta) override;
    virtual int SaveAbr(ArrayMeta& meta) override;
    virtual int ResetMbr(void) override;
    virtual string FindArrayWithDeviceSN(string devSN) override;
    virtual string GetLastUpdatedDateTime(string arrayName) override;
    virtual string GetCreatedDateTime(string arrayName) override;
    virtual int GetAbrList(std::vector<ArrayBootRecord>& abrList);
    virtual void InitDisk(UblockSharedPtr dev);

private:
    MbrManager* mbrManager;
};

} // namespace pos

#endif // ABR_MANAGER_H_
