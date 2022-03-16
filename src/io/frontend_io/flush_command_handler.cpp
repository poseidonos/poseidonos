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

#include "src/io/frontend_io/flush_command_handler.h"

#include "src/allocator/i_context_manager.h"
#include "src/allocator/i_block_allocator.h"
#include "src/allocator/i_wbstripe_allocator.h"
#include "src/event_scheduler/io_completer.h"
#include "src/logger/logger.h"
#include "src/mapper_service/mapper_service.h"
#include "src/spdk_wrapper/event_framework_api.h"

namespace pos
{
FlushCmdHandler::FlushCmdHandler(FlushIoSmartPtr flushIo)
: FlushCmdHandler(flushIo, FlushCmdManagerSingleton::Instance(),
      AllocatorServiceSingleton::Instance()->GetIBlockAllocator(flushIo->GetArrayId()),
      AllocatorServiceSingleton::Instance()->GetIWBStripeAllocator(flushIo->GetArrayId()),
      AllocatorServiceSingleton::Instance()->GetIContextManager(flushIo->GetArrayId()),
      MapperServiceSingleton::Instance()->GetIMapFlush(flushIo->GetArrayId()))
{
}

FlushCmdHandler::FlushCmdHandler(FlushIoSmartPtr flushIo, FlushCmdManager* flushCmdManager,
    IBlockAllocator* iBlockAllocator, IWBStripeAllocator* iWBStripeAllocator,
    IContextManager* ctxManager, IMapFlush* iMapFlush)
: flushCmdManager(flushCmdManager),
  iWBStripeAllocator(iWBStripeAllocator),
  iBlockAllocator(iBlockAllocator),
  icontextManager(ctxManager),
  iMapFlush(iMapFlush),
  flushIo(flushIo),
  volumeId(flushIo->GetVolumeId()),
  stripeMapFlushIssued(false)
{
}

FlushCmdHandler::~FlushCmdHandler(void)
{
}

bool
FlushCmdHandler::Execute(void)
{
    if (flushCmdManager->IsFlushEnabled() == false)
    {
        IoCompleter ioCompleter(flushIo);
        ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
        return true;
    }

    switch (flushIo->GetState())
    {
        case FLUSH__BLOCKING_ALLOCATION:
            if (flushCmdManager->TrySetFlushInProgress(volumeId) == false)
            {
                return false;
            }

            // Block allocation for new writes
            if (iBlockAllocator->BlockAllocating(volumeId) == false)
            {
                // Flush handler on volumeId already in progress
                return false;
            }
            POS_TRACE_DEBUG((int)POS_EVENT_ID::FLUSH_CMD_ONGOING,
                "Flush command started on volume {}", volumeId);

            // Trigger stripe flush of all active stripes, and get all wb stripes belonging to input volumeId
            iWBStripeAllocator->FlushAllPendingStripesInVolume(volumeId, flushIo);

            // Unblock allocation
            iBlockAllocator->UnblockAllocating(volumeId);
            flushIo->StripesScanComplete();

            flushIo->SetState(FLUSH__VSAMAP);
        case FLUSH__VSAMAP:
            // Check is all stripes flush are complete
            if (flushIo->IsStripesFlushComplete() == false)
            {
                return false;
            }
            POS_TRACE_DEBUG((int)POS_EVENT_ID::FLUSH_CMD_ONGOING,
                "All stripes flush completed");

            POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
                POS_EVENT_ID::FLUSH_CMD_ONGOING,
                "User data flush for volume {} completed", volumeId);

        {
            int ret = 0;
            EventSmartPtr eventVSAMap(new MapFlushCompleteEvent(volumeId, flushIo));
            ret = iMapFlush->FlushDirtyMpages(volumeId, eventVSAMap);
            if (ret != 0)
            {
                if (ret == -EID(MAP_FLUSH_IN_PROGRESS))
                {
                    return false;
                }
                else
                {
                    POS_TRACE_ERROR((int)POS_EVENT_ID::FLUSH_CMD_MAPPER_FLUSH_FAILED,
                            "Failed to start flushing dirty vsamap pages");
                    if (flushIo->IsInternalFlush() == false)
                    {
                        flushIo->GetCallback()->InformError(IOErrorType::GENERIC_ERROR);
                        IoCompleter ioCompleter(flushIo);
                        ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
                        iBlockAllocator->UnblockAllocating(volumeId);
                    }
                    flushCmdManager->ResetFlushInProgress(volumeId, flushIo->IsInternalFlush());
                    return true;
                }
            }
            POS_TRACE_DEBUG((int)POS_EVENT_ID::FLUSH_CMD_ONGOING,
                "VSA Map flush requested");
        }

            flushIo->SetState(FLUSH__STRIPEMAP_ALLOCATOR);

            if (flushCmdManager->CanFlushMeta(flushIo) == false)
            {
                return true;
            }
        case FLUSH__STRIPEMAP_ALLOCATOR:
            {
                int ret = 0;
                bool stripeMapFlushFailed = false;

                if (stripeMapFlushIssued != true)
                {
                    // Stripe Map Flush
                    EventSmartPtr eventStripeMap(new MapFlushCompleteEvent(STRIPE_MAP_ID, flushIo));
                    ret = iMapFlush->FlushDirtyMpages(STRIPE_MAP_ID, eventStripeMap);

                    if (ret != 0)
                    {
                        if (ret == -EID(MAP_FLUSH_IN_PROGRESS))
                        {
                            return false;
                        }
                        else
                        {
                            POS_TRACE_ERROR((int)POS_EVENT_ID::FLUSH_CMD_MAPPER_FLUSH_FAILED,
                                    "Failed to start flushing dirty stripe map pages. Error code {}", ret);
                            if (flushIo->IsInternalFlush() == false)
                            {
                                flushIo->GetCallback()->InformError(IOErrorType::GENERIC_ERROR);
                            }
                            flushIo->SetStripeMapFlushComplete(true);
                            stripeMapFlushFailed = true;
                        }
                    }
                    stripeMapFlushIssued = true;

                    POS_TRACE_DEBUG((int)POS_EVENT_ID::FLUSH_CMD_ONGOING,
                            "Stripe Map flush requested");
                }

                if (stripeMapFlushFailed == false)
                {
                    // Allocator Flush
                    EventSmartPtr callbackAllocator(new AllocatorFlushDoneEvent(flushIo));
                    ret = icontextManager->FlushContexts(callbackAllocator, false);
                    if (ret != 0)
                    {
                        if (ret == (int)POS_EVENT_ID::ALLOCATOR_META_ARCHIVE_FLUSH_IN_PROGRESS)
                        {
                            return false;
                        }

                        POS_TRACE_ERROR((int)POS_EVENT_ID::FLUSH_CMD_ALLOCATOR_FLUSH_FAILED,
                                "Failed to start flushing allocator meta pages. Error code {}", ret);
                        if (flushIo->IsInternalFlush() == false)
                        {
                            flushIo->GetCallback()->InformError(IOErrorType::GENERIC_ERROR);
                        }
                        flushIo->SetAllocatorFlushComplete(true);
                    }
                }
                else
                {
                    flushIo->SetAllocatorFlushComplete(true);
                }

                POS_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
                        POS_EVENT_ID::FLUSH_CMD_ONGOING,
                        "Meta data flush for volume {} is requested", volumeId);
            }

