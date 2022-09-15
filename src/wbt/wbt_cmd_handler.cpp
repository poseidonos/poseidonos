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

#include "wbt_cmd_handler.h"

#include <fstream>

#include "admin_pass_through_wbt_command.h"
#include "close_file_wbt_command.h"
#include "create_file_wbt_command.h"
#include "do_gc_wbt_command.h"
#include "dump_disk_layout_wbt_command.h"
#include "dump_files_list_wbt_command.h"
#include "dump_inode_info_wbt_command.h"
#include "flush_all_user_data_wbt_command.h"
#include "flush_gcov_data_wbt_command.h"
#include "get_active_stripe_tail_wbt_command.h"
#include "get_aligned_file_io_size_wbt_command.h"
#include "get_bitmap_layout_wbt_command.h"
#include "get_current_ssd_lsid_wbt_command.h"
#include "get_file_size_wbt_command.h"
#include "get_gc_status_wbt_command.h"
#include "get_gc_threshold_wbt_command.h"
#include "get_instant_meta_info_wbt_command.h"
#include "get_journal_status_wbt_command.h"
#include "get_map_layout_wbt_command.h"
#include "get_partition_size_wbt_command.h"
#include "get_segment_info_wbt_command.h"
#include "get_segment_valid_block_count_wbt_command.h"
#include "get_wb_lsid_bitmap_wbt_command.h"
#include "nvme_cli_command.h"
#include "open_file_wbt_command.h"
#include "parity_location_wbt_command.h"
#include "read_file_wbt_command.h"
#include "read_raw_data_wbt_command.h"
#include "read_reverse_map_entry_wbt_command.h"
#include "read_reverse_map_wbt_command.h"
#include "read_stripe_map_entry_wbt_command.h"
#include "read_stripe_map_wbt_command.h"
#include "read_vsa_map_entry_wbt_command.h"
#include "read_vsa_map_wbt_command.h"
#include "read_whole_reverse_map_wbt_command.h"
#include "set_active_stripe_tail_wbt_command.h"
#include "set_current_ssd_lsid_wbt_command.h"
#include "set_gc_threshold_wbt_command.h"
#include "set_segment_info_wbt_command.h"
#include "set_wb_lsid_bitmap_wbt_command.h"
#include "setup_meta_fio_test_wbt_command.h"
#include "translate_device_lba_wbt_command.h"
#include "write_file_wbt_command.h"
#include "write_raw_data_wbt_command.h"
#include "write_reverse_map_entry_wbt_command.h"
#include "write_reverse_map_wbt_command.h"
#include "write_stripe_map_entry_wbt_command.h"
#include "write_stripe_map_wbt_command.h"
#include "write_uncorrectable_lba_wbt_command.h"
#include "write_vsa_map_entry_wbt_command.h"
#include "write_vsa_map_wbt_command.h"
#include "write_whole_reverse_map_wbt_command.h"
#include "update_config_wbt_command.h"
namespace pos
{
WbtCommandMap WbtCmdHandler::wbtCommandMap;
bool WbtCmdHandler::wbtCommandsPrepared = false;

void
WbtCmdHandler::PrepareWbtCommands(void)
{
    // Mapper
    wbtCommandMap["get_map_layout"] = std::make_unique<GetMapLayoutWbtCommand>();

    wbtCommandMap["read_vsamap"] = std::make_unique<ReadVsaMapWbtCommand>();
    wbtCommandMap["read_vsamap_entry"] = std::make_unique<ReadVsaMapEntryWbtCommand>();
    wbtCommandMap["write_vsamap"] = std::make_unique<WriteVsaMapWbtCommand>();
    wbtCommandMap["write_vsamap_entry"] = std::make_unique<WriteVsaMapEntryWbtCommand>();

    wbtCommandMap["read_stripemap"] = std::make_unique<ReadStripeMapWbtCommand>();
    wbtCommandMap["read_stripemap_entry"] = std::make_unique<ReadStripeMapEntryWbtCommand>();
    wbtCommandMap["write_stripemap"] = std::make_unique<WriteStripeMapWbtCommand>();
    wbtCommandMap["write_stripemap_entry"] = std::make_unique<WriteStripeMapEntryWbtCommand>();

    wbtCommandMap["read_reverse_map"] = std::make_unique<ReadReverseMapWbtCommand>();
    wbtCommandMap["read_whole_reverse_map"] = std::make_unique<ReadWholeReverseMapWbtCommand>();
    wbtCommandMap["read_reverse_map_entry"] = std::make_unique<ReadReverseMapEntryWbtCommand>();
    wbtCommandMap["write_reverse_map"] = std::make_unique<WriteReverseMapWbtCommand>();
    wbtCommandMap["write_whole_reverse_map"] = std::make_unique<WriteWholeReverseMapWbtCommand>();
    wbtCommandMap["write_reverse_map_entry"] = std::make_unique<WriteReverseMapEntryWbtCommand>();

    // Allocator
    wbtCommandMap["get_bitmap_layout"] = std::make_unique<GetBitmapLayoutWbtCommand>();
    wbtCommandMap["get_instant_meta_info"] = std::make_unique<GetInstantMetaInfoWbtCommand>();

    wbtCommandMap["get_wb_lsid_bitmap"] = std::make_unique<GetWbLsidBitmapWbtCommand>();
    wbtCommandMap["set_wb_lsid_bitmap"] = std::make_unique<SetWbLsidBitmapWbtCommand>();

    wbtCommandMap["get_active_stripe_tail"] = std::make_unique<GetActiveStripeTailWbtCommand>();
    wbtCommandMap["set_active_stripe_tail"] = std::make_unique<SetActiveStripeTailWbtCommand>();

    wbtCommandMap["get_current_ssd_lsid"] = std::make_unique<GetCurrentSsdLsidWbtCommand>();
    wbtCommandMap["set_current_ssd_lsid"] = std::make_unique<SetCurrentSsdLsidWbtCommand>();

    wbtCommandMap["get_segment_info"] = std::make_unique<GetSegmentInfoWbtCommand>();
    wbtCommandMap["set_segment_info"] = std::make_unique<SetSegmentInfoWbtCommand>();
    wbtCommandMap["get_segment_valid_count"] = std::make_unique<GetSegmentValidBlockCountWbtCommand>();

    wbtCommandMap["mfs_create_file"] = std::make_unique<CreateFileWbtCommand>();
    wbtCommandMap["mfs_open_file"] = std::make_unique<OpenFileWbtCommand>();
    wbtCommandMap["mfs_close_file"] = std::make_unique<CloseFileWbtCommand>();
    wbtCommandMap["mfs_read_file"] = std::make_unique<ReadFileWbtCommand>();
    wbtCommandMap["mfs_write_file"] = std::make_unique<WriteFileWbtCommand>();
    wbtCommandMap["mfs_dump_inode_info"] = std::make_unique<DumpInodeInfoWbtCommand>();
    wbtCommandMap["mfs_get_file_size"] = std::make_unique<GetFileSizeWbtCommand>();
    wbtCommandMap["mfs_get_aligned_file_io_size"] = std::make_unique<GetAlignedFileIoSizeWbtCommand>();
    wbtCommandMap["mfs_setup_meta_fio_test"] = std::make_unique<SetupMetaFioTestWbtCommand>();
    wbtCommandMap["mfs_dump_files_list"] = std::make_unique<DumpFilesListWbtCommand>();

    wbtCommandMap["flush_gcov"] = std::make_unique<FlushGcovDataWbtCommand>();
    wbtCommandMap["flush"] = std::make_unique<FlushAllUserDataWbtCommand>();
    wbtCommandMap["translate_device_lba"] = std::make_unique<TranslateDeviceLbaWbtCommand>();
    wbtCommandMap["dump_disk_layout"] = std::make_unique<DumpDiskLayoutWbtCommand>();
    wbtCommandMap["parity_location"] = std::make_unique<ParityLocationWbtCommand>();
    wbtCommandMap["write_uncorrectable_lba"] = std::make_unique<WriteUncorrectableLbaWbtCommand>();
    wbtCommandMap["write_raw"] = std::make_unique<WriteRawDataCommand>();
    wbtCommandMap["read_raw"] = std::make_unique<ReadRawDataCommand>();
    wbtCommandMap["do_gc"] = std::make_unique<DoGcWbtCommand>();
    wbtCommandMap["set_gc_threshold"] = std::make_unique<SetGcThresholdWbtCommand>();
    wbtCommandMap["get_gc_threshold"] = std::make_unique<GetGcThresholdWbtCommand>();
    wbtCommandMap["get_gc_status"] = std::make_unique<GetGcStatusWbtCommand>();

    // Journal Manager
    wbtCommandMap["get_journal_status"] = std::make_unique<GetJournalStatusWbtCommand>();

    // Array
    wbtCommandMap["get_partition_size"] = std::make_unique<GetPartitionSizeWbtCommand>();

    // NVMe Cli
    wbtCommandMap["nvme_cli"] = std::make_unique<NvmeCliCommand>();
    wbtCommandMap["admin-passthru"] = std::make_unique<AdminPassThrough>();

    // Config
    wbtCommandMap["update_config"] = std::make_unique<UpdateConfigWbtCommand>();
}

WbtCmdHandler::WbtCmdHandler(std::string const& commandName)
{
    if (false == wbtCommandsPrepared)
    {
        PrepareWbtCommands();
        wbtCommandsPrepared = true;
    }

    wbtCommandIt = wbtCommandMap.find(commandName);
}

WbtCmdHandler::~WbtCmdHandler(void)
{
}

int64_t
WbtCmdHandler::operator()(Args argv, JsonElement& elem)
{
    int64_t res = -1;

    if (VerifyWbtCommand())
    {
        std::unique_ptr<WbtCommand>& wbtCommand = wbtCommandIt->second;
        std::string coutfile = "output.txt";
        std::ofstream out(coutfile.c_str(), std::ofstream::app);
        out << std::endl
            << "[Info] " << wbtCommand->GetOpcode() << "_" << wbtCommand->GetCommandName() << std::endl;
        out.close();

        res = wbtCommand->Execute(argv, elem);
    }

    return res;
}

int
WbtCmdHandler::GetTestList(std::list<std::string>& testlist)
{
    WbtCommandMap::iterator it = wbtCommandMap.begin();
    for (; it != wbtCommandMap.end(); it++)
    {
        std::unique_ptr<WbtCommand>& wbtCommand = it->second;
        testlist.push_back(wbtCommand->GetCommandName());
    }

    return 0;
}

bool
WbtCmdHandler::VerifyWbtCommand(void)
{
    return wbtCommandIt != wbtCommandMap.end();
}

} // namespace pos
