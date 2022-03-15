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

#include <array>
#include <list>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "src/bio/volume_io.h"
#include "src/event_scheduler/event.h"
#include "src/include/pos_event_id.hpp"
#include "src/io/general_io/io_controller.h"
#include "src/lib/block_alignment.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
using VirtualBlksInfo = std::pair<VirtualBlks, StripeId>;
using VirtualBlkAddrInfo = std::pair<VirtualBlkAddr, StripeId>;
class EventArgument;
class VolumeIo;
class RBAStateManager;
class IBlockAllocator;
class FlowControl;

class WriteSubmission : public IOController, public Event
{
public:
    explicit WriteSubmission(VolumeIoSmartPtr volumeIo);
    WriteSubmission(VolumeIoSmartPtr volumeIo, RBAStateManager* rbaStateManager, IBlockAllocator* iBlockAllocator, FlowControl* flowControl,
        bool isReactorNow);
    ~WriteSubmission(void) override;

    bool Execute(void) override;

private:
    VolumeIoSmartPtr volumeIo;
    uint32_t volumeId;
    BlockAlignment blockAlignment;
    uint32_t blockCount;
    uint32_t allocatedBlockCount;
    std::list<VirtualBlksInfo> allocatedVirtualBlks;
    uint32_t processedBlockCount;
    std::queue<VolumeIoSmartPtr> splitVolumeIoQueue;
    RBAStateManager* rbaStateManager;
    IBlockAllocator* iBlockAllocator;
    FlowControl* flowControl;

    void _SendVolumeIo(VolumeIoSmartPtr volumeIo);
    bool _ProcessOwnedWrite(void);
    void _AllocateFreeWriteBuffer(void);
    void _ReadOldBlock(BlkAddr rba, VirtualBlkAddrInfo& vsaInfo, bool isTail);
    void _AddVirtualBlks(VirtualBlksInfo& virtualBlks);
    VolumeIoSmartPtr _CreateVolumeIo(VirtualBlksInfo& virtualBlksInfo);
    void _PrepareSingleBlock(VirtualBlksInfo& virtualBlksInfo);
    void _ReadOldHeadBlock(void);
    void _ReadOldTailBlock(void);
    void _PrepareBlockAlignment(void);
    void _WriteDataAccordingToVsaRange(VirtualBlksInfo& virtualBlksInfo);
    void _WriteSingleBlock(void);
    void _WriteMultipleBlocks(void);
    void _SubmitVolumeIo(void);
    VirtualBlkAddrInfo _PopHeadVsa(void);
    VirtualBlkAddrInfo _PopTailVsa(void);
    void _SetupVolumeIo(VolumeIoSmartPtr newVolumeIo, VirtualBlksInfo& virtualBlksInfo,
        CallbackSmartPtr callback);
};
} // namespace pos