            flushIo->SetState(FLUSH__META_FLUSH_IN_PROGRESS);
        case FLUSH__META_FLUSH_IN_PROGRESS:
            if (flushIo->IsStripeMapAllocatorFlushComplete() == false)
            {
                if (flushIo->IsStripeMapFlushComplete() == true &&
                    flushIo->IsAllocatorFlushComplete() == true)
                {
                    flushCmdManager->FinishMetaFlush();
                    flushIo->SetStripeMapAllocatorFlushComplete(true);
                }
                else
                {
                    return false;
                }
            }

            if (flushIo->IsVsaMapFlushComplete() == false)
            {
                return false;
            }

            // Call Complete
            if (flushIo->IsInternalFlush() == false)
            {
                IoCompleter ioCompleter(flushIo);
                ioCompleter.CompleteUbio(IOErrorType::SUCCESS, true);
            }

            flushCmdManager->ResetFlushInProgress(volumeId, flushIo->IsInternalFlush());

            POS_TRACE_INFO(EID(SUCCESS), "NVMe Flush Command Complete");
        default:
            break;
    }

    return true;
}

MapFlushCompleteEvent::MapFlushCompleteEvent(int mapId, FlushIoSmartPtr flushIo)
: mapId(mapId),
  flushIo(flushIo)
{
}

MapFlushCompleteEvent::~MapFlushCompleteEvent(void)
{
}

bool
MapFlushCompleteEvent::Execute(void)
{
    if (mapId == STRIPE_MAP_ID)
    {
        flushIo->SetStripeMapFlushComplete(true);
    }
    else
    {
        flushIo->SetVsaMapFlushComplete(true);
    }

    return true;
}

AllocatorFlushDoneEvent::AllocatorFlushDoneEvent(FlushIoSmartPtr flushIo)
: flushIo(flushIo)
{
}

AllocatorFlushDoneEvent::~AllocatorFlushDoneEvent(void)
{
}

bool
AllocatorFlushDoneEvent::Execute(void)
{
    flushIo->SetAllocatorFlushComplete(true);
    return true;
}

} // namespace pos
