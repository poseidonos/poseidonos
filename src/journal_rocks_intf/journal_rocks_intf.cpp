/*
 *   BSD LICENSE
 *   Copyright (c) 2022 Samsung Electronics Corporation
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

#include "src/journal_rocks_intf/journal_rocks_intf.h"

#include <string>
#include <experimental/filesystem>

#include "src/include/pos_event_id.h"
#include "src/logger/logger.h"

namespace pos
{
JournalRocksIntf::JournalRocksIntf(void)
: pathName(""),
  isOpened(false)
{
}

JournalRocksIntf::JournalRocksIntf(const std::string arrayName)
: JournalRocksIntf()
{
    this->pathName = "/etc/pos/" + arrayName + "_RocksJournal";
    _CreateDirectory();
}

// LCOV_EXCL_START
JournalRocksIntf::~JournalRocksIntf(void)
{
}
// LCOV_EXCL_STOP

int
JournalRocksIntf::Open(void)
{
    rocksdb::Options options;
    options.create_if_missing = true;
    rocksdb::Status status = rocksdb::DB::Open(options, pathName, &rocksJournal);
    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_OPENED), "RocksDB Journal opened (path : {})", pathName);

    //TODO(sang7.park) : have to consider how to return statuses properly
    if (status.ok() == true)
    {
        isOpened = true;
        return static_cast<int>(POS_EVENT_ID::SUCCESS);
    }
    else
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_OPEN_FAILED), "RocksDB Journal open failed (path : {}), RocksDB open status : {}", pathName, status.code());
        return -1;
    }
}

int
JournalRocksIntf::Close(void)
{
    isOpened = false;
    if (rocksJournal != nullptr)
    {
        delete rocksJournal;
        rocksJournal = nullptr;
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_CLOSED), "RocksDB Journal Closed (path :{}) ", pathName);
    }
    else
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_CLOSED), "RocksDB Journal does not exist (path :{}) ", pathName);
    }
    return static_cast<int>(POS_EVENT_ID::SUCCESS);
}

int 
JournalRocksIntf::AddJournal(void){
    return 0;
}

int 
JournalRocksIntf::ReadAllJournal(void){
    return 0;
}

int 
JournalRocksIntf::ResetJournalByKey(void){
    return 0;
}

int 
JournalRocksIntf::ResetAllJournal(void){
    return 0;
}

bool
JournalRocksIntf::IsOpened(void)
{
    return isOpened;
}

bool 
JournalRocksIntf::_CreateDirectory(void)
{
    if (!std::experimental::filesystem::exists(pathName))
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_DIR_EXISTS), "RocksDB directory already exists (path :{}) ", pathName);
        return true;
    }

    bool ret = std::experimental::filesystem::create_directory(pathName);
    if (ret != true)
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_DIR_CREATION_FAILED), "RocksDB directory creation failed (path :{}) ", pathName);
        return ret;
    }

    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_DIR_CREATED), "RocksDB directory created (path :{}) ", pathName);
    return ret;
}


//TODO(sang7.park) : This method is supposed to be used when array is removed.
bool
JournalRocksIntf::DeleteDirectory(void)
{
    bool ret = std::experimental::filesystem::remove(pathName);
    if (ret != true)
    {
        POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_DIR_DELETION_FAILED), "RocksDB directory does not exists, so deletion failed (path :{}) ", pathName);
        return ret;
    }
    POS_TRACE_INFO(static_cast<int>(POS_EVENT_ID::JOURNAL_ROCKS_DIR_DELETED), "RocksDB directory deleted (path :{}) ", pathName);
    return ret;
}
} // namespace pos
