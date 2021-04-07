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
#include "mk/ibof_config.h"

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"
#include "wbt_metafs_cmd_handler.h"

namespace ibofos
{
using Args = nlohmann::json;

enum WbtCmdOp
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
    GET_USER_SEGMENT_BITMAP,
    SET_USER_SEGMENT_BITMAP,
    GET_SEGMENT_INFO,
    SET_SEGMENT_INFO,
    GET_SEGMENT_INV_COUNT,

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

    WRITE_UNCORRECTABLE_LBA,
    WRITE_RAW_DATA,
    READ_RAW_DATA,

    DO_GC,
    SET_GC_THRESHOLD,
    GET_GC_THRESHOLD,
    GET_GC_STATUS,

    NUM_WBT_CMDS
};

struct WbtCmd
{
    WbtCmdOp opcode;
    std::string testName;
};

static const WbtCmd cmdMap[NUM_WBT_CMDS] = {
    {LIST_WBT_CMDS, "list_wbt"},
    {GET_MAP_LAYOUT, "get_map_layout"},
    {READ_VSAMAP, "read_vsamap"},
    {READ_VSAMAP_ENTRY, "read_vsamap_entry"},
    {WRITE_VSAMAP, "write_vsamap"},
    {WRITE_VSAMAP_ENTRY, "write_vsamap_entry"},
    {READ_STRIPEMAP, "read_stripemap"},
    {READ_STRIPEMAP_ENTRY, "read_stripemap_entry"},
    {WRITE_STRIPEMAP, "write_stripemap"},
    {WRITE_STRIPEMAP_ENTRY, "write_stripemap_entry"},
    {READ_REVERSE_MAP, "read_reverse_map"},
    {READ_WHOLE_REVERSE_MAP, "read_whole_reverse_map"},
    {READ_REVERSE_MAP_ENTRY, "read_reverse_map_entry"},
    {WRITE_REVERSE_MAP, "write_reverse_map"},
    {WRITE_WHOLE_REVERSE_MAP, "write_whole_reverse_map"},
    {WRITE_REVERSE_MAP_ENTRY, "write_reverse_map_entry"},
    {GET_BITMAP_LAYOUT, "get_bitmap_layout"},
    {GET_INSTANT_META_INFO, "get_instant_meta_info"},
    {GET_WB_LSID_BITMAP, "get_wb_lsid_bitmap"},
    {SET_WB_LSID_BITMAP, "set_wb_lsid_bitmap"},
    {GET_ACTIVE_STRIPE_TAIL, "get_active_stripe_tail"},
    {SET_ACTIVE_STRIPE_TAIL, "set_active_stripe_tail"},
    {GET_CURRENT_SSD_LSID, "get_current_ssd_lsid"},
    {SET_CURRENT_SSD_LSID, "set_current_ssd_lsid"},
    {GET_USER_SEGMENT_BITMAP, "get_user_segment_bitmap"},
    {SET_USER_SEGMENT_BITMAP, "set_user_segment_bitmap"},
    {GET_SEGMENT_INFO, "get_segment_info"},
    {SET_SEGMENT_INFO, "set_segment_info"},
    {GET_SEGMENT_INV_COUNT, "get_segment_inv_count"},

    // MFS WBT commnads
    {MFS_DUMP_FILES_LIST, "mfs_dump_files_list"},
    {MFS_CREATE_FILE, "mfs_create_file"},
    {MFS_OPEN_FILE, "mfs_open_file"},
    {MFS_CLOSE_FILE, "mfs_close_file"},
    {MFS_CREATE_FILESYSTEM, "mfs_create_filesystem"},
    {MFS_MOUNT_FILESYSTEM, "mfs_mount_filesystem"},
    {MFS_UNMOUNT_FILESYSTEM, "mfs_unmount_filesystem"},
    {MFS_READ_FILE, "mfs_read_file"},
    {MFS_WRITE_FILE, "mfs_write_file"},
    {MFS_DUMP_INODE_INFO, "mfs_dump_inode_info"},
    {MFS_SET_INODE_INFO, "mfs_set_inode_info"},
    {MFS_GET_MAX_FILE_SIZE, "mfs_get_max_file_size"},
    {MFS_GET_FILE_SIZE, "mfs_get_file_size"},
    {MFS_GET_ALIGNED_FILE_IO_SIZE, "mfs_get_aligned_file_io_size"},
    {MFS_SETUP_META_FIO_TEST, "mfs_setup_meta_fio_test"},

    {FLUSH_GCOV_DATA, "flush_gcov"},
    {FLUSH_USER_DATA, "flush"}, // #4. sync API

    {TRANSLATE_DEVICE_LBA, "translate_device_lba"},
    {DUMP_DISK_LAYOUT, "dump_disk_layout"},
    {PARITY_LOCATION, "parity_location"},

    {WRITE_UNCORRECTABLE_LBA, "write_uncorrectable_lba"},
    {WRITE_RAW_DATA, "write_raw"},
    {READ_RAW_DATA, "read_raw"},

    {DO_GC, "do_gc"},
    {SET_GC_THRESHOLD, "set_gc_threshold"},
    {GET_GC_THRESHOLD, "get_gc_threshold"},
    {GET_GC_STATUS, "get_gc_status"}};

