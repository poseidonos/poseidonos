#!/usr/bin/env python3

import subprocess
from subprocess import check_call, call, check_output, Popen, PIPE
import random
import os
import sys
import re
import signal
import getopt
from datetime import datetime
from itertools import *
import csv
import itertools
from shutil import copyfile
import json
import sys
import time
import struct
import common_test_lib
import argparse
import psutil
import io

from collections import defaultdict

#subprcess.call ('ls -al', shell=True)
FIO_BENCH_SCRIPT = './fio_bench.py'
CLI_CMD = '../../../bin/poseidonos-cli'
POS_ROOT = '../../../'
IBOFOS_CMD = '../../../bin/poseidonos'
SETUP_IBOFOS_PATH = '../../../script/'
SETUP_IBOFOS_CMD = './setup_ibofos_nvmf_volume.sh'
SIZE = 2147483648
NVME_CLI_CMD = 'nvme'
LOG_PATH = 'pos.log'
DEFAULT_TR_ADDR = "10.100.11.16"
TR_TYPE = 'tcp'
TR_PORT = '1158'
IO_ENGINE = 'aio'
IO_DEPTH = '128'
SECTOR_SIZE = 512
TEST_INTERNAL_ERR = -1
NQN = 'nqn.2019-04.pos:subsystem1'
stopOnError = False
result = dict()
retry_limit = 4

# sec can be floating point type.


def Wait(sec):
    print("Wait " + str(sec) + " seconds....")
    time.sleep(sec)


def SeqFill():
    print("######## Sequential Fill.. ##########")
    command = BASIC_FIO_SCRIPT + ' --io_size=' + str(SIZE) + '  --readwrite="write" --bs=128k'
    ret = subprocess.call(command, shell=True)
    if (ret != 0):
        return ret
    print("##### Sequential Fill Complete ######")
    return ret


def ReadWriteCommand(lbaList, sizeList, cmd, verify, addedCmd=""):
    # cmd should be string, read / write / randread / randwrite
    count = 0
    for lba in lbaList:
        size = sizeList[count]
        print("######## " + cmd + " .. LBA : " + str(lba) +
              " Size : " + str(size) + " ##########")
        command = BASIC_FIO_SCRIPT + ' --offset=' + str(lba) + \
            ' --io_size=' + str(size) + ' --bs=4k --readwrite=' + cmd
        if(verify == True):
            command += " --verify=Pattern --verify_pattern=0x1B0FABCD"
        command += " "
        command += addedCmd
        ret = subprocess.call(command, shell=True)
        if (ret != 0):
            print("##### " + cmd + " Failed!!  ######")
            return ret
        count = count + 1
    print("##### " + cmd + " Complete ######")
    return ret


def PowerOff():
    stdout_type = subprocess.DEVNULL
    common_test_lib.terminate_pos(POS_ROOT, stdout_type)

    print("########## disconnect nvmf controllers ##########")
    command = NVME_CLI_CMD + ' disconnect -n ' + NQN

    ret = subprocess.call(command, shell=True)
    if (ret != 0):
        print("##### Disconnect Failed!!  ######")
        return ret

    print("########### Power off Completed #################")
    return 0


def PowerOn(clean):
    print("########## Power On PoseidonOS.. ##########")
    stdout_type = subprocess.DEVNULL

    bringup_argument = {
        'log_path': LOG_PATH,
        'pos_root': POS_ROOT,
        'transport': TR_TYPE,
        'target_ip': args.fabric_ip,
        'volume_size': SIZE,
        'stdout_type': stdout_type,
        'clean': clean}

    common_test_lib.bringup_pos(**bringup_argument)

    Wait(3)
    command = NVME_CLI_CMD + " connect " + " -t " + TR_TYPE + " -a " +\
        args.fabric_ip + " -s " + TR_PORT + " -n " + NQN
    print(command)
    ret = subprocess.call(command, shell=True)
    if (ret != 0):
        print("##### Connection Failed!!  ######")
        # this error is critical, exit this process.
        exit(1)

    command = "nvme list | grep POS | awk '{print $1; exit}'"
    global dev_name
    retry_count = 0
    dev_name = ""
    while dev_name == "":
        byte_string_dev_name = subprocess.check_output(command, shell=True)
        dev_name = "".join(map(chr, byte_string_dev_name))
        dev_name = dev_name[:-1]
        retry_count = retry_count + 1
        if retry_count > retry_limit:
            print("##### Connection Failed!!  ######")
            exit(1)

    tempfile = "temp.output"
    command = NVME_CLI_CMD + " id-ctrl " + dev_name + " | grep 'subnqn' > " + tempfile
    print(command)
    ret = subprocess.call(command, shell=True)
    if(ret != 0):
        print("##### Connection Failed!!  ######")
        # this error is critical, exit this process.
        exit(1)
    find_success = False
    f = open(tempfile)
    for line in f:
        if (NQN in line):
            find_success = True
    if(find_success == False):
        print("##### Connection Failed!!  ######")
        # this error is critical, exit this process.
        exit(1)

    global BASIC_FIO_SCRIPT
    BASIC_FIO_SCRIPT = FIO_BENCH_SCRIPT + ' --iodepth=' + IO_DEPTH +\
        ' --file_num=1 --filename=' + dev_name + ' --ioengine=' + IO_ENGINE

    return 0

