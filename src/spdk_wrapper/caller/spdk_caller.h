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
#include "spdk/bdev.h"
#include "spdk/bdev_module.h"
#include "spdk/nvmf.h"
#include "spdk/pos.h"
#include "src/lib/singleton.h"

namespace pos
{
class SpdkCaller
{
public:
    SpdkCaller(void);
    virtual ~SpdkCaller(void);
    virtual char* SpdkNvmfSubsystemGetCtrlrHostnqn(struct spdk_nvmf_ctrlr* ctrlr);
    virtual struct spdk_nvmf_ctrlr* SpdkNvmfSubsystemGetFirstCtrlr(struct spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_nvmf_ctrlr* SpdkNvmfSubsystemGetNextCtrlr(struct spdk_nvmf_subsystem* subsystem,
        struct spdk_nvmf_ctrlr* prevCtrlr);
    virtual int SpdkNvmfSubsystemSetPauseDirectly(struct spdk_nvmf_subsystem *subsystem);
    virtual uint32_t SpdkNvmfSubsystemGetId(spdk_nvmf_subsystem* subsystem);
    virtual struct spdk_bdev* SpdkBdevCreatePosDisk(const char* volumeName, uint32_t volumeId,
        const struct spdk_uuid* bdevUuid, uint64_t numBlocks, uint32_t blockSize,
        bool volumeTypeInMemory, const char* arrayName, uint64_t arrayId);
    virtual void SpdkBdevDeletePosDisk(struct spdk_bdev* bdev, pos_bdev_delete_callback cbFunc, void* cbArg);
    virtual struct spdk_bdev* SpdkBdevGetByName(const char* bdevName);
    virtual const char* SpdkBdevGetName(const struct spdk_bdev* bdev);
    virtual void SpdkBdevSetQosRateLimits(struct spdk_bdev* bdev, uint64_t* limits,
        void (*cbFunc)(void* cbArg, int status), void* cbArg);
    virtual const char* SpdkGetAttachedSubsystemNqn(const char* bdevName);
    virtual const struct spdk_uuid* SpdkBdevGetUuid(const struct spdk_bdev* bdev);
    virtual int SpdkUuidFmtLower(char* uuid_str, size_t uuid_str_size, const struct spdk_uuid* uuid);
    virtual int SpdkUuidParse(struct spdk_uuid* uuid, const char* uuid_str);
    virtual void SpdkBdevPosRegisterPoller(void (*func)(void));
    virtual void SpdkBdevPosUnRegisterPoller(void (*func)(void));
};

using SpdkCallerSingleton = Singleton<SpdkCaller>;
} // namespace pos
