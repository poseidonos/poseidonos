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

#include "src/include/pos_event_id.hpp"

#include "logger.h"

namespace pos
{
PosEventId::PosEventIdEntry
    PosEventId::RESERVED_EVENT_ENTRY = {POS_EVENT_ID::RESERVED, "Reserved, Not defined, yet!!"};

PosEventId::PosEventIdEntry
    PosEventId::QOS_EVENT_ENTRY[(uint32_t)POS_EVENT_ID::QOS_COUNT] =
        {
            {POS_EVENT_ID::QOS_POLLER_REGISTRATION_FAILED, "Failed to register Qos poller on reactor #: {}"},
            {POS_EVENT_ID::QOS_POLLER_UNREGISTRATION_FAILED, "Failed to un-register Qos poller on reactor #: {}"},
};

PosEventId::PosEventIdEntry
    PosEventId::IOPATH_NVMF_EVENT_ENTRY[(uint32_t)POS_EVENT_ID::IONVMF_COUNT] =
        {
            {POS_EVENT_ID::IONVMF_1H, "RDMA Invalid Private Data Length"}, // 5000
            {POS_EVENT_ID::IONVMF_2H, "RDMA Invalid RECFMT"},
            {POS_EVENT_ID::IONVMF_3H, "RDMA Invalid QID"},
            {POS_EVENT_ID::IONVMF_4H, "RDMA Invalid HSQSIZE"},
            {POS_EVENT_ID::IONVMF_5H, "RDMA Invalid HRQSIZE"},
            {POS_EVENT_ID::IONVMF_6H, "RDMA No Resources"},
            {POS_EVENT_ID::IONVMF_7H, "RDMA Invalid IRD"},
            {POS_EVENT_ID::IONVMF_8H, "RDMA Invalid ORD"},
            {POS_EVENT_ID::IONVMF_NAMESPACE_ATTACH_FAILED, "Fail to attach Namespace "},
            {POS_EVENT_ID::IONVMF_FAIL_TO_FIND_VOLID, "Fail to parse volume id from bdev name"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_FIND_ARRAYNAME, "Fail to parse array name from bdev name"},
            {POS_EVENT_ID::IONVMF_OVERRIDE_UNVMF_IO_HANDLER, "Override unvmf_io_handler"},
            {POS_EVENT_ID::IONVMF_VOLUME_DETACH_COUNT_OVERFLOW, "Volume detached count is bigger than expected volume count"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_CREATE_POS_BDEV, "Fail to create pos bdev({})"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_DELETE_POS_BDEV, "Fail to delete pos bdev"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_DETACH_NAMESPACE, "Fail to detach namespace from subsystem"},
            {POS_EVENT_ID::IONVMF_BDEV_DOES_NOT_EXIST, "Fail to find requested bdev"},
            {POS_EVENT_ID::IONVMF_BDEV_UUID_DOES_NOT_EXIST, "Fail to get requeted bdev uuid"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_CONVERT_UUID_INTO_STRING, "Fail to convert uuid into string"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_FIND_SUBSYSTEM, "Subsystem is not found"},
            {POS_EVENT_ID::IONVMF_BDEV_ALREADY_EXIST, "Requested bdev{} already exist"},
            {POS_EVENT_ID::IONVMF_FAIL_TO_READ_TRANSPORT_CONFIG, "Fail to read transport config. Will use default setting.{} "},
            {POS_EVENT_ID::IONVMF_FAIL_TO_CREATE_TRANSPORT, "Fail to create transport : {}"},
            {POS_EVENT_ID::IONVMF_TRANSPORT_NUM_SHARED_BUFFER_CHANGED, "Transport's num_shared_buffer size has changed from {} to {} due to reactor core number of system"},
};

PosEventId::PosEventIdEntry
    PosEventId::IOPATH_FRONTEND_EVENT_ENTRY[(uint32_t)POS_EVENT_ID::IOFRONTEND_COUNT] =
        {
            {POS_EVENT_ID::AFTMGR_CPU_COUNT_NOT_ENOUGH, "Unssatisfied CPU Count"}, // 5100
            {POS_EVENT_ID::AFTMGR_DISABLED_CORE, "Core {} is disabled"},
            {POS_EVENT_ID::AFTMGR_FAIL_TO_FIND_MASTER_REACTOR, "Fail to find master reactor"},
            {POS_EVENT_ID::AFTMGR_FAIL_TO_ALLOCATE_ALL_CPU, "All core is not assigned for PoseidonOS"},
            {POS_EVENT_ID::AFTMGR_FAIL_TO_OVERLAP_MASK, "Core mask is overlapped"},
            {POS_EVENT_ID::AFTMGR_FAIL_TO_PARSING_ERROR, "Cpu allowed list is wrongly set"},
            {POS_EVENT_ID::AFTMGR_CORE_NOT_SUFFICIENT, "Cpu is not sufficient"},
            {POS_EVENT_ID::AFTMGR_NO_EVENT_WORKER_ALLOCATED, "Cannot find event worker at any NUMA"},
            {POS_EVENT_ID::AFTMGR_NO_USE_CONFIG, "Use core description from default value"},
            {POS_EVENT_ID::AFTMGR_USE_CONFIG, "Use core description from config file"},

            {POS_EVENT_ID::AIO_CONTEXT_NOT_FOUND, "AIO context is not found"},
            {POS_EVENT_ID::AIO_FAIL_TO_ALLOCATE_EVENT, "Event allocation is failed at AIO completion"},
            {POS_EVENT_ID::AIO_FAIL_TO_ALLOCATE_MEMORY, "Not sufficient memory at AIO"},
            {POS_EVENT_ID::AIO_INVALID_AIO_CONTEXT, "AIO context is null at AIO completion"},
            {POS_EVENT_ID::AIO_INVALID_AIO_PRIVATE, "AIOPrivate is null at AIO completion"},
            {POS_EVENT_ID::AIO_INVALID_BDEV_IO, "Bdev IO is null at AIO completion"},
            {POS_EVENT_ID::AIO_INVALID_UBIO, "Ubio is null at AIO completion"},
            {POS_EVENT_ID::AIO_IO_FROM_INVALID_THREAD, "Only reactor can submit IO"},
            {POS_EVENT_ID::AIO_DEBUG_COMPLETION, "Aio completion : Rba {}, Size {}, readwrite {}, errortype {}"},
            {POS_EVENT_ID::AIO_DEBUG_SUBMISSION, "Aio submission : Rba {}, Size {}, readwrite {}"},
            {POS_EVENT_ID::AIO_FLUSH_START, "Flush Start in Aio, volume id : {}"},
            {POS_EVENT_ID::AIO_FLUSH_END, "Flush End in Aio, volume id : {}"},

            {POS_EVENT_ID::BLKALGN_INVALID_UBIO, "Block aligning Ubio is null"},

            {POS_EVENT_ID::BLKHDLR_FAIL_TO_ALLOCATE_EVENT, "Cannot allocate memory for event"},
            {POS_EVENT_ID::BLKHDLR_WRONG_IO_DIRECTION, "Wrong IO direction (only read/write types are suppoered)"},

            {POS_EVENT_ID::CALLBACK_INVALID_CALLEE, "Invalid callee for callback"},
            {POS_EVENT_ID::CALLBACK_INVALID_COUNT, "CompletionCount exceeds WaitingCount"},
            {POS_EVENT_ID::CALLBACK_TIMEOUT, "Callback Timeout. Caller : {}"},
            {POS_EVENT_ID::CALLBACK_DESTROY_WITHOUT_EXECUTED, "Callback destroy without executed : {}"},
            {POS_EVENT_ID::CALLBACK_DESTROY_WITH_ERROR, "Callback Error : {}"},

            {POS_EVENT_ID::EVENTFRAMEWORK_FAIL_TO_ALLOCATE_EVENT, "Fail to allocate spdk event"},
            {POS_EVENT_ID::EVENTFRAMEWORK_INVALID_EVENT, "Invalid Event to send"},
            {POS_EVENT_ID::EVENTFRAMEWORK_INVALID_REACTOR, "Reactor {} is not processable"},

            {POS_EVENT_ID::EVENTSCHEDULER_NOT_MATCH_WORKER_COUNT, "EventScheduler receives wrong worker count and cpu_set_t"},

            {POS_EVENT_ID::EVTARG_WRONG_ARGUMENT_ACCESS, "Request for getting wrong argument index"},
            {POS_EVENT_ID::EVTARG_WRONG_ARGUMENT_ADD, "Request for adding wrong argument index"},

            {POS_EVENT_ID::EVTQ_INVALID_EVENT, "Input event is null at EventQueue"},

            {POS_EVENT_ID::EVTSCHDLR_INVALID_WORKER_ID, "WorkerID is not valid at EventScheduler"},

            {POS_EVENT_ID::FLUSHREAD_DEBUG_SUBMIT, "Flush Read Submission StartLSA.stripeId : {} blocksInStripe : {}"},
            {POS_EVENT_ID::FLUSHREAD_FAIL_TO_ALLOCATE_MEMORY, "Fail to allocate memory"},
            {POS_EVENT_ID::FLUSH_DEBUG_SUBMIT, "Flush Submission vsid : {} StartLSA.stripeId : {} blocksInStripe : {}"},
            {POS_EVENT_ID::FLUSH_DEBUG_COMPLETION, "Flush Completion vsid : {} userstripeid : {} userArea : {}"},

            {POS_EVENT_ID::FREEBUFPOOL_FAIL_TO_ALLOCATE_MEMORY, "Fail to allocate memory"},

            {POS_EVENT_ID::IOAT_CONFIG_INVALID, "Invalid ioat count for numa"},

            {POS_EVENT_ID::IOATAPI_FAIL_TO_FINALIZE, "Fail to finalize IOAT"},
            {POS_EVENT_ID::IOATAPI_FAIL_TO_INITIALIZE, "Fail to initialize IOAT"},
            {POS_EVENT_ID::IOATAPI_FAIL_TO_SUBMIT_COPY, "Fail to request IOAT copy to reactor"},
            {POS_EVENT_ID::IOATAPI_DISABLED, "IOAT disabled"},
            {POS_EVENT_ID::IOATAPI_ENABLED, "IOAT enabled"},

            {POS_EVENT_ID::IODISPATCHER_INVALID_CPU_INDEX, "Invalid CPU index {}"},
            {POS_EVENT_ID::IODISPATCHER_INVALID_PARM, "Invalid Param in submit IO"},

            {POS_EVENT_ID::MERGER_INVALID_SPLIT_REQUESTED, "Requested Ubio index exceeds Ubio count of Merger"},

            {POS_EVENT_ID::RBAMGR_WRONG_VOLUME_ID, "Volume ID is not valid at RBAStateManager"},

            {POS_EVENT_ID::RDCMP_INVALID_ORIGIN_UBIO, "Origin Ubio is null at ReadCompleteteHandler"},
            {POS_EVENT_ID::RDCMP_INVALID_UBIO, "Ubio is null at ReadCompleteHandler"},
            {POS_EVENT_ID::RDCMP_READ_FAIL, "Uncorrectable data error"},

            {POS_EVENT_ID::RDHDLR_INVALID_UBIO, "Read Ubio is null"},
            {POS_EVENT_ID::RDHDLR_READ_DURING_REBUILD, "Data is read from which is under rebuild process"},

            {POS_EVENT_ID::SCHEDAPI_COMPLETION_POLLING_FAIL, "Fail to poll ibof completion"},
            {POS_EVENT_ID::SCHEDAPI_NULL_COMMAND, "Command from bdev is empty"},
            {POS_EVENT_ID::SCHEDAPI_SUBMISSION_FAIL, "Fail to submit ibof IO"},
            {POS_EVENT_ID::SCHEDAPI_WRONG_BUFFER, "Single IO command should have a continuous buffer"},

            {POS_EVENT_ID::STRIPE_INVALID_VOLUME_ID, "VolumeId is invalid"},

            {POS_EVENT_ID::REF_COUNT_RAISE_FAIL, "When Io Submit, refcount raise fail"},
            {POS_EVENT_ID::TRANSLATE_CONVERT_FAIL, "Translate() or Convert() is failed"},
            {POS_EVENT_ID::TRANSLATE_CONVERT_FAIL_IN_SYSTEM_STOP, "Translate() or Convert() is failed in system stop"},

            {POS_EVENT_ID::TRSLTR_INVALID_BLOCK_INDEX, "Block index exceeds block count at Translator"},
            {POS_EVENT_ID::TRSLTR_WRONG_ACCESS, "Only valid for single block Translator"},
            {POS_EVENT_ID::TRSLTR_WRONG_VOLUME_ID, "Volume ID is not valid at Translator"},

            {POS_EVENT_ID::VOLUMEIO_DEBUG_SUBMIT, "Volume IO Submit, Rba : {}, Size : {}, bypassIOWorker : {}"},

            {POS_EVENT_ID::UBIO_DEBUG_CHECK_VALID, "Ubio Check before Submit, Rba : {}, Pba.lba : {}, Size : {}, Ublock : {}, ReferenceIncreased :{} sync :{} Direction : {}"},
            {POS_EVENT_ID::UBIO_DEBUG_COMPLETE, "Ubio Complete Pba.lba : {} Size : {} Ublock : {} ErrorType : {} Direction : {}"},

            {POS_EVENT_ID::UBIO_ALREADY_SYNC_DONE, "Mark done to already completed Ubio"},
            {POS_EVENT_ID::UBIO_CALLBACK_EVENT_ALREADY_SET, "Callback Event is already set for Ubio"},
            {POS_EVENT_ID::UBIO_DUPLICATE_IO_ABSTRACTION, "Double set request context"},
            {POS_EVENT_ID::UBIO_FAIL_TO_ALLOCATE_MEMORY, "Not sufficient memory at Ubio"},
            {POS_EVENT_ID::UBIO_FREE_UNALLOWED_BUFFER, "Cannot free unallowed data buffer"},
            {POS_EVENT_ID::UBIO_INVALID_GC_PROGRESS, "Invalid GC progress"},
            {POS_EVENT_ID::UBIO_INVALID_POS_IO, "Invalid ibof IO"},
            {POS_EVENT_ID::UBIO_INVALID_IO_STATE, "Invalid IO state"},
            {POS_EVENT_ID::UBIO_INVALID_LSID, "Invalid LSID for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_ORIGIN_UBIO, "Invalid original Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_ORIGINAL_CORE, "Invalid origin core for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_PBA, "Invalid PBA for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_RBA, "Invalid RBA for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_UBIO_HANDLER, "Invalid ubio handler"},
            {POS_EVENT_ID::UBIO_INVALID_VOLUME_ID, "Invalid volume ID for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_VSA, "Invalid VSA for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_ARRAY_NAME, "Invalid Array Name for Ubio"},
            {POS_EVENT_ID::UBIO_INVALID_DEVICE, "Invalid UblockDevice for Ubio"},
            {POS_EVENT_ID::UBIO_NO_COMPLETION_FOR_FRONT_END_EVENT, "No explicit implementation for front-end complete type"},
            {POS_EVENT_ID::UBIO_REMAINING_COUNT_ERROR, "Completion count exceeds remaining count"},
            {POS_EVENT_ID::UBIO_REQUEST_NULL_BUFFER, "Requested buffer of Ubio is null"},
            {POS_EVENT_ID::UBIO_REQUEST_OUT_RANGE, "Requested buffer of Ubio is out of range"},
            {POS_EVENT_ID::UBIO_SYNC_FLAG_NOT_SET, "Mark done is only valid for sync Ubio"},
            {POS_EVENT_ID::UBIO_WRONG_SPLIT_SIZE, "Invalid size of Ubio split request"},

            {POS_EVENT_ID::URAM_BACKUP_FILE_NOT_EXISTS, "URAM backup file doesn't exist, ignoring backup restoration."},
            {POS_EVENT_ID::URAM_BACKUP_FILE_OPEN_FAILED, "URAM backup file does exist but opening is failed"},
            {POS_EVENT_ID::URAM_BACKUP_FILE_READ_FAILED, "Cannot read backup file"},
            {POS_EVENT_ID::URAM_BACKUP_FILE_STAT_FAILED, "Cannot get URAM backup file stat"},
            {POS_EVENT_ID::URAM_CONFIG_FILE_OPEN_FAILED, "Cannot open uram config file"},
            {POS_EVENT_ID::URAM_COMPLETION_TIMEOUT, "URAM completion checking timed out"},
            {POS_EVENT_ID::URAM_FAIL_TO_CLOSE, "Fail to close URAM"},
            {POS_EVENT_ID::URAM_FAIL_TO_OPEN, "Fail to open URAM"},
            {POS_EVENT_ID::URAM_FAIL_TO_REQUEST_IO, "Fail to request nvram IO to reactor"},
            {POS_EVENT_ID::URAM_FAIL_TO_RETRY_IO, "Fail to retry nvram IO to reactor"},
            {POS_EVENT_ID::URAM_PENDING_IO_NOT_FOUND, "Pending Ubio not found: DeviceContext: {}, Ubio: {}"},
            {POS_EVENT_ID::URAM_RESTORING_FAILED, "Cannot open URAM driver for restoring"},
            {POS_EVENT_ID::URAM_RESTORING_PAGE_DONE, "Restoring Hugepage #{} done."},
            {POS_EVENT_ID::URAM_SUBMISSION_FAILED, "IO submission failed"},
            {POS_EVENT_ID::URAM_SUBMISSION_TIMEOUT, "Could not submit the given IO request in time"},

            {POS_EVENT_ID::WRCMP_FAIL_TO_ALLOCATE_MEMORY, "Fail to allocate memory"},
            {POS_EVENT_ID::WRCMP_INVALID_SPLIT_UBIO, "Split Ubio is null at WriteCompleting state"},
            {POS_EVENT_ID::WRCMP_INVALID_STRIPE, "Stripe is null at WriteCompleting state"},
            {POS_EVENT_ID::WRCMP_IO_ERROR, "Write is failed at WriteCompleting state"},
            {POS_EVENT_ID::WRCMP_WRITE_WRAPUP_FAILED, "Write wrapup failed at WriteCompleting state"},
            {POS_EVENT_ID::WRCMP_MAP_UPDATE_FAILED, "Write wraup failed at map update"},

            {POS_EVENT_ID::IOCONTROL_REBUILD_FAIL, "Rebuild Read Failed"},
            {POS_EVENT_ID::WRHDLR_ALLOC_WRITE_BUFFER_FAILED, "Not enough room for write buffer, this write is going to be rescheduled"},
            {POS_EVENT_ID::WRHDLR_FAIL_TO_ALLOCATE_MEMORY, "Not sufficient memory"},
            {POS_EVENT_ID::WRHDLR_FAIL_BY_SYSTEM_STOP, "System Stop incurs write fail"},
            {POS_EVENT_ID::WRHDLR_INVALID_UBIO, "Write Ubio is null"},
            {POS_EVENT_ID::WRHDLR_NO_FREE_SPACE, "No free space in write buffer"},

            {POS_EVENT_ID::WRHDLR_DEBUG_READ_OLD, "Read Old Block in Write Path Rba : {} Size : {} Vsa.id : {} Vsa.offset :{} Lsid.id : {}, Lsid.Loc :{} Remained : {}"},
            {POS_EVENT_ID::WRHDLR_DEBUG_READ_OLD_COMPLETE, "Read Old Block in Write Path Rba : {} Size : {} Lsid.id : {} alignOffset : {} alignSize : {}"},

            {POS_EVENT_ID::WRWRAPUP_EVENT_ALLOC_FAILED, "Flush Event allocation failed at WriteWrapup state"},
            {POS_EVENT_ID::WRWRAPUP_STRIPE_NOT_FOUND, "Stripe #{} not found at WriteWrapup state"},
            {POS_EVENT_ID::WRWRAPUP_DEBUG_STRIPE, "Write Completion, Vsa : {}, remain : {} stripe vid : {}"},
            {POS_EVENT_ID::BLKMAP_DEBUG_UPDATE_REQUEST, "Block Map Update Request : StartRba : {} Vsa : {} blCnt : {} volumeId :{}, isGC :{} isOldData :{}"},
            {POS_EVENT_ID::BLKMAP_DEBUG_UPDATE, "Block Map Update : StartRba : {} logWriteRequestSuccessful :{}"},
};

PosEventId::PosEventIdEntry
    PosEventId::IOPATH_BACKEND_EVENT_ENTRY[(uint32_t)POS_EVENT_ID::IOBACKEND_COUNT] =
        {
            {POS_EVENT_ID::IOWORKER_OPERATION_NOT_SUPPORTED, "Requested operation is not supported by IOWorker: {}"},
            {POS_EVENT_ID::IOWORKER_DEVICE_ADDED, "{} has been added to IOWorker{}"},
            {POS_EVENT_ID::IOWORKER_UNDERFLOW_HAPPENED, "Command completed more than submitted: current outstanding: {}, completion count: {}"},
            {POS_EVENT_ID::IOSMHDLR_BUFFER_NOT_ENOUGH, "Buffer is not enough to proceed with IO request"},
            {POS_EVENT_ID::IOSMHDLR_BUFFER_NOT_ALIGNED, "Buffer size is not properly aligned to proceed with IO request"},
            {POS_EVENT_ID::IOSMHDLR_OPERATION_NOT_SUPPORTED, "Requested operation is not supported by IOSubmitHandler"},
            {POS_EVENT_ID::IOSMHDLR_DEBUG_ASYNC_READ, "IOSubmitHandler Async Read : physical lba : {}  blkcnt :{} bufferIndex : {} partitionToIO : {} ret :{}"},
            {POS_EVENT_ID::IOSMHDLR_COUNT_DIFFERENT, "IOSubmitHandler Async BufferCounts are different :  total of each entries {}  blkcnt :{}"},
            {POS_EVENT_ID::IOSMHDLR_DEBUG_ASYNC_WRITE, "IOSubmitHandler Async Write : physical lba : {} blkcnt : {} partitionToIO : {} totalcnt :{}"},
            {POS_EVENT_ID::IOSMHDLR_ARRAY_LOCK, "IOSubmitHandler Array Locking type : {} stripeId :{} count :{}"},
            {POS_EVENT_ID::IOSMHDLR_ARRAY_UNLOCK, "IOSubmitHandler Array Unlocking type : {} stripeId :{}"},
            {POS_EVENT_ID::IOSMHDLR_BYTEIO_BUFFER_NULLPTR, "Buffer pointer of Byte IO is null"},
            {POS_EVENT_ID::IOSMHDLR_BYTEIO_PARTITION_IS_NOT_BYTE_ACCESSIBLE, "Parition type is not allowed for memory access"},
            {POS_EVENT_ID::IOSMHDLR_BYTEIO_DIR_NOT_SUPORTTED, "Given Direction {} is not supported"},
            {POS_EVENT_ID::IOSMHDLR_BYTEIO_PARTITION_TRANSLATE_ERROR, "Translate or Convert Error in ByteIO"},
            {POS_EVENT_ID::I_IOSMHDLR_NULLPTR, "IIOSubmitHandler instance nullptr!"},

            {POS_EVENT_ID::DEVICE_OPEN_FAILED, "Device open failed: Name: {}, Desc: {}, libaioContextID: {}, events: {}"},
            {POS_EVENT_ID::DEVICE_CLOSE_FAILED, "Device close failed: Name: {}, Desc: {}, libaioContextID: {}, events: {}"},
            {POS_EVENT_ID::DEVICE_SCAN_FAILED, "Error occurred while scanning newly detected device: {}"},
            {POS_EVENT_ID::DEVICE_SCAN_IGNORED, "Device: {} ignored while scanning since the size: {} Bytes is lesser than required: > {} Bytes"},
            {POS_EVENT_ID::DEVICE_SUBMISSION_FAILED, "IO submission failed: Device: {}, Op: {}, Start LBA: {}, Size in Bytes: {}"},

            {POS_EVENT_ID::DEVICE_SUBMISSION_QUEUE_FULL, "IO submission pending since the queue is full: Device: {}"},
            {POS_EVENT_ID::DEVICE_SUBMISSION_TIMEOUT, "Could not submit the given IO request in time to device: {}, Op: {}, Start LBA: {}, Size in Bytes: {}"},
            {POS_EVENT_ID::DEVICE_COMPLETION_FAILED, "Error: {} occurred while getting IO completion events from device: {}"},
            {POS_EVENT_ID::DEVICE_THREAD_REGISTERED_FAILED, "Error: {} register device context failed: {}"},
            {POS_EVENT_ID::DEVICE_THREAD_UNREGISTERED_FAILED, "Error: {} unregister device context failed: {}"},
            {POS_EVENT_ID::DEVICE_OPERATION_NOT_SUPPORTED, "Requested operation: {} is not supported by DeviceDriver"},
            {POS_EVENT_ID::DEVICE_PENDING_IO_NOT_FOUND, "Pending Ubio not found: DeviceContext: {}, Ubio: {}"},
            {POS_EVENT_ID::DEVICE_UNEXPECTED_PENDING_ERROR_COUNT, "Unexpected pending error count: {}, Current pending error count: {}, Added or subtracted error count: {}"},
            {POS_EVENT_ID::DEVICE_OVERLAPPED_SET_IOWORKER, "Overlapped setting for ioworker for single device: {} "},
            {POS_EVENT_ID::DEVICE_NULLPTR_IOWORKER, "Overlapped setting for ioworker for single device: {} "},

            {POS_EVENT_ID::UNVME_SSD_DEBUG_CREATED, "Create Ublock, Pointer : {}"},
            {POS_EVENT_ID::UNVME_SSD_SCAN_FAILED, "Failed to Scan uNVMe devices"},
            {POS_EVENT_ID::UNVME_SSD_SCANNED, "uNVMe Device has been scanned: {}, {}"},
            {POS_EVENT_ID::UNVME_SSD_ATTACH_NOTIFICATION_FAILED, "Failed to notify uNVMe device attachment: Device name: {}"},
            {POS_EVENT_ID::UNVME_SSD_DETACH_NOTIFICATION_FAILED, "Failed to notify uNVMe device detachment: Device name: {}"},
            {POS_EVENT_ID::UNVME_SSD_OPEN_FAILED, "uNVMe Device open failed: namespace #{}"},
            {POS_EVENT_ID::UNVME_SSD_CLOSE_FAILED, "uNVMe Device close failed: namespace #{}"},
            {POS_EVENT_ID::UNVME_SSD_UNDERFLOW_HAPPENED, "Command completed more than submitted: current outstanding: {}, completion count: {}"},
            {POS_EVENT_ID::UNVME_SUBMISSION_FAILED, "Command submission failed: namespace: {}, errorCode: {}"},
            {POS_EVENT_ID::UNVME_SUBMISSION_QUEUE_FULL, "IO submission pending since the queue is full"},
            {POS_EVENT_ID::UNVME_SUBMISSION_RETRY_EXCEED, "Could not submit the given IO request within limited count : lba : {} sectorCount : {} deviceName : {} namespace {}"},
            {POS_EVENT_ID::UNVME_COMPLETION_TIMEOUT, "uNVMe completion checking timed out: SN: {}"},

            {POS_EVENT_ID::UNVME_COMPLETION_FAILED, "Command completion error occurred: namespace: {}, errorCode: {}"},
            {POS_EVENT_ID::UNVME_OPERATION_NOT_SUPPORTED, "Requested operation: {} is not supported by uNVMe DeviceDriver"},
            {POS_EVENT_ID::UNVME_CONTROLLER_FATAL_STATUS, "Controller Fatal Status, reset required: SN: {}"},
            {POS_EVENT_ID::UNVME_CONTROLLER_RESET_FAILED, "Controller Reset Failed: SN: {}"},
            {POS_EVENT_ID::UNVME_CONTROLLER_RESET, "Controller Reset : SN : {}"},
            {POS_EVENT_ID::UNVME_SUBMITTING_CMD_ABORT, "Requesting command abort: SN: {}, QPair: {}, CID: {}"},
            {POS_EVENT_ID::UNVME_ABORT_TIMEOUT, "Abort Also Timeout: SN: {}, QPair: {}, CID: {}"},
            {POS_EVENT_ID::UNVME_ABORT_SUBMISSION_FAILED, "Failed to submit command abort: SN: {}, QPair: {}, CID: {}"},
            {POS_EVENT_ID::UNVME_ABORT_COMPLETION_FAILED, "Failed to complete command abort: SN: {}"},
            {POS_EVENT_ID::UNVME_ABORT_COMPLETION, "successfully complete command abort: SN: {}"},
            {POS_EVENT_ID::UNVME_DO_NOTHING_ON_TIMEOUT, "Do nothing on command timeout: SN: {}"},

            {POS_EVENT_ID::UNVME_ABORT_TIMEOUT_FAILED, "Requesting command abort: SN: {}, QPair: {}, CID: {}"},
            {POS_EVENT_ID::UNVME_NOT_SUPPORTED_DEVICE, ""},
            {POS_EVENT_ID::UNVME_DEBUG_RETRY_IO, "Retry IO in unvme_drv, startLBA: {}, sectorCount : {}, direction : {}, deviceName : {}"},
            {POS_EVENT_ID::UNVME_DEBUG_REQUEST_IO, "Request IO in unvme_drv, startLBA: {}, sectorCount : {}, direction : {} deviceName : {} ret : {}"},
            {POS_EVENT_ID::UNVME_DEBUG_COMPLETE_IO, "Complete IO in unvme_drv, startLBA: {}, sectorCount : {}, direction : {}, sc : {}, sct : {} deviceName : {}"},

            {POS_EVENT_ID::DEVICE_CONTEXT_NOT_FOUND, "Device context is not found"},
            {POS_EVENT_ID::DEVCTX_ALLOC_TIMEOUT_CHECKER_FAILED, "Allocating TimeoutChecker for pending error list failed."},
            {POS_EVENT_ID::IOCTX_ERROR_KEY_NOT_SET, "Key for pending ERROR was not set, before!"},
            {POS_EVENT_ID::IOCTX_IO_KEY_NOT_SET, "Key for pending IO was not set, before!"},

            {POS_EVENT_ID::IOQ_ENQUEUE_NULL_UBIO, "Enqueue null ubio"},
            {POS_EVENT_ID::BUFFER_ENTRY_OUT_OF_RANGE, "Block / Chunk Index excceds block count of BufferEntry"},

            {POS_EVENT_ID::NFLSH_ERROR_DETECT, "Failed to proceed stripe map update request event: {}"},
            {POS_EVENT_ID::NFLSH_EVENT_ALLOCATION_FAILED, "Failed to allocate event: {}"},
            {POS_EVENT_ID::NFLSH_EVENT_MAP_UPDATE_FAILED, "Failed to update map: {}"},
            {POS_EVENT_ID::NFLSH_STRIPE_NOT_IN_WRITE_BUFFER, "Stripe #{} is not in WriteBuffer."},
            {POS_EVENT_ID::NFLSH_STRIPE_DEBUG, "Stripe Map Update Request : stripe.vsid : {} writeBufferArea : {} wbStripeid : {}"},
            {POS_EVENT_ID::NFLSH_STRIPE_DEBUG_UPDATE, "Stripe Map Update Request : stripe.vsid : {} logWriteRequestSuccess : {}"},

            {POS_EVENT_ID::FLUSH_WRAPUP_STRIPE_NOT_IN_USER_AREA, "Stripe #{} is not in UserArea."},
            {POS_EVENT_ID::STRIPEPUTEVT_STRIPE_NOT_IN_NORMAL_POOL, "Stripe #{} is not in NormalStripePool."},
};

PosEventId::~PosEventId(void)
{
}

const char*&
PosEventId::GetString(POS_EVENT_ID eventId)
{
    int targetIndex;
    PosEventIdEntry* entryToReturn = nullptr;


    if ((uint32_t)POS_EVENT_ID::QOS_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)POS_EVENT_ID::QOS_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)POS_EVENT_ID::QOS_START;
        entryToReturn = &QOS_EVENT_ENTRY[targetIndex];
    }
    else if ((uint32_t)POS_EVENT_ID::IONVMF_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)POS_EVENT_ID::IONVMF_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)POS_EVENT_ID::IONVMF_START;
        entryToReturn = &IOPATH_NVMF_EVENT_ENTRY[targetIndex];
    }
    else if ((uint32_t)POS_EVENT_ID::IOFRONTEND_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)POS_EVENT_ID::IOFRONTEND_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)POS_EVENT_ID::IOFRONTEND_START;
        entryToReturn = &IOPATH_FRONTEND_EVENT_ENTRY[targetIndex];
    }
    else if ((uint32_t)POS_EVENT_ID::IOBACKEND_START <= (uint32_t)eventId && (uint32_t)eventId < (uint32_t)POS_EVENT_ID::IOBACKEND_END)
    {
        targetIndex = (uint32_t)eventId - (uint32_t)POS_EVENT_ID::IOBACKEND_START;
        entryToReturn = &IOPATH_BACKEND_EVENT_ENTRY[targetIndex];
    }
    else
    {
        return RESERVED_EVENT_ENTRY.message;
    }

    if (eventId != entryToReturn->eventId)
    {
        POS_TRACE_ERROR((uint32_t)POS_EVENT_ID::EVENT_ID_MAPPING_WRONG,
            "Mapping EventID with Message is wrong: Requested eventId: {}, "
            "Mapped eventId: {}",
            eventId, entryToReturn->eventId);
    }

    return entryToReturn->message;
}

void
PosEventId::Print(POS_EVENT_ID id, EventLevel level)
{
    std::string nullStr;
    Print(id, level, nullStr);
}

void
PosEventId::Print(POS_EVENT_ID id, EventLevel level,
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
            POS_TRACE_CRITICAL((int)id, cStringMessage);
            break;
        }
        case EventLevel::ERROR:
        {
            POS_TRACE_ERROR((int)id, cStringMessage);
            break;
        }
        case EventLevel::WARNING:
        {
            POS_TRACE_WARN((int)id, cStringMessage);
            break;
        }
        case EventLevel::INFO:
        {
            POS_TRACE_INFO((int)id, cStringMessage);
            break;
        }
        case EventLevel::DEBUG:
        {
            POS_TRACE_DEBUG((int)id, cStringMessage);
        }
    }
}

} // namespace pos