# Input will be scatter gather list


def RandOffsetAndSize(startOffset, endOffset, testN, overlapAllowed=True):
    offsetVector = []
    sizeVector = []

    for i in range(0, testN):
        overlapFlag = True
        while overlapFlag == True:
            a = random.randrange(startOffset / SECTOR_SIZE, endOffset / SECTOR_SIZE)
            b = random.randrange(1, 128 * 1024 / SECTOR_SIZE)
            a = a * SECTOR_SIZE
            b = b * SECTOR_SIZE
            overlapFlag = False
            if (overlapAllowed == True):
                count = 0
                for offset in offsetVector:
                    size = sizeVector[count]
                    # overlap check
                    if(a <= offset + size and offset <= a + b):
                        overlapFlag = True
                        break

        offsetVector.append(a)
        sizeVector.append(b)

    return [offsetVector, sizeVector]


def ClearEnv():
    # in this function, do not check error.
    for proc in psutil.process_iter():
        try:
            if "fio" in proc.name():
                proc.kill()
                proc.wait()
        except psutil.NoSuchProcess:
            pass
    common_test_lib.terminate_pos(POS_ROOT, subprocess.DEVNULL)

    command = NVME_CLI_CMD + ' disconnect -n ' + NQN
    ret = subprocess.call(command, shell=True)


def Flush():
    command = "nvme flush " + dev_name + " -n 1"
    ret = subprocess.call(command, shell=True)
    if(ret != 0):
        print("##### Flush Failed! #####")
        return ret
    return 0


def WriteZero(lbaList, sizeList, testN):
    count = 0
    for lba in lbaList:
        size = sizeList[count]

        command = "nvme write-zeroes " + dev_name + " -n 1 -s " + \
            str(int(lba / SECTOR_SIZE)) +\
            " -c " + str(int(size / SECTOR_SIZE) - 1)
        print(command)
        ret = subprocess.call(command, shell=True)
        if(ret != 0):
            print("##### Write Zero Failed! #####")
            return ret
        count = count + 1
    return 0


def Unmap(lbaList, sizeList, testN):
    lbaString = ""
    for lba in lbaList:
        lbaString += str(int(lba / SECTOR_SIZE))
        lbaString += ","
    lbaString = lbaString.rstrip(",")
    sizeString = ""
    for size in sizeList:
        sizeString += str(int(size / SECTOR_SIZE))
        sizeString += ","
    sizeString = sizeString.rstrip(",")

    command = "nvme dsm " + dev_name + " -n 1 -d -s " + lbaString +\
              " -b " + sizeString
    print(command)
    ret = subprocess.call(command, shell=True)
    if(ret != 0):
        print("##### Unmap Failed! #####")
        return ret
    return 0


def IsZero(lbaList, sizeList, testN):

    count = 0
    for lba in lbaList:
        size = sizeList[count]
        command = "nvme read " + dev_name + " -s " + str(int(lba / SECTOR_SIZE)) \
            + " -c " + str(int(size / SECTOR_SIZE) - 1) + " -z " + str(size)\
            + " > read.bin"
        print(str(command))
        ret = subprocess.call(command, shell=True)
        if(ret != 0):
            print("##### Read Failed! #####")
            return ret
        f = open("read.bin", "rb")
        for i in range(0, size):
            byte = f.read(1)
            num = int.from_bytes(byte, "little")
            if(num != 0):
                print("####### Zero read failed LBA : " + str(f.tell() - 1) + " " + str(byte) + " #######")
                f.close()
                return -1
        f.close()
        count = count + 1
    return 0


def TestResult():
    global result
    print("")
    print("##### TEST Result ######")
    success = True
    for test in result:
        print(test, " : ", result[test])
        if result[test] == 'False':
            success &= False
    print("")
    return success


def TestSuccess(st):
    print("")
    print("##### TEST Success : " + st + " ######")
    print("")


def TestFailedCheck(st, ret):
    if(ret != 0):
        print("")
        print("##### TEST Failed : " + st + " ErrorCode : " + str(ret) + " ######")
        print("")
        if(stopOnError):
            TestResult()
            exit(ret)


def Test_FLUSH():
    SeqFill()
    TESTN = 2
    retVector = RandOffsetAndSize(0, SIZE, TESTN, False)

    ret = ReadWriteCommand(retVector[0], retVector[1], 'write', False)
    if (ret != 0):
        return ret
    ret = Flush()
    if (ret != 0):
        return ret
    ret = PowerOff()
    if (ret != 0):
        return ret
    ret = PowerOn(0)
    if (ret != 0):
        return ret
    ret = ReadWriteCommand(retVector[0], retVector[1], 'read', False)
    return ret


def Test_RND_WRITE_UNMAP():
    SeqFill()
    TESTN = 2
    retVector = RandOffsetAndSize(0, SIZE, TESTN)
    lbaVector = retVector[0]
    sizeVector = retVector[1]
    ret = Unmap(lbaVector, sizeVector, TESTN)
    if (ret != 0):
        return ret
    ret = IsZero(lbaVector, sizeVector, TESTN)
    return ret


