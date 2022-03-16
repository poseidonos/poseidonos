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

#include "src/allocator/allocator.h"

#include <fstream>
#include <sstream>
#include <string>

#include "src/allocator/context_manager/allocator_ctx/allocator_ctx.h"
#include "src/allocator/context_manager/rebuild_ctx/rebuild_ctx.h"
#include "src/allocator/context_manager/segment_ctx/segment_ctx.h"
#include "src/allocator/i_context_manager.h"
#include "src/allocator_service/allocator_service.h"
#include "src/array_models/interface/i_array_info.h"
#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"
#include "src/meta_file_intf/mock_file_intf.h"
#include "src/sys_event/volume_event_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_publisher.h"
#include "src/telemetry/telemetry_client/telemetry_client.h"

namespace pos
{
Allocator::Allocator(TelemetryPublisher* tp_, AllocatorAddressInfo* addrInfo_, ContextManager* contextManager_, BlockManager* blockManager_,
    WBStripeManager* wbStripeManager_, IArrayInfo* info_, IStateControl* iState_)
: addrInfo(addrInfo_),
  contextManager(contextManager_),
  blockManager(blockManager_),
  wbStripeManager(wbStripeManager_),
  isInitialized(false),
  iArrayInfo(info_),
  iStateControl(iState_),
  tp(tp_),
  arrayName(info_->GetName())
{
}

Allocator::Allocator(IArrayInfo* info, IStateControl* iState)
: Allocator(nullptr, nullptr, nullptr, nullptr, nullptr, info, iState)
{
    _CreateSubmodules();
    POS_TRACE_INFO(EID(ALLOCATOR_START), "Allocator in Array:{} was Created", arrayName);
}

Allocator::~Allocator(void)
{
    _DeleteSubmodules();
    POS_TRACE_INFO(EID(ALLOCATOR_START), "Allocator in Array:{} was Destroyed", arrayName);
}

int
Allocator::Init(void)
{
    if (isInitialized == false)
    {
        if (tp != nullptr)
        {
            TelemetryClientSingleton::Instance()->RegisterPublisher(tp);
        }
        addrInfo->Init(iArrayInfo);
        contextManager->Init();
        blockManager->Init(wbStripeManager);
        wbStripeManager->Init();

        _RegisterToAllocatorService();
        isInitialized = true;
    }
    return 0;
}

void
Allocator::Dispose(void)
{
    POS_TRACE_INFO(EID(UNMOUNT_ARRAY_DEBUG_MSG), "[Allocator] Dispose, init:{}", isInitialized);
    if (isInitialized == true)
    {
        POS_TRACE_INFO(EID(UNMOUNT_ARRAY_DEBUG_MSG), "Start flushing all write buffer stripes");
        wbStripeManager->FlushAllWbStripes();

        POS_TRACE_INFO(EID(UNMOUNT_ARRAY_DEBUG_MSG), "Start disposing write buffer stripe manager");
        wbStripeManager->Dispose();

        contextManager->FlushContexts(nullptr, true);
        contextManager->Dispose();

        _UnregisterFromAllocatorService();
        if (tp != nullptr)
        {
            TelemetryClientSingleton::Instance()->DeregisterPublisher(tp->GetName());
        }
        isInitialized = false;
    }
}

void
Allocator::Shutdown(void)
{
    POS_TRACE_INFO(EID(UNMOUNT_BROKEN_ARRAY_DEBUG_MSG), "[Allocator] Shutdown, init:{}", isInitialized);
    if (isInitialized == true)
    {
        POS_TRACE_INFO(EID(UNMOUNT_BROKEN_ARRAY_DEBUG_MSG), "Start flushing all write buffer stripes");
        wbStripeManager->FlushAllWbStripes();

        POS_TRACE_INFO(EID(UNMOUNT_BROKEN_ARRAY_DEBUG_MSG), "Start disposing write buffer stripe manager");
        wbStripeManager->Dispose();

        contextManager->Dispose();
        _UnregisterFromAllocatorService();
        if (tp != nullptr)
        {
            TelemetryClientSingleton::Instance()->DeregisterPublisher(tp->GetName());
        }
        isInitialized = false;
    }
}

void
Allocator::Flush(void)
{
    // no-op for IMountSequence
}

IBlockAllocator*
Allocator::GetIBlockAllocator(void)
{
    return blockManager;
}

IWBStripeAllocator*
Allocator::GetIWBStripeAllocator(void)
{
    return wbStripeManager;
}

IAllocatorWbt*
Allocator::GetIAllocatorWbt(void)
{
    return this;
}

IContextManager*
Allocator::GetIContextManager(void)
{
    return contextManager;
}

IContextReplayer*
Allocator::GetIContextReplayer(void)
{
    return (IContextReplayer*)contextManager->GetContextReplayer();
}

ISegmentCtx*
Allocator::GetISegmentCtx(void)
{
    return contextManager->GetISegmentCtx();
}

int
Allocator::PrepareRebuild(void)
{
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Start @PrepareRebuild()");
    blockManager->TurnOffBlkAllocation();

    int ret = 0;

    ret = wbStripeManager->FlushAllWbStripes();
    if (ret != 0)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Stripes flush failed, ret {}", ret);
        blockManager->TurnOnBlkAllocation();
        return ret;   
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Stripes Flush Done @PrepareRebuild()");

    // TODO (meta) Make sure all nvram segments are force changed to ssd state
    // std::set<SegmentId> nvramSegments;
    // nvramSegments = contextManager->GetNvramSegmentList();

    ret = contextManager->MakeRebuildTargetSegmentList();
    if (ret != 0)
    {
        if (ret == (int)POS_EVENT_ID::ALLOCATOR_REBUILD_TARGET_SET_EMPTY)
        {
            // No segments to rebuild, complete rebuild
            POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "No segments to rebuild");
            ret = 0;
        }
        else
        {
            // Error occured or there's no segments to rebuild
            POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Failed to flush rebuild target list");
        }

        blockManager->TurnOnBlkAllocation();
        return ret;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "MakeRebuildTarget Done @PrepareRebuild()");

