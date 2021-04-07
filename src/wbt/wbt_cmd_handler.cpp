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

#include "src/wbt/wbt_cmd_handler.h"

#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "src/allocator/allocator.h"
#include "src/array/array.h"
#include "src/array/ft/raid5.h"
#include "src/array/partition/stripe_partition.h"
#include "src/device/device_manager.h"
#include "src/device/ublock_device.h"
#include "src/gc/garbage_collector.h"
#include "src/helper/json_helper.h"
#include "src/io/general_io/io_controller.h"
#include "src/io/general_io/ubio.h"
#include "src/mapper/mapper.h"
#include "src/scheduler/event_argument.h"
#include "src/scheduler/io_dispatcher.h"
#include "src/volume/volume_manager.h"

extern "C" void __gcov_flush();

namespace ibofos
{
WbtCmdHandler::WbtCmdHandler(std::string const& testName)
{
    cmdInfo.opcode = INVALID_COMMAND;

    for (int cmdCnt = 0; cmdCnt < NUM_WBT_CMDS; cmdCnt++)
    {
        if (cmdMap[cmdCnt].testName == testName)
        {
            cmdInfo = cmdMap[cmdCnt];
            break;
        }
    }
}

int64_t
WbtCmdHandler::operator()(Args argv, JsonElement& elem)
{
    int64_t res = 0;
    ofstream out(coutfile.c_str(), std::ofstream::app);
    out << std::endl
        << "[Info] " << cmdInfo.testName << std::endl;
    out.close();

    switch (cmdInfo.opcode)
    {
        case GET_MAP_LAYOUT:
            res = _GetMapLayout();
            break;
        case READ_VSAMAP:
            res = _ReadVsamap(argv);
            break;
        case READ_VSAMAP_ENTRY:
            res = _ReadVsamapEntry(argv);
            break;
        case WRITE_VSAMAP:
            res = _WriteVsamap(argv);
            break;
        case WRITE_VSAMAP_ENTRY:
            res = _WriteVsamapEntry(argv);
            break;
        case READ_STRIPEMAP:
            res = _ReadStripemap(argv);
            break;
        case READ_STRIPEMAP_ENTRY:
            res = _ReadStripemapEntry(argv);
            break;
        case WRITE_STRIPEMAP:
            res = _WriteStripemap(argv);
            break;
        case WRITE_STRIPEMAP_ENTRY:
            res = _WriteStripemapEntry(argv);
            break;
        case READ_REVERSE_MAP:
            res = _ReadReverseMap(argv);
            break;
        case READ_WHOLE_REVERSE_MAP:
            res = _ReadWholeReverseMap(argv);
            break;
        case READ_REVERSE_MAP_ENTRY:
            res = _ReadReverseMapEntry(argv);
            break;
        case WRITE_REVERSE_MAP:
            res = _WriteReverseMap(argv);
            break;
        case WRITE_WHOLE_REVERSE_MAP:
            res = _WriteWholeReverseMap(argv);
            break;
        case WRITE_REVERSE_MAP_ENTRY:
            res = _WriteReverseMapEntry(argv);
            break;
        case GET_BITMAP_LAYOUT:
            res = _GetBitmapLayout();
            break;
        case GET_INSTANT_META_INFO:
            res = _GetInstantMetaInfo();
            break;
        case GET_WB_LSID_BITMAP:
            res = _GetWbLsidBitmap(argv);
            break;
        case SET_WB_LSID_BITMAP:
            res = _SetWbLsidBitmap(argv);
            break;
        case GET_ACTIVE_STRIPE_TAIL:
            res = _GetActiveStripeTail(argv);
            break;
        case SET_ACTIVE_STRIPE_TAIL:
            res = _SetActiveStripeTail(argv);
            break;
        case GET_CURRENT_SSD_LSID:
            res = _GetCurrentSsdLsid(argv);
            break;
        case SET_CURRENT_SSD_LSID:
            res = _SetCurrentSsdLsid(argv);
            break;
        case GET_USER_SEGMENT_BITMAP:
            res = _GetUserSegmentBitmap(argv);
            break;
        case SET_USER_SEGMENT_BITMAP:
            res = _SetUserSegmentBitmap(argv);
            break;
        case GET_SEGMENT_INFO:
            res = _GetSegmentInfo(argv);
            break;
        case SET_SEGMENT_INFO:
            res = _SetSegmentInfo(argv);
            break;
        case GET_SEGMENT_INV_COUNT:
            res = _GetSegmentInvalidCount(argv);
            break;
        case MFS_DUMP_FILES_LIST:
            res = metaCmdHandler.DumpFilesList(argv);
            break;
        case MFS_CREATE_FILESYSTEM:
            res = metaCmdHandler.CreateFileSystem();
            break;
        case MFS_MOUNT_FILESYSTEM:
            res = metaCmdHandler.MountFileSystem();
            break;
        case MFS_UNMOUNT_FILESYSTEM:
            res = metaCmdHandler.UmountFileSystem();
            break;
        case MFS_CREATE_FILE:
            res = metaCmdHandler.CreateFile(argv);
            break;
        case MFS_OPEN_FILE:
            res = metaCmdHandler.OpenFile(argv);
            break;
        case MFS_CLOSE_FILE:
            res = metaCmdHandler.CloseFile(argv);
            break;
        case MFS_READ_FILE:
            res = metaCmdHandler.ReadFile(argv);
            break;
        case MFS_WRITE_FILE:
            res = metaCmdHandler.WriteFile(argv);
            break;
        case MFS_GET_FILE_SIZE:
            res = metaCmdHandler.GetFileSize(argv);
            break;
        case MFS_GET_ALIGNED_FILE_IO_SIZE:
            res = metaCmdHandler.GetAlignedFileIOSize(argv);
            break;
        case MFS_DUMP_INODE_INFO:
            res = metaCmdHandler.DumpInodeInfo(argv);
            break;
        case MFS_SET_INODE_INFO:
            res = metaCmdHandler.SetInodeInfo(argv);
            break;
        case MFS_GET_MAX_FILE_SIZE:
            res = metaCmdHandler.GetMaxFileSize(argv);
            break;
        case MFS_SETUP_META_FIO_TEST:
            res = metaCmdHandler.SetupMetaFioTest(argv);
            break;
        /*case MFS_GET_FILE_CHECKSUM:
        res = metaCmdHandler.GetFileChecksum(argv);
        break;*/
        case FLUSH_GCOV_DATA:
            res = _FlushGcovData();
            break;
        case FLUSH_USER_DATA:
            res = _FlushAllUserData();
            break;
        case TRANSLATE_DEVICE_LBA:
            res = _TranslateDeviceLba(argv);
            break;
        case DUMP_DISK_LAYOUT:
            res = _DumpDiskLayout();
            break;
        case PARITY_LOCATION:
            res = _ParityLocation(argv);
            break;
        case WRITE_UNCORRECTABLE_LBA:
            res = _PassThroughToDevice(argv);
            break;
        case WRITE_RAW_DATA:
            res = _WriteRaw(argv);
            break;
        case READ_RAW_DATA:
            res = _ReadRaw(argv);
            break;
        case DO_GC:
            res = _DoGc();
            break;
        case SET_GC_THRESHOLD:
            res = _SetGcThreshold(argv);
            break;
        case GET_GC_THRESHOLD:
            res = _GetGcThreshold(elem);
            break;
        case GET_GC_STATUS:
            res = _GetGcStatus(elem);
            break;
        default:
            res = -1;
            break;
    }

    return res;
}

int
WbtCmdHandler::GetTestList(std::list<std::string>& testlist)
{
    for (int cmdCnt = LIST_WBT_CMDS + 1; cmdCnt < NUM_WBT_CMDS; cmdCnt++)
    {
        testlist.push_back(cmdMap[cmdCnt].testName);
    }
    return 0;
}

int
WbtCmdHandler::_GetMapLayout(void)
{
    return MapperSingleton::Instance()->GetMapLayout(coutfile);
}

int
WbtCmdHandler::_GetBitmapLayout(void)
{
    return AllocatorSingleton::Instance()->GetBitmapLayout(coutfile);
}

int
WbtCmdHandler::_GetInstantMetaInfo(void)
{
    return AllocatorSingleton::Instance()->GetInstantMetaInfo(coutfile);
}

int
WbtCmdHandler::_ReadVsamap(Args argv)
{
    int res = 0;

    int volId = VolumeManagerSingleton::Instance()->VolumeID(
        argv["name"].get<std::string>());

    if (volId < 0)
    {
        res = volId;
    }
    else
    {
        res = MapperSingleton::Instance()->ReadVsaMap(volId,
            argv["output"].get<std::string>());
    }

    return res;
}

int
WbtCmdHandler::_ReadVsamapEntry(Args argv)
{
    int res = -1;

    int volId = VolumeManagerSingleton::Instance()->VolumeID(
        argv["name"].get<std::string>());

    if (volId < 0)
    {
        res = volId;
    }
    else
    {
        try
        {
            BlkAddr rba = static_cast<BlkAddr>(std::stoull(argv["rba"].get<std::string>()));

            res = MapperSingleton::Instance()->ReadVsaMapEntry(volId, rba, coutfile);
        }
        catch (const std::exception& e)
        {
            IBOF_TRACE_ERROR(res, e.what());
        }
    }

    return res;
}

int
WbtCmdHandler::_WriteVsamap(Args argv)
{
    int res;

    int volId = VolumeManagerSingleton::Instance()->VolumeID(
        argv["name"].get<std::string>());

    if (volId < 0)
    {
        res = volId;
    }
    else
    {
        res = MapperSingleton::Instance()->WriteVsaMap(volId,
            argv["input"].get<std::string>());
    }

    return res;
}

int
WbtCmdHandler::_WriteVsamapEntry(Args argv)
{
    int res;

    int volId = VolumeManagerSingleton::Instance()->VolumeID(
        argv["name"].get<std::string>());

    if (volId < 0)
    {
        res = volId;
    }
    else
    {
        BlkAddr rba = static_cast<BlkAddr>(std::stoull(argv["rba"].get<std::string>()));
        VirtualBlkAddr vsa =
            {
                .stripeId = static_cast<StripeId>(std::stoul(argv["vsid"].get<std::string>())),
                .offset = static_cast<BlkOffset>(std::stoull(argv["offset"].get<std::string>()))};

        res = MapperSingleton::Instance()->WriteVsaMapEntry(volId, rba, vsa);
    }

    return res;
}

int
WbtCmdHandler::_ReadStripemap(Args argv)
{
    int res = MapperSingleton::Instance()->ReadStripeMap(
        argv["output"].get<std::string>());

    return res;
}

int
WbtCmdHandler::_ReadStripemapEntry(Args argv)
{
    StripeId vsid = static_cast<StripeId>(std::stoul(
        argv["vsid"].get<std::string>()));
    int res = MapperSingleton::Instance()->ReadStripeMapEntry(vsid, coutfile);

    return res;
}

int
WbtCmdHandler::_WriteStripemap(Args argv)
{
    int res = MapperSingleton::Instance()->WriteStripeMap(
        argv["input"].get<std::string>());

    return res;
}

int
WbtCmdHandler::_WriteStripemapEntry(Args argv)
{
    StripeId vsid = static_cast<StripeId>(std::stoul(
        argv["vsid"].get<std::string>()));
    StripeLoc loc = static_cast<StripeLoc>(std::stoi(
        argv["loc"].get<std::string>()));
    StripeId lsid = static_cast<StripeId>(std::stoul(
        argv["lsid"].get<std::string>()));
    int res = MapperSingleton::Instance()->WriteStripeMapEntry(vsid, loc, lsid);

    return res;
}

int
WbtCmdHandler::_ReadReverseMap(Args argv)
{
    StripeId vsid = static_cast<StripeId>(std::stoul(
        argv["vsid"].get<std::string>()));
    int res = MapperSingleton::Instance()->ReadReverseMap(vsid,
        argv["output"].get<std::string>());

    return res;
}

int
WbtCmdHandler::_ReadWholeReverseMap(Args argv)
{
    int res;

    res = MapperSingleton::Instance()->ReadWholeReverseMap(
        argv["output"].get<std::string>());

    return res;
}

int
WbtCmdHandler::_WriteReverseMap(Args argv)
{
    StripeId vsid = static_cast<StripeId>(std::stoul(
        argv["vsid"].get<std::string>()));
    int res = MapperSingleton::Instance()->WriteReverseMap(vsid,
        argv["input"].get<std::string>());

    return res;
}

int
WbtCmdHandler::_WriteWholeReverseMap(Args argv)
{
    int res = MapperSingleton::Instance()->WriteWholeReverseMap(
        argv["input"].get<std::string>());

    return res;
}

int
WbtCmdHandler::_ReadReverseMapEntry(Args argv)
{
    StripeId vsid = static_cast<StripeId>(std::stoul(
        argv["vsid"].get<std::string>()));
    BlkOffset offset = static_cast<BlkOffset>(std::stoull(
        argv["offset"].get<std::string>()));
    int res = MapperSingleton::Instance()->ReadReverseMapEntry(vsid, offset,
        coutfile);

    return res;
}

int
WbtCmdHandler::_WriteReverseMapEntry(Args argv)
{
    int res = 0;

    StripeId vsid = static_cast<StripeId>(std::stoul(
        argv["vsid"].get<std::string>()));
    BlkOffset offset = static_cast<BlkOffset>(std::stoull(
        argv["offset"].get<std::string>()));
    BlkAddr rba = static_cast<BlkAddr>(std::stoull(argv["rba"].get<std::string>()));

    int volId = VolumeManagerSingleton::Instance()->VolumeID(
        argv["name"].get<std::string>());
    if (volId < 0)
    {
        res = volId;
    }
    else
    {
        res = MapperSingleton::Instance()->WriteReverseMapEntry(vsid, offset, rba,
            volId);
    }

    return res;
}

int
WbtCmdHandler::_GetWbLsidBitmap(Args argv)
{
    return AllocatorSingleton::Instance()->GetMeta(WB_LSID_BITMAP,
        argv["output"].get<std::string>());
}

int
WbtCmdHandler::_SetWbLsidBitmap(Args argv)
{
    return AllocatorSingleton::Instance()->SetMeta(WB_LSID_BITMAP,
        argv["input"].get<std::string>());
}

int
WbtCmdHandler::_GetActiveStripeTail(Args argv)
{
    return AllocatorSingleton::Instance()->GetMeta(ACTIVE_STRIPE_TAIL,
        argv["output"].get<std::string>());
}

int
WbtCmdHandler::_SetActiveStripeTail(Args argv)
{
    return AllocatorSingleton::Instance()->SetMeta(ACTIVE_STRIPE_TAIL,
        argv["input"].get<std::string>());
}

int
WbtCmdHandler::_GetCurrentSsdLsid(Args argv)
{
    return AllocatorSingleton::Instance()->GetMeta(CURRENT_SSD_LSID,
        argv["output"].get<std::string>());
}

int
WbtCmdHandler::_SetCurrentSsdLsid(Args argv)
{
    return AllocatorSingleton::Instance()->SetMeta(CURRENT_SSD_LSID,
        argv["input"].get<std::string>());
}

int
WbtCmdHandler::_GetUserSegmentBitmap(Args argv)
{
    return AllocatorSingleton::Instance()->GetMeta(SEGMENT_BITMAP,
        argv["output"].get<std::string>());
}

int
WbtCmdHandler::_SetUserSegmentBitmap(Args argv)
{
    return AllocatorSingleton::Instance()->SetMeta(SEGMENT_BITMAP,
        argv["input"].get<std::string>());
}

int
WbtCmdHandler::_GetSegmentInfo(Args argv)
{
    return AllocatorSingleton::Instance()->GetMeta(SEGMENT_INFO,
        argv["output"].get<std::string>());
}

int
WbtCmdHandler::_SetSegmentInfo(Args argv)
{
    return AllocatorSingleton::Instance()->SetMeta(SEGMENT_INFO,
        argv["input"].get<std::string>());
}

int
WbtCmdHandler::_GetSegmentInvalidCount(Args argv)
{
    return AllocatorSingleton::Instance()->GetMeta(SEGMENT_INVALID_CNT,
        argv["output"].get<std::string>());
}

int
WbtCmdHandler::_FlushGcovData(void)
{
    int res = 1;
#ifdef IBOF_CONFIG_GCOV
    __gcov_flush();
    res = 0;
#endif
    return res;
}

int
WbtCmdHandler::_FlushAllUserData(void)
{
    int res = -1;
    StateManager* stateManager = StateManagerSingleton::Instance();
    State posState = stateManager->GetState();
    if ((State::NORMAL == posState) || (State::BUSY == posState))
    {
        std::cout << "Start Flush process..." << std::endl;
        // IO Flush & Quiesce
        AllocatorSingleton::Instance()->FlushAllUserdataWBT();
        std::cout << "Stripes finalized..." << std::endl;
        res = 0;
    }
    else
    {
        std::string currentPOSState =
            StatePolicySingleton::Instance()->ToString(posState);
        std::cout << "The state of POS is not proper for flush: " << currentPOSState << std::endl;
    }

    return res;
}

int
WbtCmdHandler::_TranslateDeviceLba(Args argv)
{
    LogicalBlkAddr lsa =
        {
            .stripeId = static_cast<uint32_t>(strtoul(argv["lsid"].get<std::string>().c_str(), nullptr, 0)),
            .offset = strtoul(argv["offset"].get<std::string>().c_str(), nullptr, 0)};

    ofstream out(coutfile.c_str(), std::ofstream::app);
    out << "logical stripe : " << lsa.stripeId << std::endl;
    out << "logical offset : " << lsa.offset << std::endl;

    PhysicalBlkAddr pba;
    int ret = ArraySingleton::Instance()->Translate(USER_DATA, pba, lsa);
    if (ret != 0 || pba.dev->uBlock == nullptr)
    {
        out << "translation failed" << std::endl;
    }
    else
    {
        string deviceName = pba.dev->uBlock->GetName();
        uint64_t lba = pba.lba;

        out << "device name : " << deviceName << std::endl;
        out << "lba : " << lba << std::endl;
        out << std::endl;
    }
    out.close();

    return 0;
}

int
WbtCmdHandler::_DumpDiskLayout(void)
{
    Array* array = ArraySingleton::Instance();

    ofstream out(coutfile.c_str(), std::ofstream::app);

    LogicalBlkAddr lsa = {.stripeId = 0, .offset = 0};

    PhysicalBlkAddr pba;
    array->Translate(META_SSD, pba, lsa);
    out << "meta ssd start lba : " << pba.lba << std::endl;

    const PartitionLogicalSize* logicalSize = array->GetSizeInfo(META_SSD);
    uint64_t blk = (uint64_t)logicalSize->totalStripes *
        logicalSize->blksPerChunk * ArrayConfig::SECTORS_PER_BLOCK;
    out << "meta ssd end lba : " << pba.lba + blk - 1 << std::endl;

    array->Translate(USER_DATA, pba, lsa);
    out << "user data start lba : " << pba.lba << std::endl;

    logicalSize = array->GetSizeInfo(USER_DATA);
    blk = (uint64_t)logicalSize->totalStripes * logicalSize->blksPerChunk * ArrayConfig::SECTORS_PER_BLOCK;
    out << "user data end lba : " << pba.lba + blk - 1 << std::endl;

    if (0 /*useNvm*/)
    {
        array->Translate(WRITE_BUFFER, pba, lsa);
        out << "write buffer start lba : " << pba.lba << std::endl;

        logicalSize = array->GetSizeInfo(WRITE_BUFFER);
        blk = (uint64_t)logicalSize->totalStripes * logicalSize->blksPerChunk * ArrayConfig::SECTORS_PER_BLOCK;
        out << "write buffer end lba : " << pba.lba + blk - 1 << std::endl;
    }

    out << std::endl;
    out.close();

    return 0;
}

int
WbtCmdHandler::_ParityLocation(Args& argv)
{
    ofstream out(coutfile.c_str(), std::ofstream::app);

    if (!argv.contains("dev") || !argv.contains("lba"))
    {
        out << "invalid parameter" << endl;
        return 0;
    }

    string devName = argv["dev"].get<std::string>();
    uint64_t lba = atoi(argv["lba"].get<std::string>().c_str());

    Array& sysArray = *ArraySingleton::Instance();
    if (sysArray.state_.IsMounted() == false)
    {
        out << "array is not mounted" << endl;
        return 0;
    }

    ArrayDeviceType devType;
    ArrayDevice* dev = nullptr;
    DevName name(devName);
    UBlockDevice* uBlock = DeviceManagerSingleton::Instance()->GetDev(name);
    tie(dev, devType) = sysArray.devMgr_.GetDev(uBlock);
    if (dev == nullptr || devType != ArrayDeviceType::DATA)
    {
        out << "device not found" << endl;
        return 0;
    }
    PhysicalBlkAddr pba = {.dev = dev,
        .lba = lba};
    StripePartition* ptn = dynamic_cast<StripePartition*>(
        sysArray.ptnMgr_.partitions_[PartitionType::USER_DATA]);
    Raid5* method = dynamic_cast<Raid5*>(ptn->GetMethod());

    FtBlkAddr fba = ptn->_P2FTranslate(pba);
    uint32_t parityIndex = method->_GetParityOffset(fba.stripeId);
    ArrayDevice* parityDev = ptn->devs_.at(parityIndex);

    out << "device name : " << parityDev->uBlock->GetName() << endl;
    out << "lba : " << lba << endl;

    out << std::endl;
    out.close();
    return 0;
}

int
WbtCmdHandler::_PassThroughToDevice(Args argv)
{
    int returnValue = -1;

    // cmdInfo.opcode
    switch (cmdInfo.opcode)
    {
        case WRITE_UNCORRECTABLE_LBA:
            returnValue = _WriteUncorrectable(argv);
            break;

        default:
            break;
    }

    return returnValue;
}

int
WbtCmdHandler::_WriteUncorrectable(Args argv)
{
    int returnValue = -1;

    if (argv.contains("dev") && argv.contains("lba"))
    {
        DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
        DevName _dev(argv["dev"].get<std::string>());
        UBlockDevice* targetDevice = deviceMgr->GetDev(_dev);
        ArrayDevice* arrayDev = new ArrayDevice(targetDevice);
        if (nullptr != targetDevice)
        {
            uint8_t dummyBuffer[Ubio::BYTES_PER_UNIT];
            uint32_t unitCount = 1;
            UbioSmartPtr ubio(new Ubio(dummyBuffer, unitCount));
            ubio->dir = UbioDir::WriteUncor;
            PhysicalBlkAddr pba = {.dev = arrayDev,
                .lba = std::stoull(argv["lba"].get<std::string>())};
            ubio->SetPba(pba);

            IODispatcher* ioDispatcher = EventArgument::GetIODispatcher();
            int retValue = ioDispatcher->Submit(ubio, true);
            if (retValue >= 0 &&
                ubio->GetError() == CallbackError::SUCCESS)
            {
                returnValue = 0;
            }
        }
        delete arrayDev;
    }

    return returnValue;
}

std::string
WbtCmdHandler::_GetParameter(Args& argv, const char* paramToSearch)
{
    if (argv.contains(paramToSearch))
    {
        return argv[paramToSearch].get<std::string>();
    }

    std::string emptyString;

    return emptyString;
}

uint32_t
WbtCmdHandler::_ConvertValueToUINT32(std::string& value)
{
    uint32_t startOffsetOfRealValue = value.find("0x");
    if (std::string::npos != startOffsetOfRealValue)
    {
        return std::stoul(value.substr(startOffsetOfRealValue), nullptr, 16);
    }

    return std::stoul(value);
}
bool
WbtCmdHandler::_VerifyWriteReadRawCommonParameters(Args& argv)
{
    bool parametersValid = false;

    std::string diskName = _GetParameter(argv, "dev");
    std::string startOffset = _GetParameter(argv, "lba");
    std::string numLogicalBlockCount = _GetParameter(argv, "count");

    if ((0 < diskName.length()) && (0 < startOffset.length()) &&
        (0 < numLogicalBlockCount.length()))
    {
        parametersValid = true;
    }

    return parametersValid;
}

int
WbtCmdHandler::_WriteRaw(Args argv)
{
    int returnValue = -1;

    if ((true == _VerifyWriteReadRawCommonParameters(argv)) &&
        (true == _VerifyWriteRawSpecificParameters(argv)))
    {
        std::string numLogicalBlockCount = _GetParameter(argv, "count");

        const uint32_t HUGE_PAGE_SIZE_IN_BYTES = 2 * 1024 * 1024;
        uint32_t unitCount = HUGE_PAGE_SIZE_IN_BYTES / Ubio::BYTES_PER_UNIT;
        if (unitCount > std::stoul(numLogicalBlockCount)) // NLB is zero based.
        {
            std::string diskName = _GetParameter(argv, "dev");
            DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
            DevName dev(diskName);

            UBlockDevice* targetDevice = deviceMgr->GetDev(dev);
            if (nullptr != targetDevice)
            {
                unitCount = std::stoul(numLogicalBlockCount) + 1;

                UbioSmartPtr ubio(new Ubio(nullptr, unitCount));
                ubio->dir = UbioDir::Write;

                std::string startOffset = _GetParameter(argv, "lba");
                ArrayDevice* arrayDev = new ArrayDevice(targetDevice);
                if (nullptr != arrayDev)
                {
                    PhysicalBlkAddr pba = {.dev = arrayDev,
                        .lba = std::stoull(startOffset)};
                    ubio->SetPba(pba);

                    returnValue = _FilloutPayloadToWriteRaw(argv,
                        ubio->GetBuffer(), ubio->GetMemSize());
                    if (0 == returnValue)
                    {
                        IODispatcher* ioDispatcher =
                            EventArgument::GetIODispatcher();
                        int retValue = ioDispatcher->Submit(ubio, true);
                        if (retValue < 0 || ubio->GetError() != CallbackError::SUCCESS)
                        {
                            returnValue = -1;
                        }
                    }

                    delete arrayDev;
                }
            }
        }
    }

    return returnValue;
}

bool
WbtCmdHandler::_VerifyWriteRawSpecificParameters(Args& argv)
{
    bool parametersValid = false;

    std::string pattern = _GetParameter(argv, "pattern");
    std::string inputFileName = _GetParameter(argv, "input");

    if (0 < pattern.length())
    {
        parametersValid = true;
    }

    if (0 < inputFileName.length())
    {
        int errorCode = access(inputFileName.c_str(), R_OK);
        if (0 == errorCode)
        {
            parametersValid = true;
        }
    }

    return parametersValid;
}

int
WbtCmdHandler::_FilloutPayloadToWriteRaw(Args& argv,
    void* buffer, uint32_t bytesToWrite)
{
    int returnValue = -1;

    std::string pattern = _GetParameter(argv, "pattern");
    if (0 < pattern.length())
    {
        uint32_t patternToInput = _ConvertValueToUINT32(pattern);
        _WritePattern(buffer, patternToInput, bytesToWrite);
        returnValue = 0;
    }
    else
    {
        std::string inputFileName = _GetParameter(argv, "input");
        int fd = open(inputFileName.c_str(), O_RDONLY);
        if (2 < fd)
        {
            int bytesRead = read(fd, buffer, bytesToWrite);

            if (0 < bytesRead)
            {
                if (static_cast<uint32_t>(bytesRead) < bytesToWrite)
                {
                    uint8_t* bufferToFill = static_cast<uint8_t*>(buffer);
                    uint32_t remainingBytes = bytesToWrite - bytesRead;
                    memset(&bufferToFill[bytesRead], 0xCE, remainingBytes);
                }

                returnValue = 0;
            }
        }

        if (fd != -1)
        {
            close(fd);
        }
    }

    return returnValue;
}

int
WbtCmdHandler::_WritePattern(void* buf, uint32_t pattern, uint32_t bytesToWrite)
{
    int bytesWritten = 0;
    uint32_t* bucket = static_cast<uint32_t*>(buf);
    uint32_t bucketCount = bytesToWrite / sizeof(uint32_t);

    for (uint32_t bucketIndex = 0; bucketIndex < bucketCount; bucketIndex++)
    {
        bucket[bucketIndex] = pattern;
    }

    bytesWritten = bucketCount * sizeof(uint32_t);

    return bytesWritten;
}

int
WbtCmdHandler::_ReadRaw(Args argv)
{
    int returnValue = -1;

    if ((true == _VerifyWriteReadRawCommonParameters(argv)) &&
        (true == _VerifyReadRawSpecificParameters(argv)))
    {
        std::string numLogicalBlockCount = _GetParameter(argv, "count");

        const uint32_t HUGE_PAGE_SIZE_IN_BYTES = 2 * 1024 * 1024;
        uint32_t unitCount = HUGE_PAGE_SIZE_IN_BYTES / Ubio::BYTES_PER_UNIT;
        if (unitCount > std::stoul(numLogicalBlockCount)) // NLB is zero based.
        {
            std::string diskName = _GetParameter(argv, "dev");
            DeviceManager* deviceMgr = DeviceManagerSingleton::Instance();
            DevName dev(diskName);

            UBlockDevice* targetDevice = deviceMgr->GetDev(dev);
            if (nullptr != targetDevice)
            {
                std::string startOffset = _GetParameter(argv, "lba");
                unitCount = std::stoul(numLogicalBlockCount) + 1;

                UbioSmartPtr ubio(new Ubio(nullptr, unitCount));
                ubio->dir = UbioDir::Read;
                ArrayDevice* arrayDev = new ArrayDevice(targetDevice);
                if (nullptr != arrayDev)
                {
                    PhysicalBlkAddr pba = {.dev = arrayDev,
                        .lba = std::stoull(startOffset)};
                    ubio->SetPba(pba);

                    IODispatcher* ioDispatcher =
                        EventArgument::GetIODispatcher();
                    int retValue = ioDispatcher->Submit(ubio, true);
                    if (retValue >= 0 &&
                        ubio->GetError() == CallbackError::SUCCESS)
                    {
                        returnValue = _DumpPayloadFromReadRaw(argv,
                            ubio->GetBuffer(), ubio->GetMemSize());
                    }

                    delete arrayDev;
                }
            }
        }
    }

    return returnValue;
}

bool
WbtCmdHandler::_VerifyReadRawSpecificParameters(Args& argv)
{
    bool parametersValid = false;

    std::string inputFileName = _GetParameter(argv, "output");

    if (0 < inputFileName.length())
    {
        int errorCode = access(inputFileName.c_str(), F_OK);
        if (-1 == errorCode)
        {
            parametersValid = true;
        }

        errorCode = access(inputFileName.c_str(), W_OK);
        if (0 == errorCode)
        {
            parametersValid = true;
        }
    }

    return parametersValid;
}

int
WbtCmdHandler::_DumpPayloadFromReadRaw(Args& argv,
    void* buffer, uint32_t bytesToRead)
{
    int returnValue = -1;

    std::string inputFileName = _GetParameter(argv, "output");
    if (0 < inputFileName.length())
    {
        int flag = O_CREAT | O_WRONLY | O_TRUNC;
        int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        int fd = open(inputFileName.c_str(), flag, mode);
        if (2 < fd)
        {
            int bytesWritten = write(fd, buffer, bytesToRead);
            if (0 < bytesWritten)
            {
                if (static_cast<uint32_t>(bytesWritten) < bytesToRead)
                {
                    uint8_t* bufferToFill = static_cast<uint8_t*>(buffer);
                    uint32_t remainingBytes = bytesToRead - bytesWritten;
                    memset(&bufferToFill[bytesWritten], 0xFD, remainingBytes);
                }

                returnValue = 0;
            }
        }
        if (fd != -1)
        {
            close(fd);
        }
    }

    return returnValue;
}

int
WbtCmdHandler::_DoGc(void)
{
    int returnValue = -1;
    int isPossible = GarbageCollectorSingleton::Instance()->IsGcPossible();

    if (0 != isPossible)
    {
        return returnValue;
    }

    int gcStarted = GarbageCollectorSingleton::Instance()->Start();

    if (0 != gcStarted)
    {
        return returnValue;
    }

    SegmentId victimId = AllocatorSingleton::Instance()->
                            GetMostInvalidatedSegment();

    if (UNMAP_SEGMENT == victimId)
    {
        return returnValue;
    }

    returnValue = GarbageCollectorSingleton::Instance()->DisableThreshold();

    return returnValue;
}
int
WbtCmdHandler::_SetGcThreshold(Args argv)
{
    int returnValue = -1;
    int isGcPossible = GarbageCollectorSingleton::Instance()->IsGcPossible();
    if (0 != isGcPossible)
    {
        return returnValue;
    }

    if (!argv.contains("normal") || !argv.contains("urgent"))
    {
        return returnValue;
    }

    std::string gcThreshold = argv["normal"].get<std::string>();
    std::string urgentThreshold = argv["urgent"].get<std::string>();

    if (gcThreshold.at(0) == '-' || urgentThreshold.at(0) == '-')
    {
        return returnValue;
    }

    try
    {
        uint32_t numGcThreshold = std::stoul(gcThreshold);
        uint32_t numUrgentThreshold = std::stoul(urgentThreshold);

        if (numGcThreshold <= numUrgentThreshold)
        {
            return returnValue;
        }

        Allocator* allocator = AllocatorSingleton::Instance();
        allocator->SetGcThreshold(numGcThreshold);
        allocator->SetUrgentThreshold(numUrgentThreshold);

        uint32_t freeSegments = allocator->GetNumOfFreeUserDataSegment();
        if (allocator->GetUrgentThreshold() < freeSegments)
        {
            allocator->SetBlockingSegmentAllocation(false);
        }

        returnValue = 0;
    }
    catch (const std::exception& e)
    {
        IBOF_TRACE_ERROR(returnValue, e.what());
    }

    return returnValue;
}

int
WbtCmdHandler::_GetGcThreshold(JsonElement& elem)
{
    int isGcPossible = GarbageCollectorSingleton::Instance()->IsGcPossible();
    if (0 != isGcPossible)
    {
        return -1;
    }

    uint32_t numGcThreshold = AllocatorSingleton::Instance()->GetGcThreshold();
    uint32_t numUrgentThreshold =
        AllocatorSingleton::Instance()->GetUrgentThreshold();

    JsonElement thresholdElem("gc_threshold");
    thresholdElem.SetAttribute(JsonAttribute("normal", numGcThreshold));
    thresholdElem.SetAttribute(JsonAttribute("urgent", numUrgentThreshold));

    elem.SetElement(thresholdElem);

    return 0;
}

int
WbtCmdHandler::_GetGcStatus(JsonElement& elem)
{
    int isGcPossible = GarbageCollectorSingleton::Instance()->IsGcPossible();
    if (0 != isGcPossible)
    {
        return -1;
    }

    bool gcRunning = GarbageCollectorSingleton::Instance()->GetGcRunning();
    uint32_t freeSegments =
        AllocatorSingleton::Instance()->GetNumOfFreeUserDataSegment();
    uint32_t numGcThreshold = AllocatorSingleton::Instance()->GetGcThreshold();
    uint32_t numUrgentThreshold =
        AllocatorSingleton::Instance()->GetUrgentThreshold();

    std::string gcActive;
    std::string gcMode;

    gcActive = (false == gcRunning) ? "done" : "active";

    if (freeSegments <= numUrgentThreshold)
    {
        gcMode = "urgent";
    }
    else if (freeSegments < numGcThreshold)
    {
        gcMode = "normal";
    }
    else
    {
        gcMode = "none";
    }

    JsonElement gcElem("gc");

    JsonElement statusElem("status");
    statusElem.SetAttribute(JsonAttribute("active", "\"" + gcActive + "\""));
    statusElem.SetAttribute(JsonAttribute("mode", "\"" + gcMode + "\""));
    gcElem.SetElement(statusElem);

    JsonElement timeElem("time");

    struct timeval startTv =
        GarbageCollectorSingleton::Instance()->GetStartTime();

    std::ostringstream oss;

    if (startTv.tv_sec == 0 && startTv.tv_usec == 0)
    {
        oss << "N/A";
    }
    else
    {
        std::tm tm;
        localtime_r(&startTv.tv_sec, &tm);

        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    }

    timeElem.SetAttribute(JsonAttribute("start", "\"" + oss.str() + "\""));

    oss.str("");
    oss.clear();
    uint32_t elapsed = 0;

    if (false == gcRunning)
    {
        struct timeval endTv =
            GarbageCollectorSingleton::Instance()->GetEndTime();
        if (endTv.tv_sec == 0 && endTv.tv_usec == 0)
        {
            oss << "N/A";
        }
        else
        {
            std::tm tm;
            localtime_r(&endTv.tv_sec, &tm);
            oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        }

        elapsed = ((endTv.tv_sec - startTv.tv_sec) * 1000) + ((endTv.tv_usec - startTv.tv_usec) / 1000);
    }

    timeElem.SetAttribute(JsonAttribute("end", "\"" + oss.str() + "\""));
    timeElem.SetAttribute(JsonAttribute("elapsed", elapsed));

    gcElem.SetElement(timeElem);

    const PartitionLogicalSize* udSize;
    udSize = ArraySingleton::Instance()->GetSizeInfo(PartitionType::USER_DATA);
    uint32_t totalSegments = udSize->totalSegments;

    JsonElement segElem("segment");

    segElem.SetAttribute(JsonAttribute("total", totalSegments));
    segElem.SetAttribute(JsonAttribute("used", totalSegments - freeSegments));
    segElem.SetAttribute(JsonAttribute("free", freeSegments));
    gcElem.SetElement(segElem);

    elem.SetElement(gcElem);

    return 0;
}

} // namespace ibofos
