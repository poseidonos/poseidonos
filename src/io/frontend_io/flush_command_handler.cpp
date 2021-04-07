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

#include "src/io/frontend_io/flush_command_handler.h"

#include "src/device/event_framework_api.h"
#include "src/logger/logger.h"

namespace ibofos
{
#if defined NVMe_FLUSH_HANDLING
FlushCmdHandler::FlushCmdHandler(VolumeIoSmartPtr volumeIo)
: allocator(AllocatorSingleton::Instance()),
  mapper(MapperSingleton::Instance()),
  volumeIo(volumeIo),
  volumeId(volumeIo->GetVolumeId()),
  flushCmdManager(FlushCmdManagerSingleton::Instance())
{
    hasLock = false;
    totalMapsCompleted = 0;
    totalMapsToFlush = 0;
    mapperFlushComplete = false;
    allocaterFlushComplete = false;
    // TODO: To be changed when multi volumes flush is handled
    numVSAMapsForFlushing = 1;
    numStripeMapsForFlushing = 1;
}

FlushCmdHandler::~FlushCmdHandler(void)
{
}

bool
FlushCmdHandler::Execute()
{
    FlushState currentState = flushCmdManager->GetState();

    // Check if present flush event is in progress
    if (currentState != FLUSH_RESET && hasLock == false)
    {
        return false;
    }

    switch (currentState)
    {
        case FLUSH_RESET:
            // Acquire flush lock. Only one flush can be in progress.
            if (false == flushCmdManager->LockIo())
            {
                return false;
            }
            flushCmdManager->SetState(FLUSH_START);
            hasLock = true;
        case FLUSH_START:
            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "Flush command started on volume {}", volumeId);

            // Block allocation for new writes and get all active stripes for flush
            allocator->GetAllActiveStripes(volumeId);
            flushCmdManager->SetState(FLUSH_PENDING_WRITE_IN_PROGRESS);
        case FLUSH_PENDING_WRITE_IN_PROGRESS:
            // Check if all writes on partial stripes are complete and issue flush on them.
            if (allocator->FlushPartialStripes() == false)
            {
                // Pending IO(s) on active stripes not complete.
                // Put flush event back to SPDK reactor queue and return
                return false;
            }

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "Flush partial stripes requested");