    ret = contextManager->SetNextSsdLsid();
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "Failed to get next segment");
        blockManager->TurnOnBlkAllocation();
        return ret;
    }
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "SetNextSsdLsid Done @PrepareRebuild()");

    blockManager->TurnOnBlkAllocation();
    POS_TRACE_INFO(EID(ALLOCATOR_MAKE_REBUILD_TARGET), "End @PrepareRebuild() with return {}", ret);

    return ret;
}

void
Allocator::SetNormalGcThreshold(uint32_t inputThreshold)
{
    contextManager->GetGcCtx()->SetNormalGcThreshold(inputThreshold);
}

void
Allocator::SetUrgentThreshold(uint32_t inputThreshold)
{
    contextManager->GetGcCtx()->SetUrgentThreshold(inputThreshold);
}

int
Allocator::GetMeta(WBTAllocatorMetaType type, std::string fname, MetaFileIntf* file)
{
    MetaFileIntf* dumpFile = new MockFileIntf(fname, iArrayInfo->GetIndex());
    if (file != nullptr)
    {
        delete dumpFile;
        dumpFile = file;
    }

    int ret = dumpFile->Create(0);
    if (ret < 0)
    {
        POS_TRACE_ERROR(EID(ALLOCATOR_START), "WBT failed to open output file {}", fname);
        return -EID(ALLOCATOR_START);
    }

    dumpFile->Open();
    uint64_t curOffset = 0;
    if ((WBT_SEGMENT_VALID_COUNT == type) || (WBT_SEGMENT_OCCUPIED_STRIPE == type))
    {
        uint32_t len = sizeof(uint32_t) * addrInfo->GetnumUserAreaSegments();
        char* buf = new char[len]();
        SegmentCtx* segCtx = contextManager->GetSegmentCtx();
        segCtx->CopySegmentInfoToBufferforWBT(type, buf);
        ret = dumpFile->IssueIO(MetaFsIoOpcode::Write, 0, len, buf);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE), "WBT Sync Write(SegmentInfo) to {} Failed, ret:{}", fname, ret);
            ret = -EID(ALLOCATOR_META_ARCHIVE_STORE);
        }
        delete[] buf;
    }
    else
    {
        if (WBT_NUM_ALLOCATOR_META <= type)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE), "WBT wrong alloctor meta type, type:{}", type);
            ret = -EID(ALLOCATOR_META_ARCHIVE_STORE);
        }
        else
        {
            ret = dumpFile->AppendIO(MetaFsIoOpcode::Write, curOffset, contextManager->GetContextSectionSize(ALLOCATOR_CTX, type + 1), contextManager->GetContextSectionAddr(ALLOCATOR_CTX, type + 1));
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_STORE), "WBT Sync Write(allocatorCtx) to {} Failed, ret:{}", fname, ret);
                ret = -EID(ALLOCATOR_META_ARCHIVE_STORE);
            }
        }
    }

    dumpFile->Close();
    delete dumpFile;
    return ret;
}

