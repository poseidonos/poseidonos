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

#include "mfs_state_procedure.h"
#include "meta_file_util.h"
#include "metafs_common.h"
#include "mfs_geometry.h"
#include "metafs_mbr_mgr.h"
#include "meta_io_manager.h"
#include "mfs_state_proc_helper.h"
#include "mss.h"

#include <string>

namespace pos
{
MfsStateProcedure::MfsStateProcedure(void)
{
    // Add procedure with the format below
    // * ADD_PROCEDURE(procLookupTable, <MetaFsSystemState>)
    ADD_PROCEDURE(procLookupTable, PowerOn);
    ADD_PROCEDURE(procLookupTable, Init);
    ADD_PROCEDURE(procLookupTable, Ready);
    ADD_PROCEDURE(procLookupTable, Create);
    ADD_PROCEDURE(procLookupTable, Open);
    ADD_PROCEDURE(procLookupTable, Active);
    ADD_PROCEDURE(procLookupTable, Quiesce);
    ADD_PROCEDURE(procLookupTable, Shutdown);
}

MetaFsStateProcedureFuncPointer
MfsStateProcedure::DispatchProcedure(MetaFsSystemState state)
{
    assert(procLookupTable[state] != nullptr);
    return procLookupTable[state];
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_PowerOn(std::string& arrayName)
{
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Init(std::string& arrayName)
{
    MetaFsStorageIoInfoList& mediaInfoList = mbrMap.GetAllStoragePartitionInfo(arrayName);
    for (auto& item : mediaInfoList)
    {
        if (false == item.valid)
            continue;

        MetaVolumeType volumeType = MetaFileUtil::ConvertToVolumeType(item.mediaType);
        MetaLpnType maxVolumeLpn = item.totalCapacity / MetaFsIoConfig::META_PAGE_SIZE_IN_BYTES;

        if (MetaStorageType::SSD == item.mediaType)
        {
            maxVolumeLpn -= mbrMap.GetRegionSizeInLpn(arrayName); // considered due to MBR placement for SSD volume
        }
        mvmTopMgr.Init(volumeType, arrayName, maxVolumeLpn);
    }

    bool isTheFirst = (1 == mbrMap.GetMountedMbrCount()) ? true : false;
    if (isTheFirst)
    {
        mimTopMgr.Init();
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Ready(std::string& arrayName)
{
    bool isTheFirst = (1 == mbrMap.GetMountedMbrCount()) ? true : false;
    if (isTheFirst)
    {
        mvmTopMgr.Bringup();
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Create(std::string& arrayName)
{
    MetaFsStorageIoInfoList& mediaInfoList = mbrMap.GetAllStoragePartitionInfo(arrayName);

    for (auto& item : mediaInfoList)
    {
        if (false == item.valid)
            continue;

        POS_EVENT_ID rc;
        rc = metaStorage->CreateMetaStore(arrayName, item.mediaType, item.totalCapacity, true);
        if (rc != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
                "Failed to mount meta storage subsystem");
            return POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
        }

        MetaVolumeType volumeType = MetaFileUtil::ConvertToVolumeType(item.mediaType);

        if (false == mvmTopMgr.CreateVolume(volumeType, arrayName))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
                "Error occurred to create volume (volume id={})",
                (int)volumeType);
            return POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
        }
    }
    if (true != mbrMap.CreateMBR(arrayName))
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_VOLUME_CREATE_FAILED,
            "Error occurred to create MFS MBR");

        return POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
    }

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Open(std::string& arrayName)
{
    bool isNPOR = false;
    bool isTheFirst = (1 == mbrMap.GetMountedMbrCount()) ? true : false;
    MetaFsStorageIoInfoList& mediaInfoList = mbrMap.GetAllStoragePartitionInfo(arrayName);

    for (auto& item : mediaInfoList)
    {
        if (false == item.valid)
            continue;

        POS_EVENT_ID rc;
        rc = metaStorage->CreateMetaStore(arrayName, item.mediaType, item.totalCapacity);
        if (rc != POS_EVENT_ID::SUCCESS)
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED,
                "Failed to mount meta storage subsystem");
            return POS_EVENT_ID::MFS_META_STORAGE_CREATE_FAILED;
        }
    }

    if (true == mbrMap.LoadMBR(arrayName))
    {
        if (false == mbrMap.IsValidMBRExist(arrayName))
        {
            MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_FILE_NOT_FOUND,
                "Filesystem MBR has been corrupted or Filesystem cannot be found.");
            return POS_EVENT_ID::MFS_FILE_NOT_FOUND;
        }
        isNPOR = mbrMap.GetPowerStatus(arrayName);

        if (isNPOR == true)
        {
            MFS_TRACE_INFO((int)POS_EVENT_ID::MFS_INFO_MESSAGE,
                "This open is NPOR case!!!");
        }
    }
    else
    {
        MFS_TRACE_ERROR((int)POS_EVENT_ID::MFS_META_LOAD_FAILED,
            "Error occurred while loading filesystem MBR");
        return POS_EVENT_ID::MFS_META_LOAD_FAILED;
    }

    if (false == mvmTopMgr.Open(isNPOR, arrayName))
    {
        return POS_EVENT_ID::MFS_META_VOLUME_OPEN_FAILED;
    }

    if (isTheFirst)
    {
        mimTopMgr.Bringup();
    }

#if (1 == COMPACTION_EN) || not defined COMPACTION_EN
    if (false == mvmTopMgr.Compaction(isNPOR, arrayName))
    {
        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "compaction method returns false");
    }
#endif

    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Quiesce(std::string& arrayName)
{
    return POS_EVENT_ID::SUCCESS;
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Shutdown(std::string& arrayName)
{
    bool resetCxt = false;
    try
    {
        if (false == mvmTopMgr.Close(resetCxt /* output */, arrayName)) // close volumes, and clear context
        {
            if (resetCxt == true)
            { // Reset MetaFS DRAM Context
                throw POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED;
            }
            else
            {
                throw POS_EVENT_ID::MFS_META_VOLUME_CLOSE_FAILED_DUE_TO_ACTIVE_FILE;
            }
        }

        mbrMap.SetPowerStatus(arrayName, true /*NPOR status*/);

        // FIXME: remove this code when MFS supports RAID1 along with FT layer
        if (true != mbrMap.SaveContent(arrayName))
        {
            throw POS_EVENT_ID::MFS_META_SAVE_FAILED;
        }

        MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
            "Meta filesystem is in shutdown state. Now it's safe to turn mgmt power off..");
        throw POS_EVENT_ID::SUCCESS;
    }
    catch (POS_EVENT_ID event)
    {
        if (true == resetCxt)
        {
            mimTopMgr.Close();
            mbrMap.Remove(arrayName);
        }
        metaStorage->Close(arrayName);
        return event;
    }
}

POS_EVENT_ID
MfsStateProcedure::_ProcessSystemState_Active(std::string& arrayName)
{
    MFS_TRACE_DEBUG((int)POS_EVENT_ID::MFS_DEBUG_MESSAGE,
        "Meta filesystem is in active state");
    return POS_EVENT_ID::SUCCESS;
}
} // namespace pos
