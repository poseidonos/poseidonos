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

#include <cstdint>
#include <functional>

#include "spdk/pos.h"
#include "src/bio/flush_io.h"
#include "src/bio/volume_io.h"
#include "src/event_scheduler/callback.h"
#include "src/volume/volume_service.h"
namespace pos
{
struct IOCtx
{
    IOCtx(void);

    int cnt;
    int needPollingCount;
};

class VolumeManager;
#ifdef _ADMIN_ENABLED
class IArrayInfo;
class IDevInfo;
class IIODispatcher;
#endif
class AioCompletion : public Callback,
                      public std::enable_shared_from_this<AioCompletion>
{
public:
    AioCompletion(FlushIoSmartPtr flushIo, pos_io& posIo, IOCtx& ioContext);
    AioCompletion(FlushIoSmartPtr flushIo, pos_io& posIo, IOCtx& ioContext, std::function<bool(uint32_t)> isSameReactorNowFunc);
    AioCompletion(VolumeIoSmartPtr volumeIo, pos_io& posIo, IOCtx& ioContext);
    AioCompletion(VolumeIoSmartPtr volumeIo, pos_io& posIo, IOCtx& ioContext, std::function<bool(uint32_t)> isSameReactorNowFunc);
    ~AioCompletion(void) override;

private:
    void _SendUserCompletion(void);
    bool _DoSpecificJob(void) override;

    FlushIoSmartPtr flushIo;
    VolumeIoSmartPtr volumeIo;
    pos_io& posIo;
    IOCtx& ioContext;
    static VolumeService& volumeService;
    std::function<bool(uint32_t)> isSameReactorNowFunc;
};

class AIO
{
public:
    AIO(void);
    void SubmitAsyncIO(pos_io& posIo);
    void CompleteIOs(void);
#ifdef _ADMIN_ENABLED
    void SubmitAsyncAdmin(pos_io& io);
#endif

private:
    static thread_local IOCtx ioContext;
    VolumeIoSmartPtr _CreateVolumeIo(pos_io& posIo);
    FlushIoSmartPtr _CreateFlushIo(pos_io& posIo);
};
#ifdef _ADMIN_ENABLED
class AdminCompletion : public Callback,
                        public std::enable_shared_from_this<AdminCompletion>

{
public:
    AdminCompletion(pos_io* posIo, IOCtx& ioContext);
    ~AdminCompletion(void) override;

private:
    bool _DoSpecificJob(void) override;
    pos_io* io;
    IOCtx& ioContext;
};
#endif
} // namespace pos