            flushCmdManager->SetState(FLUSH_ACTIVE_STRIPE_FLUSH_IN_PROGRESS);
        case FLUSH_ACTIVE_STRIPE_FLUSH_IN_PROGRESS:
            // Check if all partial stripes flush complete
            if (allocator->WaitForPartialStripesFlush() == false)
            {
                // Partial stripe(s) flush not complete.
                // Put flush event back to SPDK reactor queue and return
                return false;
            }

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "Partial stripes flush completed");

            flushCmdManager->SetState(FLUSH_ALL_STRIPE_FLUSH_IN_PROGRESS);
        case FLUSH_ALL_STRIPE_FLUSH_IN_PROGRESS:
            // Check is all stripes flush are complete
            if (allocator->WaitForAllStripesFlush(volumeId) == false)
            {
                // All stripe(s) flush not complete.
                // Put flush event back to SPDK reactor queue and return
                return false;
            }

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "All stripes flush completed");

            IBOF_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
                IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "User data flush for volume {} completed", volumeId);

            flushCmdManager->SetState(FLUSH_STRIPE_FLUSH_COMPLETE);
        case FLUSH_STRIPE_FLUSH_COMPLETE:
        {
            // Mapper Flush
            // TODO change when all volume flush to be handled
            int ret;
            int mapId;

            // Flush volume specific map and stripe map
            totalMapsToFlush = numVSAMapsForFlushing + numStripeMapsForFlushing;

            mapId = volumeId;
            EventSmartPtr callbackVSAMap(new MapFlushCompleteEvent(volumeId, this));
            ret = mapper->FlushMap(volumeId, callbackVSAMap);
            if (ret != 0)
            {
                IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::FLUSH_CMD_MAPPER_FLUSH_FAILED,
                    "Failed to start flushing dirty vsamap pages");
                volumeIo->GetCallback()->InformError(CallbackError::GENERIC_ERROR);
                totalMapsToFlush--;
            }

            mapId = STRIPE_MAP_ID;
            EventSmartPtr callbackStripeMap(new MapFlushCompleteEvent(mapId, this));
            ret = mapper->FlushMap(STRIPE_MAP_ID, callbackStripeMap);
            if (ret != 0)
            {
                IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::FLUSH_CMD_MAPPER_FLUSH_FAILED,
                    "Failed to start flushing dirty stripe map pages");
                volumeIo->GetCallback()->InformError(CallbackError::GENERIC_ERROR);
                totalMapsToFlush--;
            }

            if (totalMapsToFlush == 0)
            {
                mapperFlushComplete = true;
            }

            IBOF_TRACE_DEBUG((int)IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "Map flush requested (num maps to flush: {})", totalMapsToFlush);

            // Allocator Flush
            EventSmartPtr callbackAllocator(new AllocatorFlushDoneEvent(this));
            ret = allocator->FlushMetadata(callbackAllocator);
            if (ret != 0)
            {
                IBOF_TRACE_ERROR((int)IBOF_EVENT_ID::FLUSH_CMD_ALLOCATOR_FLUSH_FAILED,
                    "Failed to start flushing allocator meta pages");
                volumeIo->GetCallback()->InformError(CallbackError::GENERIC_ERROR);
                allocaterFlushComplete = true;
            }

            // Unblock allocation
            allocator->UnblockAllocating();
            flushCmdManager->SetState(FLUSH_META_FLUSH_IN_PROGRESS);

            IBOF_TRACE_INFO_IN_MEMORY(ModuleInDebugLogDump::FLUSH_CMD,
                IBOF_EVENT_ID::FLUSH_CMD_ONGOING,
                "Meta data flush for volume {} is requested", volumeId);
        }
        case FLUSH_META_FLUSH_IN_PROGRESS:
            if (mapperFlushComplete && allocaterFlushComplete)
            {
                flushCmdManager->SetState(FLUSH_META_FLUSH_COMPLETE);
            }
            else
            {
                return false;
            }
        case FLUSH_META_FLUSH_COMPLETE:
            flushCmdManager->SetState(FLUSH_RESET);
            flushCmdManager->UnlockIo();

            // Call Complete
            volumeIo->CompleteWithoutRecovery(CallbackError::SUCCESS);

            IBOF_TRACE_INFO((int)IBOF_EVENT_ID::SUCCESS, "NVMe Flush Command Complete");
        default:
            break;
    }

    return true;
}

void
FlushCmdHandler::MapFlushCompleted(int mapId)
{
    std::unique_lock<std::mutex> lock(mapCountUpdateLock);
    if (++totalMapsCompleted == totalMapsToFlush)
    {
        mapperFlushComplete = true;
    }
}

void
FlushCmdHandler::AllocatorMetaFlushCompleted()
{
    allocaterFlushComplete = true;
}

MapFlushCompleteEvent::MapFlushCompleteEvent(int mapId, FlushCmdHandler* flushCmdHandler)
: mapId(mapId),
  flushCmdHandler(flushCmdHandler)
{
}

MapFlushCompleteEvent::~MapFlushCompleteEvent(void)
{
}

bool
MapFlushCompleteEvent::Execute(void)
{
    flushCmdHandler->MapFlushCompleted(mapId);
    return true;
}

AllocatorFlushDoneEvent::AllocatorFlushDoneEvent(FlushCmdHandler* flushCmdHandler)
: flushCmdHandler(flushCmdHandler)
{
}

AllocatorFlushDoneEvent::~AllocatorFlushDoneEvent(void)
{
}

bool
AllocatorFlushDoneEvent::Execute(void)
{
    flushCmdHandler->AllocatorMetaFlushCompleted();
    return true;
}

#endif
} // namespace ibofos