def Test_SEQ_WRITE_UNMAP():
    SeqFill()
    TESTN = 1
    lbaVector = [0]
    sizeVector = [1024 * 128]
    ret = Unmap(lbaVector, sizeVector, TESTN)
    if (ret != 0):
        return ret
    ret = IsZero(lbaVector, sizeVector, TESTN)
    return ret


def Test_UNMAP_NPOR():
    SeqFill()
    TESTN = 2
    retVector = RandOffsetAndSize(0, 128 * 1024, TESTN)
    lbaVector = retVector[0]
    sizeVector = retVector[1]
    ret = Unmap(lbaVector, sizeVector, TESTN)
    if (ret != 0):
        return ret

    ret = PowerOff()
    if (ret != 0):
        return ret
    ret = PowerOn(0)
    if (ret != 0):
        return ret
    ret = IsZero(lbaVector, sizeVector, TESTN)
    return ret


def Test_SEQ_WRITE_ZERO():
    SeqFill()
    TESTN = 1
    lbaVector = [0]
    sizeVector = [1024 * 128]
    ret = WriteZero(lbaVector, sizeVector, TESTN)
    if (ret != 0):
        return ret

    ret = IsZero(lbaVector, sizeVector, TESTN)
    return ret


def Test_RND_WRITE_ZERO():
    SeqFill()
    TESTN = 2

    retVector = RandOffsetAndSize(0, SIZE, TESTN)
    lbaVector = retVector[0]
    sizeVector = retVector[1]

    ret = WriteZero(lbaVector, sizeVector, TESTN)
    if (ret != 0):
        return ret

    ret = IsZero(lbaVector, sizeVector, TESTN)
    return ret


def Test_WRITE_ZERO_NPOR():
    SeqFill()
    TESTN = 2
    retVector = RandOffsetAndSize(0, 128 * 1024, TESTN)
    lbaVector = retVector[0]
    sizeVector = retVector[1]
    ret = WriteZero(lbaVector, sizeVector, TESTN)

    if (ret != 0):
        return ret

    ret = PowerOff()
    if (ret != 0):
        return ret
    ret = PowerOn(0)
    if (ret != 0):
        return ret
    ret = IsZero(lbaVector, sizeVector, TESTN)

    return ret


def parse_arguments():
    parser = argparse.ArgumentParser(
            description='Please add fio option to nvmf_etc_command.py')
    parser.add_argument('-t', '--test',
                        help='Specify Test Name (FLUSH, SEQ_WRITE_UNMAP, RND_WRITE_UNMAP, \
            UNMAP_NPOR, SEQ_WRITE_ZERO, RND_WRITE_ZERO, WRITE_ZERO_NPOR')
    parser.add_argument('-s', '--stop_on_error', action='store_true',
                        help='If Error Failure happens, Stop and do not clear poseidonos')
    parser.add_argument('-f', '--fabric_ip', default=DEFAULT_TR_ADDR,
                        help='Set Fabric IP, default: ' + DEFAULT_TR_ADDR)
    global args
    args = parser.parse_args()
    print(args)


functionTable = {
    "FLUSH": Test_FLUSH,
    "SEQ_WRITE_UNMAP": Test_SEQ_WRITE_UNMAP,
    "RND_WRITE_UNMAP": Test_RND_WRITE_UNMAP,
    "UNMAP_NPOR": Test_UNMAP_NPOR,
    "SEQ_WRITE_ZERO": Test_SEQ_WRITE_ZERO,
    "RND_WRITE_ZERO": Test_RND_WRITE_ZERO,
    "WRITE_ZERO_NPOR": Test_WRITE_ZERO_NPOR,
}


def main():
    global stopOnError
    global result
    parse_arguments()

    if (args.stop_on_error):
        print("stop on error set")
        stopOnError = True

    if (args.test != None):
        if (args.test in functionTable):
            ClearEnv()
            ret = PowerOn(1)
            TestFailedCheck("Power On Failed.. ", ret)

            ret = functionTable[args.test]()
            if (ret == 0):
                TestSuccess(args.test)
            else:
                TestFailedCheck(args.test, ret)
            ClearEnv()
        else:
            print("Specify Test Name after -t option (FLUSH, SEQ_WRITE_UNMAP, RND_WRITE_UNMAP, \
                 UNMAP_NPOR, SEQ_WRITE_ZERO, RND_WRITE_ZERO, WRITE_ZERO_NPOR")
    else:
        ClearEnv()

        for test in functionTable:
            # TODO: Enable UNMAP related TC after functionality is implemented
            if "UNMAP" in test:
                print("Skip " + test)
                continue

            ret = PowerOn(1)
            ret = functionTable[test]()
            if (ret == 0):
                result[test] = 'Success'
                TestSuccess(test)
            else:
                result[test] = 'Fail'
                TestFailedCheck(test, ret)
            ClearEnv()

        success = TestResult()
        ClearEnv()
        if success:
            exit(0)
        else:
            exit(-1)


if __name__ == "__main__":
    main()