int
Allocator::SetMeta(WBTAllocatorMetaType type, std::string fname, MetaFileIntf* file)
{
    MetaFileIntf* fileProvided = new MockFileIntf(fname, iArrayInfo->GetIndex());
    if (file != nullptr)
    {
        delete fileProvided;
        fileProvided = file;
    }

    int ret = 0;
    fileProvided->Open();
    uint64_t curOffset = 0;
    if ((WBT_SEGMENT_VALID_COUNT == type) || (WBT_SEGMENT_OCCUPIED_STRIPE == type))
    {
        uint32_t len = sizeof(uint32_t) * addrInfo->GetnumUserAreaSegments();
        char* buf = new char[len]();
        ret = fileProvided->AppendIO(MetaFsIoOpcode::Read, curOffset, len, buf);
        if (ret < 0)
        {
            POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_LOAD), "WBT Sync Read(SegmentInfo) from {} Failed, ret:{}", fname, ret);
            ret = -EID(ALLOCATOR_META_ARCHIVE_LOAD);
        }
        else
        {
            SegmentCtx* segCtx = contextManager->GetSegmentCtx();
            segCtx->CopySegmentInfoFromBufferforWBT(type, buf);
        }
        delete[] buf;
    }
    else
    {
        if (WBT_WBLSID_BITMAP == type)
        {
            uint32_t numBitsSet = 0;
            ret = fileProvided->AppendIO(MetaFsIoOpcode::Read, curOffset, sizeof(numBitsSet), (char*)&numBitsSet);
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_LOAD), "WBT Sync Read(wblsid bitmap) from {} Failed, ret:{}", fname, ret);
                ret = -EID(ALLOCATOR_META_ARCHIVE_LOAD);
            }
            else
            {
                AllocatorCtx* allocCtx = contextManager->GetAllocatorCtx();
                allocCtx->SetAllocatedWbStripeCount(numBitsSet);
            }
        }
        // ACTIVE_STRIPE_TAIL, CURRENT_SSD_LSID, SEGMENT_STATE
        else
        {
            ret = fileProvided->AppendIO(MetaFsIoOpcode::Read, curOffset, contextManager->GetContextSectionSize(ALLOCATOR_CTX, type + 1), contextManager->GetContextSectionAddr(ALLOCATOR_CTX, type + 1));
            if (ret < 0)
            {
                POS_TRACE_ERROR(EID(ALLOCATOR_META_ARCHIVE_LOAD), "WBT Sync Read(allocatorCtx) from {} Failed, ret:{}", fname, ret);
                ret = -EID(ALLOCATOR_META_ARCHIVE_LOAD);
            }
        }
    }

    fileProvided->Close();
    delete fileProvided;
    return ret;
}

int
Allocator::GetInstantMetaInfo(std::string fname)
{
    std::ostringstream oss;
    std::ofstream ofs(fname, std::ofstream::app);
    AllocatorCtx* allocCtx = contextManager->GetAllocatorCtx();
    SegmentCtx* segCtx = contextManager->GetSegmentCtx();
    oss << "<< WriteBuffers >>" << std::endl;
    oss << "Set:" << std::dec << allocCtx->GetAllocatedWbStripeCount() << " / ToTal:" << allocCtx->GetNumTotalWbStripe() << std::endl;
    oss << "activeStripeTail[] Info" << std::endl;
    for (int volumeId = 0; volumeId < MAX_VOLUME_COUNT; ++volumeId)
    {
        for (int idx = volumeId; idx < ACTIVE_STRIPE_TAIL_ARRAYLEN; idx += MAX_VOLUME_COUNT)
        {
            VirtualBlkAddr asTail = allocCtx->GetActiveStripeTail(idx);
            oss << "Idx:" << std::dec << idx << " stripeId:0x" << std::hex << asTail.stripeId << " offset:0x" << asTail.offset << "  ";
        }
        oss << std::endl;
    }
    oss << std::endl;

    oss << "<< Segments >>" << std::endl;
    oss << "Set:" << std::dec << segCtx->GetAllocatedSegmentCount() << " / ToTal:" << addrInfo->GetnumUserAreaSegments() << std::endl;
    oss << "currentSsdLsid: " << allocCtx->GetCurrentSsdLsid() << std::endl;
    for (uint32_t segmentId = 0; segmentId < addrInfo->GetnumUserAreaSegments(); ++segmentId)
    {
        SegmentState state = segCtx->GetSegmentState(segmentId);
        if ((segmentId > 0) && (segmentId % 4 == 0))
        {
            oss << std::endl;
        }
        oss << "SegmentId:" << segmentId << " state:" << static_cast<int>(state) << " ValidBlockCnt:" << segCtx->GetValidBlockCount(segmentId) << "  ";
    }
    oss << std::endl
        << std::endl;

    oss << "<< Rebuild >>" << std::endl;
    oss << "NeedRebuildCont:" << std::boolalpha << contextManager->NeedRebuildAgain() << std::endl;
    oss << "TargetSegmentCount:" << segCtx->GetRebuildTargetSegmentCount() << std::endl;
    oss << "TargetSegnent ID" << std::endl;
    int cnt = 0;
    std::set<SegmentId> segmentList = segCtx->GetRebuildSegmentList();
    for (RTSegmentIter iter = segmentList.begin(); iter != segmentList.end(); ++iter, ++cnt)
    {
        if (cnt > 0 && (cnt % 16 == 0))
        {
            oss << std::endl;
        }
        oss << *iter << " " << std::endl;
    }
    oss << std::endl;

    ofs << oss.str();
    return 0;
}

