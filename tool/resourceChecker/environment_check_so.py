#!/usr/bin/env python3
import os
import subprocess
from ctypes import *


def CheckRaidSupport(numCntNvmeList):
    print("6. Check RAID configuration")
    print("-" * 30)
    print("RAID Type".ljust(15, " "), end='')
    print("|  Support")
    print("-" * 30)

    print("RAID0 ".ljust(15, " "), end='' + "|")
    if numCntNvmeList >= 2:
        print("\033[96m  Available\033[0m")
    else:
        print("\033[95m  Non-Available  \033[0m")

    print("RAID10 ".ljust(15, " "), end='' + "|")
    if numCntNvmeList >= 2:
        print("\033[96m  Available\033[0m")
    else:
        print("\033[95m  Non-Available\033[0m")

    print("RAID5 ".ljust(15, " "), end='' + "|")
    if numCntNvmeList >= 3:
        print("\033[96m  Available\033[0m")
    else:
        print("\033[95m  Non-Available\033[0m")
    print("-" * 30)

if __name__ == "__main__":
  
    os.system("make_environment_checker_so_file.sh")
    rootDir = os.path.dirname(os.path.abspath(__file__))
    fileName = "/environment_checker.so"
    cmd = rootDir + fileName
    api = cdll.LoadLibrary(cmd)

    # 1. Host os version
    result = api.CheckSupportedOsVersion()
    print("#1")
    if result is False:
        print("1. OS : \033[94m Mismatched with host OS version"'\033[0m')
        exit(0)
    else:
        print("1. OS : \033[94m Matched with host OS version"'\033[0m')

    result = api.CheckSupportedKernelVersion()
    if result is False:
        print("2. Kernel : \033[94m Mismatched with host Kernel version"'\033[0m')
        exit(0)
    else:
        print("2. Kernel : \033[94m Matched with host Kernel version"'\033[0m')

    # 2. number of cpu
    cpuNum = api.GetHostCpuNum()
    print("3-1 number of cpu :", '\033[94m' '%d' '\033[0m' % (cpuNum))

    cpuClock = api.GetHostCpuClock()
    print("3-2 clock of cpu :", '\033[94m' '%d' " MHz"'\033[0m ' % (cpuClock))

    # 3. Memory size
    memSize = api.GetTotalMemorySizeInfo()
    print("4-1 Total mem info :", '\033[94m' '%d' " MB"'\033[0m' % (memSize / 1024))

    memSize = api.GetAvailableMemorySizeInfo()
    print("4-2 available mem info :", '\033[94m' '%d' " MB"'\033[0m' % (memSize / 1024))

    # 4. PCIe list
    numPCIeDevice = api.PcieCountInfo()
    print("5. number of pcie device :", '\033[94m' '%d' '\033[0m' % (numPCIeDevice))

    # 5. Check RAID configuration
    CheckRaidSupport(numPCIeDevice)

    isMeetMinCpuNum  = api.IsMeetMinimumCore()
    if isMeetMinCpuNum is False:
        print("!! Need more CPU")
        exit(0)
    else:
        print("Meet minimum cpu numebr")

    isMeetMinMemorySize = api.IsMeetMinimumMemorySize()
    if isMeetMinMemorySize is False :
        print("!! Need more memory")
        exit(0)
    else:
        print("Meet minimum memory")

    # 6. Check existed file

    isMExsitedFile = api.CheckRulesdFile()
    if isMExsitedFile is False :
        print("No file : 99-custom-nvme.rules")
        exit(0)
    else:
        print("99-custom-nvme.rules File existed")

exit(0)
