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

namespace pos
{
enum WbtCommandOpcode
{
    INVALID_COMMAND = -1,
    LIST_WBT_CMDS = 0,
    GET_MAP_LAYOUT,
    READ_VSAMAP,
    READ_VSAMAP_ENTRY,
    WRITE_VSAMAP,
    WRITE_VSAMAP_ENTRY,
    READ_STRIPEMAP,
    READ_STRIPEMAP_ENTRY,
    WRITE_STRIPEMAP,
    WRITE_STRIPEMAP_ENTRY,
    READ_REVERSE_MAP,
    READ_WHOLE_REVERSE_MAP,
    READ_REVERSE_MAP_ENTRY,
    WRITE_REVERSE_MAP,
    WRITE_WHOLE_REVERSE_MAP,
    WRITE_REVERSE_MAP_ENTRY,
    GET_BITMAP_LAYOUT,
    GET_INSTANT_META_INFO,
    GET_WB_LSID_BITMAP,
    SET_WB_LSID_BITMAP,
    GET_ACTIVE_STRIPE_TAIL,
    SET_ACTIVE_STRIPE_TAIL,
    GET_CURRENT_SSD_LSID,
    SET_CURRENT_SSD_LSID,
    GET_SEGMENT_INFO,
    SET_SEGMENT_INFO,
    GET_SEGMENT_VALID_COUNT,

    // MetaFs Commands
    MFS_COMMAND_START,
    MFS_DUMP_FILES_LIST = MFS_COMMAND_START,
    MFS_CREATE_FILE,
    MFS_OPEN_FILE,
    MFS_CLOSE_FILE,
    MFS_CREATE_FILESYSTEM,
    MFS_MOUNT_FILESYSTEM,
    MFS_UNMOUNT_FILESYSTEM,
    MFS_READ_FILE,
    MFS_WRITE_FILE,
    // MFS_READ_FILE_ASYNC,
    // MFS_WRITE_FILE_ASYNC,
    // MFS_ALLOC_IO_TAG,
    // MFS_RELEASE_IO_TAG,

    // MFS_GET_FILE_CHECKSUM,
    MFS_DUMP_INODE_INFO,
    MFS_SET_INODE_INFO,
    MFS_GET_MAX_FILE_SIZE,
    MFS_GET_FILE_SIZE,
    MFS_GET_ALIGNED_FILE_IO_SIZE,
    MFS_SETUP_META_FIO_TEST,
    MFS_COMMAND_END = MFS_SETUP_META_FIO_TEST,

    FLUSH_GCOV_DATA,
    FLUSH_USER_DATA,

    TRANSLATE_DEVICE_LBA,
    DUMP_DISK_LAYOUT,
    PARITY_LOCATION,
    GET_PARTITION_SIZE,

    WRITE_UNCORRECTABLE_LBA,
    WRITE_RAW_DATA,
    READ_RAW_DATA,

    DO_GC,
    SET_GC_THRESHOLD,
    GET_GC_THRESHOLD,
    GET_GC_STATUS,

    // NVMe Cli
    NVME_CLI,
    ADMIN_PASS_THROUGH,

    NUM_WBT_CMDS,

    GET_JOURNAL_STATUS,

    // Config
    UPDATE_CONFIG
};

} // namespace pos