int
Allocator::GetBitmapLayout(std::string fname)
{
    std::ofstream ofs(fname, std::ofstream::app);

    ofs << "numWbStripe: 0x" << std::hex << addrInfo->GetnumWbStripes() << std::endl;
    ofs << "numUserAreaSegment: 0x" << std::hex << addrInfo->GetnumUserAreaSegments() << std::endl;
    ofs << "numUserAreaStripes: 0x" << std::hex << addrInfo->GetnumUserAreaStripes()
        << std::endl;
    ofs << "blksPerStripe: 0x" << std::hex << addrInfo->GetblksPerStripe() << std::endl;
    ofs << "ValidBlockCountSize: 0x" << std::hex << sizeof(uint32_t) << std::endl;
    ofs << std::endl;

    return 0;
}

void
Allocator::FlushAllUserdataWBT(void)
{
    std::vector<Stripe*> stripesToFlush;
    std::vector<StripeId> vsidToCheckFlushDone;

    blockManager->TurnOffBlkAllocation();
    wbStripeManager->FlushAllWbStripes();
    blockManager->TurnOnBlkAllocation();
}

void
Allocator::_CreateSubmodules(void)
{
    addrInfo = new AllocatorAddressInfo();
    std::string arrName = iArrayInfo->GetName();
    tp = new TelemetryPublisher(("Allocator"));
    tp->AddDefaultLabel("array_name", arrName);
    contextManager = new ContextManager(tp, addrInfo, iArrayInfo->GetIndex());
    blockManager = new BlockManager(tp, addrInfo, contextManager, iArrayInfo->GetIndex(), iArrayInfo->IsWriteThroughEnabled());
    wbStripeManager = new WBStripeManager(tp, addrInfo, contextManager, blockManager, arrayName, iArrayInfo->GetIndex());
}

void
Allocator::_DeleteSubmodules(void)
{
    if (wbStripeManager != nullptr)
    {
        delete wbStripeManager;
        wbStripeManager = nullptr;
    }
    if (blockManager != nullptr)
    {
        delete blockManager;
        blockManager = nullptr;
    }
    if (contextManager != nullptr)
    {
        delete contextManager;
        contextManager = nullptr;
    }
    if (tp != nullptr)
    {
        delete tp;
        tp = nullptr;
    }
    if (addrInfo != nullptr)
    {
        delete addrInfo;
        addrInfo = nullptr;
    }
}

void
Allocator::_RegisterToAllocatorService(void)
{
    AllocatorService* allocatorService = AllocatorServiceSingleton::Instance();
    allocatorService->RegisterAllocator(arrayName, iArrayInfo->GetIndex(), GetIBlockAllocator(),
        GetIWBStripeAllocator(), GetIAllocatorWbt(), GetIContextManager(), GetIContextReplayer());
}

void
Allocator::_UnregisterFromAllocatorService(void)
{
    AllocatorService* allocatorService = AllocatorServiceSingleton::Instance();
    allocatorService->UnregisterAllocator(arrayName);
}

} // namespace pos