class WbtCmdHandler
{
public:
    WbtCmdHandler(std::string const& testName);
    int64_t operator()(const Args argv, JsonElement& elem);
    JsonElement GetData();

    bool
    IsValid(void)
    {
        return (cmdInfo.opcode != INVALID_COMMAND);
    }

    int GetTestList(std::list<std::string>& testlist);

private:
    int _GetMapLayout(void);
    int _GetBitmapLayout(void);
    int _GetInstantMetaInfo(void);
    int _ReadVsamap(Args argv);
    int _ReadVsamapEntry(Args argv);
    int _WriteVsamap(Args argv);
    int _WriteVsamapEntry(Args argv);
    int _ReadStripemap(Args argv);
    int _ReadStripemapEntry(Args argv);
    int _WriteStripemap(Args argv);
    int _WriteStripemapEntry(Args argv);
    int _ReadReverseMap(Args argv);
    int _ReadWholeReverseMap(Args argv);
    int _WriteReverseMap(Args argv);
    int _WriteWholeReverseMap(Args argv);
    int _ReadReverseMapEntry(Args argv);
    int _WriteReverseMapEntry(Args argv);
    int _GetWbLsidBitmap(Args argv);
    int _SetWbLsidBitmap(Args argv);
    int _GetActiveStripeTail(Args argv);
    int _SetActiveStripeTail(Args argv);
    int _GetCurrentSsdLsid(Args argv);
    int _SetCurrentSsdLsid(Args argv);
    int _GetUserSegmentBitmap(Args argv);
    int _SetUserSegmentBitmap(Args argv);
    int _GetSegmentInfo(Args argv);
    int _SetSegmentInfo(Args argv);
    int _GetSegmentInvalidCount(Args argv);

    int _FlushGcovData(void);
    int _FlushAllUserData(void);

    int _TranslateDeviceLba(Args argv);
    int _DumpDiskLayout(void);
    int _ParityLocation(Args& argv);
    int _DetachDevice(Args argv);

    int _PassThroughToDevice(Args argv);
    int _WriteUncorrectable(Args argv);

    int _WriteRaw(Args argv);
    int _ReadRaw(Args argv);
    int _WritePattern(void* buf, uint32_t pattern, uint32_t bytesToWrite);
    bool _VerifyWriteReadRawCommonParameters(Args& argv);
    bool _VerifyWriteRawSpecificParameters(Args& argv);
    bool _VerifyReadRawSpecificParameters(Args& argv);
    int _FilloutPayloadToWriteRaw(Args& argv, void* buffer, uint32_t bytesToWrite);
    int _DumpPayloadFromReadRaw(Args& argv, void* buffer, uint32_t bytesToRead);

    int _DoGc(void);
    int _SetGcThreshold(Args argv);
    int _GetGcThreshold(JsonElement& elem);
    int _GetGcStatus(JsonElement& elem);

    std::string _GetParameter(Args& argv, const char* paramToSearch);
    uint32_t _ConvertValueToUINT32(std::string& value);

    WbtCmd cmdInfo;
    std::string coutfile = "output.txt";
    WbtMetafsCmdHandler metaCmdHandler;
};

} // namespace ibofos
