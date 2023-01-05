import gdb
import sys
import os

current_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(1, current_path)
sys.path.insert(1, current_path + "/../")
import core_dump_lib
import gdb_lib


def show_metafs_mbr(metafsPtr):
    print("- mbr info")
    requestString = 'p ((pos::MetaFs*)' + metafsPtr + ').mgmt.sysMgr.mbrMgr.mbr.content'
    output = gdb.execute(requestString + '.isNPOR', to_string=True)
    output = output.strip(',\n ')
    print("isNPOR: " + output.split('=')[1].strip())

    output = gdb.execute(requestString + '.mbrSignature', to_string=True)
    output = output.strip(',\n ')
    print("mbrSignature: " + output.split('=')[1].strip())

    output = gdb.execute(requestString + '.mfsEpochSignature', to_string=True)
    output = output.strip(',\n ')
    print("mfsEpochSignature: " + output.split('=')[1].strip())

    output = gdb.execute(requestString + '.geometry.volumeInfo.totalFilesystemVolumeCnt', to_string=True)
    output = output.strip(',\n ')
    partitionCnt = int(output.strip(',\n ').split('=')[1].strip())

    for idx in range(partitionCnt):
        result = ""
        output = gdb.execute(requestString + '.geometry.mediaPartitionInfo._M_elems[' + str(idx) + ']', to_string=True)
        output = output.strip(',\n ').split('\n')
        output = [i.split('=') for i in output]
        for i in range(1, len(output) - 1):
            result += output[i][0].strip() + ": " + output[i][1].strip(", ") + " "
        print(result)

def print_metafs_config(configName):
    config = gdb.execute('p pos::singletonInfo.metaFsService.configManager.' + configName, to_string=True)
    config = config.split('=')
    print(configName + ": " + config[1].strip())


def show_metafs_config():
    print_metafs_config("mioPoolCapacity_")
    print_metafs_config("mpioPoolCapacity_")
    print_metafs_config("writeMpioEnabled_")
    print_metafs_config("writeMpioCapacity_")
    print_metafs_config("directAccessEnabled_")
    print_metafs_config("timeIntervalInMillisecondsForMetric_")


def get_metafs_ptr_list():
    metafsList = gdb.execute('p pos::singletonInfo->metaFsService->fileSystems', to_string=True)
    metafsList = metafsList.split('\n')
    metafsPtrList = []

    # array
    for item in metafsList:
        if "_M_elems =" in item:
            first = item.split('=')[1].strip(',\n {}')
            if len(first) > 0:
                count = 0
                addrList = first.split(',')
                for addr in addrList:
                    metafsPtrList.append(addr.strip(',\n '))
                    count += 1
    return metafsPtrList


def show_array_list():
    arrayList = gdb.execute('p pos::singletonInfo->metaFsService->arrayNameToId', to_string=True)
    arrayList = arrayList.split('\n')
    count = 0
    # unordered_map
    for item in arrayList:
        if "[" in item:
            first = item.split('=')[0].strip()[1:][:-1]
            second = item.split('=')[1].strip(',\n ')
            print(str(count) + ": arrayName: " + first + ", arrayId: " + second)
            count += 1


def show_inode_info(inodePtr):
    fieldList = gdb.execute('p ((pos::MetaFileInode*)' + inodePtr + ').data.basic.field', to_string=True)
    fieldList = fieldList.split('\n')
    fd = 0
    fileName = ""
    for line in fieldList:
        if "fd =" in line:
            fd = line.split('=')[1].strip(',\n ')
        elif "_M_elems =" in line:
            fileName = line.split('=')[1].strip(',\n ').split(',')[0]
    print("fd: " + str(fd) + ", fileName: " + fileName)


def show_volume_info(volumePtr):
    volumeType = gdb.execute('p ((pos::MetaVolume*)' + volumePtr + ').volumeType', to_string=True)
    volumeType = volumeType.split('=')[1].strip(',\n ')
    print("- volume ptr: " + volumePtr + ", volumeType: " + volumeType)
    inodePtrList = gdb.execute('p ((pos::MetaVolume*)' + volumePtr + ').inodeMgr.fd2InodeMap', to_string=True)
    inodePtrList = inodePtrList.split('\n')
    for line in inodePtrList:
        if "[" in line:
            inodeAddr = line.split('=')[1].strip(',\n ')
            show_inode_info(inodeAddr)
    print("- extent allocator")
    output = gdb.execute('p ((pos::MetaVolume*)' + volumePtr + ').inodeMgr.extentAllocator.freeList', to_string=True)
    output = output.split('\n')
    print("free list")
    extentStr = ""
    for item in output:
        if "startLpn =" in item:
            extentStr = "startLpn: " + item.strip(", ").split(" = ")[1]
        elif "count =" in item:
            extentStr += ", count: " + item.strip(", ").split(" = ")[1]
    print(extentStr)


def show_metafs_io_info(metafsPtr):
    print("- meta io manager")
    output = gdb.execute('p ((pos::MetaFs*)' + metafsPtr + ').io.ioMgr.mioHandlerCount', to_string=True)
    output = output.split('=')
    print("meta thread count: " + output[1].strip())


def get_metafs_info_str(metafsPtr):
    requestStr = "p ((pos::MetaFs*)" + metafsPtr + ")"
    arrayId = gdb.execute(requestStr + '.arrayId_', to_string=True)
    arrayId = arrayId.split('=')[1].strip(',\n ')
    arrayName = gdb.execute(requestStr + '.arrayName_._M_dataplus._M_p', to_string=True)
    arrayName = arrayName.split('\"')[1].strip(',\n ')
    result = "arrayId: " + arrayId + ", arrayName: " + arrayName
    return result


def show_status():
    print("##### metafs information #####")
    print("\n# metafs config")
    show_metafs_config()

    print("\n# metafs ptr list (metaFsService->fileSystems)")
    metafsPtrList = get_metafs_ptr_list()
    count = 0
    for metafsPtr in metafsPtrList:
        if metafsPtr != "0x0":
            print(str(count) + ": " + metafsPtr)
            count += 1

    count = 0
    for metafsPtr in metafsPtrList:
        if metafsPtr != "0x0":
            arrayStr = get_metafs_info_str(metafsPtr)
            print("\n# metafs ptr " + str(count) + ": " + metafsPtr + ", " + arrayStr)
            show_metafs_io_info(metafsPtr)
            show_metafs_mbr(metafsPtr)
            count += 1
            volumeContainerList = gdb.execute('p ((pos::MetaFs*)' + metafsPtr + ').ctrl.volMgr.volContainer.volumeContainer', to_string=True)
            volumeContainerList = volumeContainerList.split('\n')
            for metaVolume in volumeContainerList:
                if "get() =" in metaVolume:
                    volumeAddr = metaVolume.split('=')
                    volumeAddr = volumeAddr[1].strip(',\n {}')
                    show_volume_info(volumeAddr)
            print("- if you want to see ctrl.cxtList: " + "p ((pos::MetaFs*)" + metafsPtr + ").ctrl.cxtList")
