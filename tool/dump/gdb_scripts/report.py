import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")

import gdb_lib
import core_dump_lib


debug_info_str = "pos::singletonInfo"
arrayManager_str = "arrayManager"
arrayList_str = "arrayList"
arrayNodeCount_str = "._M_t._M_impl._M_node_count"

allocatorService_str = "allocatorService"
iWBStripeAllocator_str = "iWBStripeAllocator"
ioSubmitHandlerCount_str = "ioSubmitHandlerCount"

wbStripeMgr_str = "pos::WBStripeManager"


def make_report():
    gdb.execute("set pagination off")
    gdb.execute("set logging file initial_report.txt")
    # segment fault occurred with below line
    gdb.execute("set logging on")

    cmd = "p " + "*" + debug_info_str
    gdb.execute(cmd)

    print("================ Array Infomation ================")
    # Array Manager
    cmd = gdb_lib.advance_ptr(debug_info_str, arrayManager_str)
    cmd = gdb_lib.advance_ptr(cmd, arrayList_str)
    cmd = cmd + arrayNodeCount_str
    num_of_array_list = gdb.parse_and_eval(cmd)

    cmd = gdb_lib.advance_ptr(debug_info_str, arrayManager_str)
    array_manager = gdb.parse_and_eval(cmd)
    print("number of array list : %s" % (str(num_of_array_list)))
    print("array_manager : %s" % (str(array_manager)))

    temp = int(num_of_array_list)
    for array_index in range(temp):
        # WBStripeManager
        cmd = gdb_lib.advance_ptr(debug_info_str, allocatorService_str)
        cmd = gdb_lib.advance_ptr(cmd, iWBStripeAllocator_str)
        cmd = cmd + "._M_elems[" + str(array_index) + "]"
        wbStripeManagerAddr = gdb.parse_and_eval(cmd)

        print("array_index = %d, WBStripeManager addr = %s" % (array_index, str(wbStripeManagerAddr)))

        if (int(wbStripeManagerAddr) == 0x0):
            continue

        cmd = gdb_lib.make_gdb_cmd_p(wbStripeMgr_str, wbStripeManagerAddr)
        print("    cmd info: %s" % cmd)

        # array name
        # TODO: need to parse string data (arrayname) only
        print("\n================ ArrayName ================")
        cmd = "p (*(pos::WBStripeManager*)" + str(wbStripeManagerAddr) + ")" + "->arrayName"
        gdb.execute(cmd)

        print("\n================ Partition Info ================")
        # (*(pos::singletonInfo *) 0x430ef00)->volumeService->items[0]->arrayInfo->ptnMgr->partitions._M_elems[0]
        cmd = gdb_lib.advance_ptr(debug_info_str, "volumeService")
        volumeServiceAddr = gdb.parse_and_eval(cmd)
        print("    cmd info: p *%s" % cmd)

        volumeManagerAddr = \
            gdb.parse_and_eval("((pos::VolumeService *)%s)->items[%d]" % (volumeServiceAddr, array_index))

        # meta nvm partition
        ptnMgrAddr = \
            gdb.parse_and_eval("((pos::VolumeManager *)%s)->arrayInfo->ptnMgr" % (volumeManagerAddr))

        partitonAddr = gdb.parse_and_eval("((pos::PartitionManager *)%s)->partitions._M_elems[0]" % (ptnMgrAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        blksPerChunk = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerChunk" % (partitonAddr))
        blksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerStripe" % (partitonAddr))
        chunksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.chunksPerStripe" % (partitonAddr))
        stripesPerSegment = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.stripesPerSegment" % (partitonAddr))
        totalStripes = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalStripes" % (partitonAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        totalSegments = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalSegments" % (partitonAddr))

        print("\n================ META NVM partition Info ================")
        print("Logical Size info")
        print("minWriteBlkCnt:%5d, blksPerChunk:%5d, blksPerStripe:%5d" % (minWriteBlkCnt, blksPerChunk, blksPerStripe))
        print("chunksPerStripe:%5d, stripesPerSegment:%5d, totalStripes:%5d, totalSegments:%5d" % (chunksPerStripe, stripesPerSegment, totalStripes, totalSegments))

        startLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.startLba" % (partitonAddr))
        lastLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.lastLba" % (partitonAddr))

        print("\nPhysical Size info")
        print("startLba:%d, lastLba:%d" % (startLba, lastLba))

        # write buffer partition
        partitonAddr = gdb.parse_and_eval("((pos::PartitionManager *)%s)->partitions._M_elems[1]" % (ptnMgrAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        blksPerChunk = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerChunk" % (partitonAddr))
        blksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerStripe" % (partitonAddr))
        chunksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.chunksPerStripe" % (partitonAddr))
        stripesPerSegment = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.stripesPerSegment" % (partitonAddr))
        totalStripes = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalStripes" % (partitonAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        totalSegments = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalSegments" % (partitonAddr))

        print("\n================ Write Buffer partition Info ================")
        print("Logical Size info")
        print("minWriteBlkCnt:%5d, blksPerChunk:%5d, blksPerStripe:%5d" % (minWriteBlkCnt, blksPerChunk, blksPerStripe))
        print("chunksPerStripe:%5d, stripesPerSegment:%5d, totalStripes:%5d, totalSegments:%5d" % (chunksPerStripe, stripesPerSegment, totalStripes, totalSegments))

        startLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.startLba" % (partitonAddr))
        lastLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.lastLba" % (partitonAddr))

        print("\nPhysical Size info")
        print("startLba:%d, lastLba:%d" % (startLba, lastLba))

        # meta ssd partition
        partitonAddr = gdb.parse_and_eval("((pos::PartitionManager *)%s)->partitions._M_elems[2]" % (ptnMgrAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        blksPerChunk = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerChunk" % (partitonAddr))
        blksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerStripe" % (partitonAddr))
        chunksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.chunksPerStripe" % (partitonAddr))
        stripesPerSegment = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.stripesPerSegment" % (partitonAddr))
        totalStripes = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalStripes" % (partitonAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        totalSegments = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalSegments" % (partitonAddr))

        print("\n================ META SSD partition Info ================")
        print("Logical Size info")
        print("minWriteBlkCnt:%5d, blksPerChunk:%5d, blksPerStripe:%5d" % (minWriteBlkCnt, blksPerChunk, blksPerStripe))
        print("chunksPerStripe:%5d, stripesPerSegment:%5d, totalStripes:%5d, totalSegments:%5d" % (chunksPerStripe, stripesPerSegment, totalStripes, totalSegments))

        startLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.startLba" % (partitonAddr))
        lastLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.lastLba" % (partitonAddr))

        print("\nPhysical Size info")
        print("startLba:%d, lastLba:%d" % (startLba, lastLba))

        # user data partition
        partitonAddr = gdb.parse_and_eval("((pos::PartitionManager *)%s)->partitions._M_elems[2]" % (ptnMgrAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        blksPerChunk = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerChunk" % (partitonAddr))
        blksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerStripe" % (partitonAddr))
        chunksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.chunksPerStripe" % (partitonAddr))
        stripesPerSegment = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.stripesPerSegment" % (partitonAddr))
        totalStripes = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalStripes" % (partitonAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        totalSegments = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalSegments" % (partitonAddr))

        print("\n================ USER DATA partition Info ================")
        print("Logical Size info")
        print("minWriteBlkCnt:%5d, blksPerChunk:%5d, blksPerStripe:%5d" % (minWriteBlkCnt, blksPerChunk, blksPerStripe))
        print("chunksPerStripe:%5d, stripesPerSegment:%5d, totalStripes:%5d, totalSegments:%5d" % (chunksPerStripe, stripesPerSegment, totalStripes, totalSegments))

        startLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.startLba" % (partitonAddr))
        lastLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.lastLba" % (partitonAddr))

        print("\nPhysical Size info")
        print("startLba:%d, lastLba:%d" % (startLba, lastLba))

        # journal ssd partition
        partitonAddr = gdb.parse_and_eval("((pos::PartitionManager *)%s)->partitions._M_elems[1]" % (ptnMgrAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        blksPerChunk = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerChunk" % (partitonAddr))
        blksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.blksPerStripe" % (partitonAddr))
        chunksPerStripe = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.chunksPerStripe" % (partitonAddr))
        stripesPerSegment = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.stripesPerSegment" % (partitonAddr))
        totalStripes = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalStripes" % (partitonAddr))
        minWriteBlkCnt = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.minWriteBlkCnt" % (partitonAddr))
        totalSegments = gdb.parse_and_eval("((pos::NvmPartition *)%s)->logicalSize.totalSegments" % (partitonAddr))

        print("\n================ JOURNAL SSD partition Info ================")
        print("Logical Size info")
        print("minWriteBlkCnt:%5d, blksPerChunk:%5d, blksPerStripe:%5d" % (minWriteBlkCnt, blksPerChunk, blksPerStripe))
        print("chunksPerStripe:%5d, stripesPerSegment:%5d, totalStripes:%5d, totalSegments:%5d" % (chunksPerStripe, stripesPerSegment, totalStripes, totalSegments))

        startLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.startLba" % (partitonAddr))
        lastLba = gdb.parse_and_eval("((pos::NvmPartition *)%s)->physicalSize.lastLba" % (partitonAddr))

        print("\nPhysical Size info")
        print("startLba:%d, lastLba:%d" % (startLba, lastLba))

        # block allocation status
        print("\n================ block allocation status ================")

        # current ssd lsid
        currentSsdLsid = \
            gdb.parse_and_eval("((pos::WBStripeManager *)%s)->blockManager->allocCtx->currentSsdLsid" % str(wbStripeManagerAddr))
        print("\nCurrent SSD LSID : %d" % currentSsdLsid)

        blkAllocProhibited = \
            gdb.parse_and_eval("((pos::WBStripeManager *)%s)->blockManager->allocStatus->blkAllocProhibited._M_base._M_i" % str(wbStripeManagerAddr))
        print("\nBlock allocation prohibited : %s" % blkAllocProhibited)
        userBlkAllocProhibited = \
            gdb.parse_and_eval("((pos::WBStripeManager *)%s)->blockManager->allocStatus->userBlkAllocProhibited._M_base._M_i" % str(wbStripeManagerAddr))
        print("User Block allocation prohibitted : %s" % userBlkAllocProhibited)

        # pending I/Os
        print("\n================ Pending I/O ================")
        cmd = gdb_lib.advance_ptr(debug_info_str, ioSubmitHandlerCount_str)
        ioSubmitHandlerAddr = gdb.parse_and_eval(cmd)
        print("    cmd info: p *%s" % cmd)

        pendingRead = \
            gdb.parse_and_eval("((pos::IOSubmitHandlerCount *)%s)->pendingRead.pendingCount._M_i" % str(ioSubmitHandlerAddr))
        pendingWrite = \
            gdb.parse_and_eval("((pos::IOSubmitHandlerCount *)%s)->pendingWrite.pendingCount._M_i" % str(ioSubmitHandlerAddr))
        pendingByteIo = \
            gdb.parse_and_eval("((pos::IOSubmitHandlerCount *)%s)->pendingByteIo.pendingCount._M_i" % str(ioSubmitHandlerAddr))

        print("\nPending Read : %d, pending write :  %d, pending byte io : %d" % (pendingRead, pendingWrite, pendingByteIo))

        callbackNotCalledCount = \
            gdb.parse_and_eval("((pos::IOSubmitHandlerCount *)%s)->callbackNotCalledCount.pendingCount._M_i" % str(ioSubmitHandlerAddr))

        print("callbackNotCalledCount : %d" % (callbackNotCalledCount))

        # numVolumes
        print("\n================ Volume ================")
        volCnt = gdb.parse_and_eval("((pos::WBStripeManager *)%s)->volumeManager->volumes.volCnt" % str(wbStripeManagerAddr))
        print("\nNumber of total volumes : %d" % volCnt)
        for volume_index in range(volCnt):
            # VolumeStatus::Unmounted
            pendingIoCntUnmountedStatus = \
                gdb.parse_and_eval("((pos::WBStripeManager *)%s)->volumeManager->volumes.pendingIOCount[%d][%d]._M_i" % (str(wbStripeManagerAddr), volume_index, 0))

            # VolumeStatus::Mounted
            pendingIoCntMountedStatus = \
                gdb.parse_and_eval("((pos::WBStripeManager *)%s)->volumeManager->volumes.pendingIOCount[%d][%d]._M_i" % (str(wbStripeManagerAddr), volume_index, 1))

            # VolumeStatus::InitiaSyncMode
            pendingIoCntInitialSyncModeStatus = \
                gdb.parse_and_eval("((pos::WBStripeManager *)%s)->volumeManager->volumes.pendingIOCount[%d][%d]._M_i" % (str(wbStripeManagerAddr), volume_index, 2))

            if ((0 != pendingIoCntUnmountedStatus - 1) or (0 != pendingIoCntMountedStatus - 1) or (0 != pendingIoCntInitialSyncModeStatus)):
                print("\nvolume[%d] : VolumeStatus::Unmounted, pending io : %d" % (volume_index, pendingIoCntUnmountedStatus - 1))
                print("volume[%d] : VolumeStatus::Mounted, pending io : %d" % (volume_index, pendingIoCntMountedStatus - 1))
                print("volume[%d] : VolumeStatus::InitiaSyncMode, pending io : %d" % (volume_index, pendingIoCntInitialSyncModeStatus))

        # stripe load status
        print("\n================ Stripe load status ================")
        numStripesToload = \
            gdb.parse_and_eval("((pos::WBStripeManager *)%s)->stripeLoadStatus->numStripesToload._M_i" % str(wbStripeManagerAddr))
        print("\nNumber of stripes to load: %d" % numStripesToload)

        numStripesLoaded = \
            gdb.parse_and_eval("((pos::WBStripeManager *)%s)->stripeLoadStatus->numStripesLoaded._M_i" % str(wbStripeManagerAddr))
        print("Number of stripes numStripesLoaded: %d" % numStripesLoaded)

        numStripesFailed = \
            gdb.parse_and_eval("((pos::WBStripeManager *)%s)->stripeLoadStatus->numStripesFailed._M_i" % str(wbStripeManagerAddr))
        print("Number of stripes load failed: %d" % numStripesFailed)

        # ContextManager
        contextManagerAddr = gdb.parse_and_eval("((pos::WBStripeManager *)%s)->contextManager" % str(wbStripeManagerAddr))

        print("\n================ Segment Info ================")
        # free segment count
        numSegment = gdb.parse_and_eval("((pos::ContextManager *)%s)->segmentCtx->segmentList[%d].numSegments" % (contextManagerAddr, 0))
        print("\nNumber of FREE State segments: %d" % numSegment)

        numSegment = gdb.parse_and_eval("((pos::ContextManager *)%s)->segmentCtx->segmentList[%d].numSegments" % (contextManagerAddr, 1))
        print("Number of NVRAM State segments: %d" % numSegment)

        numSegment = gdb.parse_and_eval("((pos::ContextManager *)%s)->segmentCtx->segmentList[%d].numSegments" % (contextManagerAddr, 2))
        print("Number of SSD State segments: %d" % numSegment)

        numSegment = gdb.parse_and_eval("((pos::ContextManager *)%s)->segmentCtx->segmentList[%d].numSegments" % (contextManagerAddr, 3))
        print("Number of VICTIM State segments: %d" % numSegment)

        # GC
        print("\n================ Garbage Collection ================")
        # gc mode
        currentGcMode = \
            gdb.parse_and_eval("((pos::ContextManager *)%s)->gcCtx->curGcMode" % (contextManagerAddr))
        prevGcMode = \
            gdb.parse_and_eval("((pos::ContextManager *)%s)->gcCtx->prevGcMode" % (contextManagerAddr))
        print("\nCurrent GC Mode is %s, previous GC Mode was %s" % (currentGcMode, prevGcMode))

        # gc threshold
        normalGcThreshold = \
            gdb.parse_and_eval("((pos::ContextManager *)%s)->gcCtx->normalGcThreshold" % (contextManagerAddr))
        urgentGcThreshold = \
            gdb.parse_and_eval("((pos::ContextManager *)%s)->gcCtx->urgentGcThreshold" % (contextManagerAddr))
        print("\nNormal GC Threshold %s, Urgent GC Threshold %s" % (normalGcThreshold, urgentGcThreshold))

        # Mapper
        print("\n================ Mapper ================")
        cmd = gdb_lib.advance_ptr(debug_info_str, "mapperService")
        mapperServiceAddr = gdb.parse_and_eval(cmd)
        print("    cmd info: p *%s" % cmd)

        mapperAddr = \
            gdb.parse_and_eval("((pos::MapperService *)%s)->iMapFlushes._M_elems[%d]" % (mapperServiceAddr, array_index))

        # stripeMap
        stripeMapManagerAddr = \
            gdb.parse_and_eval("((pos::Mapper *)%s)->stripeMapManager" % (mapperAddr))

        numWriteIssuedCount = \
            gdb.parse_and_eval("((pos::StripeMapManager *)%s).numWriteIssuedCount._M_i" % str(stripeMapManagerAddr))
        print("Number of stripe map write issued count : %d" % numWriteIssuedCount)
        numLoadIssuedCount = \
            gdb.parse_and_eval("((pos::StripeMapManager *)%s).numLoadIssuedCount._M_i" % str(stripeMapManagerAddr))
        print("Number of stripe map load issued count : %d" % numLoadIssuedCount)

    # need to enable if set logging on
    gdb.execute("set logging off")
