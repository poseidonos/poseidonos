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

    int LoadAbr(string arrayName, ArrayMeta& meta, unsigned int& arrayIndex) override;
    int SaveAbr(string arrayName, ArrayMeta& meta) override;
    bool GetMfsInit(string arrayName) override;
    int SetMfsInit(string arrayName, bool value) override;
    virtual int CreateAbr(string arrayName, ArrayMeta& meta, unsigned int& arrayIndex) override;
    virtual int DeleteAbr(string arrayName, ArrayMeta& meta) override;
    virtual int ResetMbr(void) override;
    int GetAbrList(std::vector<ArrayBootRecord>& abrList);
    virtual string FindArrayWithDeviceSN(string devSN);

private:
    MbrManager* mbrManager;
};

} // namespace pos

#endif // ABR_MANAGER_H_
