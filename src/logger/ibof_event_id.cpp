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

#include "src/include/ibof_event_id.hpp"

#include "logger.h"

namespace ibofos
{
IbofEventId::IbofEventIdEntry
    IbofEventId::RESERVED_EVENT_ENTRY = {IBOF_EVENT_ID::RESERVED, "Reserved, Not defined, yet!!"};

IbofEventId::IbofEventIdEntry
    IbofEventId::IOPATH_NVMF_EVENT_ENTRY[(uint32_t)IBOF_EVENT_ID::IONVMF_COUNT] =
        {
            {IBOF_EVENT_ID::IONVMF_1H, "RDMA Invalid Private Data Length"}, // 5000
            {IBOF_EVENT_ID::IONVMF_2H, "RDMA Invalid RECFMT"},
            {IBOF_EVENT_ID::IONVMF_3H, "RDMA Invalid QID"},
            {IBOF_EVENT_ID::IONVMF_4H, "RDMA Invalid HSQSIZE"},
            {IBOF_EVENT_ID::IONVMF_5H, "RDMA Invalid HRQSIZE"},
            {IBOF_EVENT_ID::IONVMF_6H, "RDMA No Resources"},
            {IBOF_EVENT_ID::IONVMF_7H, "RDMA Invalid IRD"},
            {IBOF_EVENT_ID::IONVMF_8H, "RDMA Invalid ORD"},
            {IBOF_EVENT_ID::IONVMF_NAMESPACE_ATTACH_FAILED, "Fail to attach Namespace "},
};

IbofEventId::IbofEventIdEntry
    IbofEventId::IOPATH_FRONTEND_EVENT_ENTRY[(uint32_t)IBOF_EVENT_ID::IOFRONTEND_COUNT] =
        {
            {IBOF_EVENT_ID::AFTMGR_CPU_COUNT_NOT_ENOUGH, "Unssatisfied CPU Count"}, // 5100
            {IBOF_EVENT_ID::AFTMGR_DISABLED_CORE, "Core {} is disabled"},
            {IBOF_EVENT_ID::AFTMGR_FAIL_TO_FIND_MASTER_REACTOR, "Fail to find master reactor"},
            {IBOF_EVENT_ID::AFTMGR_FAIL_TO_ALLOCATE_ALL_CPU, "All core is not assigned for Ibofos"},
            {IBOF_EVENT_ID::AFTMGR_FAIL_TO_OVERLAP_MASK, "Core mask is overlapped"},
            {IBOF_EVENT_ID::AFTMGR_FAIL_TO_PARSING_ERROR, "Cpu allowed list is wrongly set"},
            {IBOF_EVENT_ID::AFTMGR_NO_EVENT_WORKER_ALLOCATED, "Cannot find event worker at any NUMA"},
            {IBOF_EVENT_ID::AFTMGR_NO_USE_CONFIG, "Use core description from default value"},
            {IBOF_EVENT_ID::AFTMGR_USE_CONFIG, "Use core description from config file"},

            {IBOF_EVENT_ID::AIO_CONTEXT_NOT_FOUND, "AIO context is not found"},
            {IBOF_EVENT_ID::AIO_FAIL_TO_ALLOCATE_EVENT, "Event allocation is failed at AIO completion"},
            {IBOF_EVENT_ID::AIO_FAIL_TO_ALLOCATE_MEMORY, "Not sufficient memory at AIO"},
            {IBOF_EVENT_ID::AIO_INVALID_AIO_CONTEXT, "AIO context is null at AIO completion"},
            {IBOF_EVENT_ID::AIO_INVALID_AIO_PRIVATE, "AIOPrivate is null at AIO completion"},
            {IBOF_EVENT_ID::AIO_INVALID_BDEV_IO, "Bdev IO is null at AIO completion"},
            {IBOF_EVENT_ID::AIO_INVALID_UBIO, "Ubio is null at AIO completion"},
            {IBOF_EVENT_ID::AIO_IO_FROM_INVALID_THREAD, "Only reactor can submit IO"},
            {IBOF_EVENT_ID::AIO_DEBUG_COMPLETION, "Aio completion : Rba {}, Size {}, readwrite {}, errortype {}"},
            {IBOF_EVENT_ID::AIO_DEBUG_SUBMISSION, "Aio submission : Rba {}, Size {}, readwrite {}"},
            {IBOF_EVENT_ID::AIO_FLUSH_START, "Flush Start in Aio, volume id : {}"},
            {IBOF_EVENT_ID::AIO_FLUSH_END, "Flush End in Aio, volume id : {}"},

            {IBOF_EVENT_ID::BLKALGN_INVALID_UBIO, "Block aligning Ubio is null"},

            {IBOF_EVENT_ID::BLKHDLR_FAIL_TO_ALLOCATE_EVENT, "Cannot allocate memory for event"},
            {IBOF_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION, "Wrong IO direction (only read/write types are suppoered)"},

            {IBOF_EVENT_ID::CALLBACK_INVALID_CALLEE, "Invalid callee for callback"},
            {IBOF_EVENT_ID::CALLBACK_INVALID_COUNT, "CompletionCount exceeds WaitingCount"},
            {IBOF_EVENT_ID::CALLBACK_TIMEOUT, "Callback Timeout. Caller : {}"},

            {IBOF_EVENT_ID::EVENTFRAMEWORK_FAIL_TO_ALLOCATE_EVENT, "Fail to allocate spdk event"},
            {IBOF_EVENT_ID::EVENTFRAMEWORK_INVALID_EVENT, "Invalid Event to send"},
            {IBOF_EVENT_ID::EVENTSCHEDULER_NOT_MATCH_WORKER_COUNT, "EventScheduler receives wrong worker count and cpu_set_t"},

            {IBOF_EVENT_ID::EVTARG_WRONG_ARGUMENT_ACCESS, "Request for getting wrong argument index"},
            {IBOF_EVENT_ID::EVTARG_WRONG_ARGUMENT_ADD, "Request for adding wrong argument index"},

            {IBOF_EVENT_ID::EVTQ_INVALID_EVENT, "Input event is null at EventQueue"},

            {IBOF_EVENT_ID::EVTSCHDLR_INVALID_WORKER_ID, "WorkerID is not valid at EventScheduler"},

            {IBOF_EVENT_ID::FLUSHREAD_DEBUG_SUBMIT, "Flush Read Submission StartLSA.stripeId : {} blocksInStripe : {}"},
            {IBOF_EVENT_ID::FLUSHREAD_FAIL_TO_ALLOCATE_MEMORY, "Fail to allocate memory"},
            {IBOF_EVENT_ID::FLUSH_DEBUG_SUBMIT, "Flush Submission vsid : {} StartLSA.stripeId : {} blocksInStripe : {}"},
            {IBOF_EVENT_ID::FLUSH_DEBUG_COMPLETION, "Flush Completion vsid : {} userstripeid : {} userArea : {}"},

            {IBOF_EVENT_ID::FREEBUFPOOL_FAIL_TO_ALLOCATE_MEMORY, "Fail to allocate memory"},

            {IBOF_EVENT_ID::IOAT_CONFIG_INVALID, "Invalid ioat count for numa"},

            {IBOF_EVENT_ID::IOATAPI_FAIL_TO_FINALIZE, "Fail to finalize IOAT"},
            {IBOF_EVENT_ID::IOATAPI_FAIL_TO_INITIALIZE, "Fail to initialize IOAT"},
            {IBOF_EVENT_ID::IOATAPI_FAIL_TO_SUBMIT_COPY, "Fail to request IOAT copy to reactor"},
            {IBOF_EVENT_ID::IOATAPI_DISABLED, "IOAT disabled"},
            {IBOF_EVENT_ID::IOATAPI_ENABLED, "IOAT enabled"},

            {IBOF_EVENT_ID::IODISPATCHER_INVALID_CPU_INDEX, "Invalid CPU index {}"},
            {IBOF_EVENT_ID::IODISPATCHER_INVALID_PARM, "Invalid Param in submit IO"},

            {IBOF_EVENT_ID::MERGER_INVALID_SPLIT_REQUESTED, "Requested Ubio index exceeds Ubio count of Merger"},

            {IBOF_EVENT_ID::RBAMGR_WRONG_VOLUME_ID, "Volume ID is not valid at RBAStateManager"},

            {IBOF_EVENT_ID::RDCMP_INVALID_ORIGIN_UBIO, "Origin Ubio is null at ReadCompleteteHandler"},
            {IBOF_EVENT_ID::RDCMP_INVALID_UBIO, "Ubio is null at ReadCompleteHandler"},
            {IBOF_EVENT_ID::RDCMP_READ_FAIL, "Uncorrectable data error"},

            {IBOF_EVENT_ID::RDHDLR_INVALID_UBIO, "Read Ubio is null"},
            {IBOF_EVENT_ID::RDHDLR_READ_DURING_REBUILD, "Data is read from which is under rebuild process"},

            {IBOF_EVENT_ID::SCHEDAPI_COMPLETION_POLLING_FAIL, "Fail to poll ibof completion"},
            {IBOF_EVENT_ID::SCHEDAPI_NULL_COMMAND, "Command from bdev is empty"},
            {IBOF_EVENT_ID::SCHEDAPI_SUBMISSION_FAIL, "Fail to submit ibof IO"},
            {IBOF_EVENT_ID::SCHEDAPI_WRONG_BUFFER, "Single IO command should have a continuous buffer"},

            {IBOF_EVENT_ID::STRIPE_INVALID_VOLUME_ID, "VolumeId is invalid"},

            {IBOF_EVENT_ID::REF_COUNT_RAISE_FAIL, "When Io Submit, refcount raise fail"},
            {IBOF_EVENT_ID::TRANSLATE_CONVERT_FAIL, "Translate() or Convert() is failed"},
            {IBOF_EVENT_ID::TRANSLATE_CONVERT_FAIL_IN_SYSTEM_STOP, "Translate() or Convert() is failed in system stop"},

            {IBOF_EVENT_ID::TRSLTR_INVALID_BLOCK_INDEX, "Block index exceeds block count at Translator"},
            {IBOF_EVENT_ID::TRSLTR_WRONG_ACCESS, "Only valid for single block Translator"},
            {IBOF_EVENT_ID::TRSLTR_WRONG_VOLUME_ID, "Volume ID is not valid at Translator"},

            {IBOF_EVENT_ID::VOLUMEIO_DEBUG_SUBMIT, "Volume IO Submit, Rba : {}, Size : {}, bypassIOWorker : {}"},

            {IBOF_EVENT_ID::UBIO_DEBUG_CHECK_VALID, "Ubio Check before Submit, Rba : {}, Pba.lba : {}, Size : {}, Ublock : {}, ReferenceIncreased :{} sync :{} Direction : {}"},
            {IBOF_EVENT_ID::UBIO_DEBUG_COMPLETE, "Ubio Complete Pba.lba : {} Size : {} Ublock : {} ErrorType : {} Direction : {}"},

            {IBOF_EVENT_ID::UBIO_ALREADY_SYNC_DONE, "Mark done to already completed Ubio"},
            {IBOF_EVENT_ID::UBIO_CALLBACK_EVENT_ALREADY_SET, "Callback Event is already set for Ubio"},
            {IBOF_EVENT_ID::UBIO_DUPLICATE_IO_ABSTRACTION, "Double set request context"},
            {IBOF_EVENT_ID::UBIO_FAIL_TO_ALLOCATE_MEMORY, "Not sufficient memory at Ubio"},
            {IBOF_EVENT_ID::UBIO_FREE_UNALLOWED_BUFFER, "Cannot free unallowed data buffer"},
            {IBOF_EVENT_ID::UBIO_INVALID_GC_PROGRESS, "Invalid GC progress"},
            {IBOF_EVENT_ID::UBIO_INVALID_IBOF_IO, "Invalid ibof IO"},
            {IBOF_EVENT_ID::UBIO_INVALID_IO_STATE, "Invalid IO state"},
            {IBOF_EVENT_ID::UBIO_INVALID_LSID, "Invalid LSID for Ubio"},
            {IBOF_EVENT_ID::UBIO_INVALID_ORIGIN_UBIO, "Invalid original Ubio"},
            {IBOF_EVENT_ID::UBIO_INVALID_ORIGINAL_CORE, "Invalid origin core for Ubio"},
            {IBOF_EVENT_ID::UBIO_INVALID_PBA, "Invalid PBA for Ubio"},
            {IBOF_EVENT_ID::UBIO_INVALID_RBA, "Invalid RBA for Ubio"},
            {IBOF_EVENT_ID::UBIO_INVALID_UBIO_HANDLER, "Invalid ubio handler"},
            {IBOF_EVENT_ID::UBIO_INVALID_VOLUME_ID, "Invalid volume ID for Ubio"},
            {IBOF_EVENT_ID::UBIO_INVALID_VSA, "Invalid VSA for Ubio"},
            {IBOF_EVENT_ID::UBIO_NO_COMPLETION_FOR_FRONT_END_EVENT, "No explicit implementation for front-end complete type"},
            {IBOF_EVENT_ID::UBIO_REMAINING_COUNT_ERROR, "Completion count exceeds remaining count"},
            {IBOF_EVENT_ID::UBIO_REQUEST_NULL_BUFFER, "Requested buffer of Ubio is null"},
            {IBOF_EVENT_ID::UBIO_REQUEST_OUT_RANGE, "Requested buffer of Ubio is out of range"},
            {IBOF_EVENT_ID::UBIO_SYNC_FLAG_NOT_SET, "Mark done is only valid for sync Ubio"},
            {IBOF_EVENT_ID::UBIO_WRONG_SPLIT_SIZE, "Invalid size of Ubio split request"},

            {IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_NOT_EXISTS, "uNVRAM backup file doesn't exist, ignoring backup restoration."},
            {IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_OPEN_FAILED, "uNVRAM backup file does exist but opening is failed"},
            {IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_READ_FAILED, "Cannot read backup file"},
            {IBOF_EVENT_ID::UNVRAM_BACKUP_FILE_STAT_FAILED, "Cannot get uNVRAM backup file stat"},
            {IBOF_EVENT_ID::UNVRAM_COMPLETION_TIMEOUT, "uNVRAM completion checking timed out"},
            {IBOF_EVENT_ID::UNVRAM_FAIL_TO_CLOSE, "Fail to close uNVRAM"},
            {IBOF_EVENT_ID::UNVRAM_FAIL_TO_OPEN, "Fail to open uNVRAM"},
            {IBOF_EVENT_ID::UNVRAM_FAIL_TO_REQUEST_IO, "Fail to request nvram IO to reactor"},
            {IBOF_EVENT_ID::UNVRAM_FAIL_TO_RETRY_IO, "Fail to retry nvram IO to reactor"},
            {IBOF_EVENT_ID::UNVRAM_PENDING_IO_NOT_FOUND, "Pending Ubio not found: DeviceContext: {}, Ubio: {}"},
            {IBOF_EVENT_ID::UNVRAM_RESTORING_FAILED, "Cannot open uNVRAM driver for restoring"},
            {IBOF_EVENT_ID::UNVRAM_RESTORING_PAGE_DONE, "Restoring Hugepage #{} done."},
            {IBOF_EVENT_ID::UNVRAM_SUBMISSION_FAILED, "IO submission failed"},
            {IBOF_EVENT_ID::UNVRAM_SUBMISSION_TIMEOUT, "Could not submit the given IO request in time"},

            {IBOF_EVENT_ID::WRCMP_FAIL_TO_ALLOCATE_MEMORY, "Fail to allocate memory"},
            {IBOF_EVENT_ID::WRCMP_INVALID_SPLIT_UBIO, "Split Ubio is null at WriteCompleting state"},
            {IBOF_EVENT_ID::WRCMP_INVALID_STRIPE, "Stripe is null at WriteCompleting state"},
            {IBOF_EVENT_ID::WRCMP_IO_ERROR, "Write is failed at WriteCompleting state"},
            {IBOF_EVENT_ID::WRCMP_WRITE_WRAPUP_FAILED, "Write wrapup failed at WriteCompleting state"},
            {IBOF_EVENT_ID::WRCMP_MAP_UPDATE_FAILED, "Write wraup failed at map update"},

            {IBOF_EVENT_ID::IOCONTROL_REBUILD_FAIL, "Rebuild Read Failed"},
            {IBOF_EVENT_ID::WRHDLR_ALLOC_WRITE_BUFFER_FAILED, "Not enough room for write buffer, this write is going to be rescheduled"},
            {IBOF_EVENT_ID::WRHDLR_FAIL_TO_ALLOCATE_MEMORY, "Not sufficient memory"},
            {IBOF_EVENT_ID::WRHDLR_FAIL_BY_SYSTEM_STOP, "System Stop incurs write fail"},
            {IBOF_EVENT_ID::WRHDLR_INVALID_UBIO, "Write Ubio is null"},
            {IBOF_EVENT_ID::WRHDLR_NO_FREE_SPACE, "No free space in write buffer"},

            {IBOF_EVENT_ID::WRHDLR_DEBUG_READ_OLD, "Read Old Block in Write Path Rba : {} Size : {} Vsa.id : {} Vsa.offset :{} Lsid.id : {}, Lsid.Loc :{} Remained : {}"},
            {IBOF_EVENT_ID::WRHDLR_DEBUG_READ_OLD_COMPLETE, "Read Old Block in Write Path Rba : {} Size : {} Lsid.id : {} alignOffset : {} alignSize : {}"},

            {IBOF_EVENT_ID::WRWRAPUP_EVENT_ALLOC_FAILED, "Flush Event allocation failed at WriteWrapup state"},
            {IBOF_EVENT_ID::WRWRAPUP_STRIPE_NOT_FOUND, "Stripe #{} not found at WriteWrapup state"},
            {IBOF_EVENT_ID::WRWRAPUP_DEBUG_STRIPE, "Write Completion, Vsa : {}, remain : {} stripe vid : {}"},
            {IBOF_EVENT_ID::BLKMAP_DEBUG_UPDATE_REQUEST, "Block Map Update Request : StartRba : {} Vsa : {} blCnt : {} volumeId :{}, isGC :{} isOldData :{}"},
            {IBOF_EVENT_ID::BLKMAP_DEBUG_UPDATE, "Block Map Update : StartRba : {} logWriteRequestSuccessful :{}"},
};

IbofEventId::IbofEventIdEntry
    IbofEventId::IOPATH_BACKEND_EVENT_ENTRY[(uint32_t)IBOF_EVENT_ID::IOBACKEND_COUNT] =
        {
            {IBOF_EVENT_ID::IOWORKER_OPERATION_NOT_SUPPORTED, "Requested operation is not supported by IOWorker: {}"},
            {IBOF_EVENT_ID::IOWORKER_DEVICE_ADDED, "{} has been added to IOWorker{}"},
            {IBOF_EVENT_ID::IOWORKER_UNDERFLOW_HAPPENED, "Command completed more than submitted: current outstanding: {}, completion count: {}"},
            {IBOF_EVENT_ID::IOSMHDLR_BUFFER_NOT_ENOUGH, "Buffer is not enough to proceed with IO request"},
            {IBOF_EVENT_ID::IOSMHDLR_BUFFER_NOT_ALIGNED, "Buffer size is not properly aligned to proceed with IO request"},
            {IBOF_EVENT_ID::IOSMHDLR_OPERATION_NOT_SUPPORTED, "Requested operation is not supported by IOSubmitHandler"},
            {IBOF_EVENT_ID::IOSMHDLR_DEBUG_ASYNC_READ, "IOSubmitHandler Async Read : physical lba : {}  blkcnt :{} bufferIndex : {} partitionToIO : {} ret :{}"},
            {IBOF_EVENT_ID::IOSMHDLR_COUNT_DIFFERENT, "IOSubmitHandler Async BufferCounts are different :  total of each entries {}  blkcnt :{}"},
            {IBOF_EVENT_ID::IOSMHDLR_DEBUG_ASYNC_WRITE, "IOSubmitHandler Async Write : physical lba : {} blkcnt : {} partitionToIO : {} totalcnt :{}"},
            {IBOF_EVENT_ID::IOSMHDLR_ARRAY_LOCK, "IOSubmitHandler Array Locking type : {} stripeId :{} count :{}"},
            {IBOF_EVENT_ID::IOSMHDLR_ARRAY_UNLOCK, "IOSubmitHandler Array Unlocking type : {} stripeId :{}"},

            {IBOF_EVENT_ID::DEVICE_OPEN_FAILED, "Device open failed: Name: {}, Desc: {}, libaioContextID: {}, events: {}"},
            {IBOF_EVENT_ID::DEVICE_CLOSE_FAILED, "Device close failed: Name: {}, Desc: {}, libaioContextID: {}, events: {}"},
            {IBOF_EVENT_ID::DEVICE_SCAN_FAILED, "Error occurred while scanning newly detected device: {}"},
            {IBOF_EVENT_ID::DEVICE_SCAN_IGNORED, "Device: {} ignored while scanning since the size: {} Bytes is lesser than required: > {} Bytes"},
            {IBOF_EVENT_ID::DEVICE_SUBMISSION_FAILED, "IO submission failed: Device: {}, Op: {}, Start LBA: {}, Size in Bytes: {}"},

            {IBOF_EVENT_ID::DEVICE_SUBMISSION_QUEUE_FULL, "IO submission pending since the queue is full: Device: {}"},
            {IBOF_EVENT_ID::DEVICE_SUBMISSION_TIMEOUT, "Could not submit the given IO request in time to device: {}, Op: {}, Start LBA: {}, Size in Bytes: {}"},
            {IBOF_EVENT_ID::DEVICE_COMPLETION_FAILED, "Error: {} occurred while getting IO completion events from device: {}"},
            {IBOF_EVENT_ID::DEVICE_THREAD_REGISTERED_FAILED, "Error: {} register device context failed: {}"},
            {IBOF_EVENT_ID::DEVICE_THREAD_UNREGISTERED_FAILED, "Error: {} unregister device context failed: {}"},
            {IBOF_EVENT_ID::DEVICE_OPERATION_NOT_SUPPORTED, "Requested operation: {} is not supported by DeviceDriver"},
            {IBOF_EVENT_ID::DEVICE_PENDING_IO_NOT_FOUND, "Pending Ubio not found: DeviceContext: {}, Ubio: {}"},
            {IBOF_EVENT_ID::DEVICE_UNEXPECTED_PENDING_ERROR_COUNT, "Unexpected pending error count: {}, Current pending error count: {}, Added or subtracted error count: {}"},
            {IBOF_EVENT_ID::DEVICE_OVERLAPPED_SET_IOWORKER, "Overlapped setting for ioworker for single device: {} "},
            {IBOF_EVENT_ID::DEVICE_NULLPTR_IOWORKER, "Overlapped setting for ioworker for single device: {} "},

            {IBOF_EVENT_ID::QOSMGR_REGISTER_POLLER_FAILED, "Failed to register Qos poller on reactor #: {}"},

            {IBOF_EVENT_ID::UNVME_SSD_DEBUG_CREATED, "Create Ublock, Pointer : {}"},
            {IBOF_EVENT_ID::UNVME_SSD_SCAN_FAILED, "Failed to Scan uNVMe devices"},
            {IBOF_EVENT_ID::UNVME_SSD_SCANNED, "uNVMe Device has been scanned: {}, {}"},
            {IBOF_EVENT_ID::UNVME_SSD_ATTACH_NOTIFICATION_FAILED, "Failed to notify uNVMe device attachment: Device name: {}"},
            {IBOF_EVENT_ID::UNVME_SSD_DETACH_NOTIFICATION_FAILED, "Failed to notify uNVMe device detachment: Device name: {}"},
            {IBOF_EVENT_ID::UNVME_SSD_OPEN_FAILED, "uNVMe Device open failed: namespace #{}"},
            {IBOF_EVENT_ID::UNVME_SSD_CLOSE_FAILED, "uNVMe Device close failed: namespace #{}"},
            {IBOF_EVENT_ID::UNVME_SSD_UNDERFLOW_HAPPENED, "Command completed more than submitted: current outstanding: {}, completion count: {}"},
            {IBOF_EVENT_ID::UNVME_SUBMISSION_FAILED, "Command submission failed: namespace: {}, errorCode: {}"},
            {IBOF_EVENT_ID::UNVME_SUBMISSION_QUEUE_FULL, "IO submission pending since the queue is full"},
            {IBOF_EVENT_ID::UNVME_SUBMISSION_RETRY_EXCEED, "Could not submit the given IO request within limited count : lba : {} sectorCount : {} deviceName : {} namespace {}"},
            {IBOF_EVENT_ID::UNVME_COMPLETION_TIMEOUT, "uNVMe completion checking timed out: SN: {}"},

            {IBOF_EVENT_ID::UNVME_COMPLETION_FAILED, "Command completion error occurred: namespace: {}, errorCode: {}"},
            {IBOF_EVENT_ID::UNVME_OPERATION_NOT_SUPPORTED, "Requested operation: {} is not supported by uNVMe DeviceDriver"},
            {IBOF_EVENT_ID::UNVME_CONTROLLER_FATAL_STATUS, "Controller Fatal Status, reset required: SN: {}"},
            {IBOF_EVENT_ID::UNVME_CONTROLLER_RESET_FAILED, "Controller Reset Failed: SN: {}"},
            {IBOF_EVENT_ID::UNVME_CONTROLLER_RESET, "Controller Reset : SN : {}"},
            {IBOF_EVENT_ID::UNVME_SUBMITTING_CMD_ABORT, "Requesting command abort: SN: {}, QPair: {}, CID: {}"},
            {IBOF_EVENT_ID::UNVME_ABORT_TIMEOUT, "Abort Also Timeout: SN: {}, QPair: {}, CID: {}"},
            {IBOF_EVENT_ID::UNVME_ABORT_SUBMISSION_FAILED, "Failed to submit command abort: SN: {}, QPair: {}, CID: {}"},
            {IBOF_EVENT_ID::UNVME_ABORT_COMPLETION_FAILED, "Failed to complete command abort: SN: {}"},
            {IBOF_EVENT_ID::UNVME_ABORT_COMPLETION, "successfully complete command abort: SN: {}"},
            {IBOF_EVENT_ID::UNVME_ABORT_DISABLE_AND_RESET, "abort disabled, try reset SN : {}, QPair : {}, CID: {}"},
            {IBOF_EVENT_ID::UNVME_DO_NOTHING_ON_TIMEOUT, "Do nothing on command timeout: SN: {}"},

            {IBOF_EVENT_ID::UNVME_ABORT_TIMEOUT_FAILED, "Requesting command abort: SN: {}, QPair: {}, CID: {}"},
            {IBOF_EVENT_ID::UNVME_ABORT_DISABLE_COUNT_ERR, "Abort Disable count Err : {}"},
            {IBOF_EVENT_ID::UNVME_NOT_SUPPORTED_DEVICE, ""},
            {IBOF_EVENT_ID::UNVME_DEBUG_RETRY_IO, "Retry IO in unvme_drv, startLBA: {}, sectorCount : {}, direction : {}, deviceName : {}"},
            {IBOF_EVENT_ID::UNVME_DEBUG_REQUEST_IO, "Request IO in unvme_drv, startLBA: {}, sectorCount : {}, direction : {} deviceName : {} ret : {}"},
            {IBOF_EVENT_ID::UNVME_DEBUG_COMPLETE_IO, "Complete IO in unvme_drv, startLBA: {}, sectorCount : {}, direction : {}, sc : {}, sct : {} deviceName : {}"},

            {IBOF_EVENT_ID::DEVICE_CONTEXT_NOT_FOUND, "Device context is not found"},
            {IBOF_EVENT_ID::DEVCTX_ALLOC_TIMEOUT_CHECKER_FAILED, "Allocating TimeoutChecker for pending error list failed."},
            {IBOF_EVENT_ID::IOCTX_ERROR_KEY_NOT_SET, "Key for pending ERROR was not set, before!"},
            {IBOF_EVENT_ID::IOCTX_IO_KEY_NOT_SET, "Key for pending IO was not set, before!"},

            {IBOF_EVENT_ID::IOQ_ENQUEUE_NULL_UBIO, "Enqueue null ubio"},
            {IBOF_EVENT_ID::BUFFER_ENTRY_OUT_OF_RANGE, "Block / Chunk Index excceds block count of BufferEntry"},

            {IBOF_EVENT_ID::NFLSH_ERROR_DETECT, "Failed to proceed stripe map update request event: {}"},
            {IBOF_EVENT_ID::NFLSH_EVENT_ALLOCATION_FAILED, "Failed to allocate event: {}"},
            {IBOF_EVENT_ID::NFLSH_EVENT_MAP_UPDATE_FAILED, "Failed to update map: {}"},
            {IBOF_EVENT_ID::NFLSH_STRIPE_NOT_IN_WRITE_BUFFER, "Stripe #{} is not in WriteBuffer."},
            {IBOF_EVENT_ID::NFLSH_STRIPE_DEBUG, "Stripe Map Update Request : stripe.vsid : {} writeBufferArea : {} wbStripeid : {}"},
            {IBOF_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE, "Stripe Map Update Request : stripe.vsid : {} logWriteRequestSuccess : {}"},

            {IBOF_EVENT_ID::FLUSH_WRAPUP_STRIPE_NOT_IN_USER_AREA, "Stripe #{} is not in UserArea."},
            {IBOF_EVENT_ID::STRIPEPUTEVT_STRIPE_NOT_IN_NORMAL_POOL, "Stripe #{} is not in NormalStripePool."},
};

IbofEventId::~IbofEventId(void)
{
}

const char*&
IbofEventId::GetString(IBOF_EVENT_ID eventId)
{
    int targetIndex;
    IbofEventIdEntry* entryToReturn = nullptr;

    if ((uint32_t)IBOF_EVENT_ID::IONVMF_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)IBOF_EVENT_ID::IONVMF_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)IBOF_EVENT_ID::IONVMF_START;
        entryToReturn = &IOPATH_NVMF_EVENT_ENTRY[targetIndex];
    }
    else if ((uint32_t)IBOF_EVENT_ID::IOFRONTEND_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)IBOF_EVENT_ID::IOFRONTEND_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)IBOF_EVENT_ID::IOFRONTEND_START;
        entryToReturn = &IOPATH_FRONTEND_EVENT_ENTRY[targetIndex];
    }
    else if ((uint32_t)IBOF_EVENT_ID::IOBACKEND_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)IBOF_EVENT_ID::IOBACKEND_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)IBOF_EVENT_ID::IOBACKEND_START;
        entryToReturn = &IOPATH_BACKEND_EVENT_ENTRY[targetIndex];
    }
    else
    {
        return RESERVED_EVENT_ENTRY.message;
    }

    if (eventId != entryToReturn->eventId)
    {
        IBOF_TRACE_ERROR((uint32_t)IBOF_EVENT_ID::EVENT_ID_MAPPING_WRONG,
            "Mapping EventID with Message is wrong: Requested eventId: {}, "
            "Mapped eventId: {}",
            eventId, entryToReturn->eventId);
    }

    return entryToReturn->message;
}

void
IbofEventId::Print(IBOF_EVENT_ID id, EventLevel level)
{
    std::string nullStr;
    Print(id, level, nullStr);
}

void
IbofEventId::Print(IBOF_EVENT_ID id, EventLevel level,
    std::string& additionalMessage)
{
    const char*& predefinedMessage = GetString(id);
    std::string fullMessage(predefinedMessage);
    fullMessage += additionalMessage;
    const char* cStringMessage = fullMessage.c_str();

    switch (level)
    {
        default:
        case EventLevel::CRITICAL:
        {
            IBOF_TRACE_CRITICAL((int)id, cStringMessage);
            break;
        }
        case EventLevel::ERROR:
        {
            IBOF_TRACE_ERROR((int)id, cStringMessage);
            break;
        }
        case EventLevel::WARNING:
        {
            IBOF_TRACE_WARN((int)id, cStringMessage);
            break;
        }
        case EventLevel::INFO:
        {
            IBOF_TRACE_INFO((int)id, cStringMessage);
            break;
        }
        case EventLevel::DEBUG:
        {
            IBOF_TRACE_DEBUG((int)id, cStringMessage);
        }
    }
}

} // namespace ibofos
