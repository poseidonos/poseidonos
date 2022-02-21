#!/usr/bin/env python3

import subprocess
import sys

import TEST
import TEST_LOG
import TEST_LIB

#######################################################################################
q_depth = 4
block_size = '4K'
core_mask = '0x40'
#######################################################################################


def write(arrayId, volumeId, offset, size, pattern, runTime=0):
    TEST_LOG.print_info("* Write data: array" + str(arrayId) + ", vol" + str(volumeId) + ", offset " + str(offset) +
                        ", size " + str(size) + ", pattern " + str(pattern) + ", run_time " + str(runTime))
    ret = run_fio("write", arrayId, volumeId, offset, size, pattern, TEST.traddr, TEST.port, "tcp", runTime)
    if ret != 0:
        TEST_LOG.print_err("* Write Failed (array{}, vol{})".format(arrayId, volumeId))
        sys.exit(1)
    else:
        TEST_LOG.print_info("* Write Success (array{}, vol{})".format(arrayId, volumeId))


def verify(arrayId, volumeId, offset, size, pattern):
    TEST_LOG.print_info("* Verify data: array" + str(arrayId) + ", vol" + str(volumeId) + ", offset " + str(offset) +
                        ", size " + str(size) + ", pattern " + str(pattern))
    ret = run_fio("read", arrayId, volumeId, offset, size, pattern, TEST.traddr, TEST.port, "tcp")
    if ret != 0:
        TEST_LOG.print_err("* Verify Failed (array{}, vol{})".format(arrayId, volumeId))
        sys.exit(1)
    else:
        TEST_LOG.print_info("* Verify Success (array{}, vol{})".format(arrayId, volumeId))


# TODO (huijeong.kim) to update test cases with this verify
def verify_with_two_patterns(arrayId, volumeId, offset, size, pattern1, pattern2):
    TEST_LOG.print_info("* Verify data: array " + str(arrayId) + " vol " + str(volumeId) + ", offset " + str(offset) +
                        ", size " + str(size) + ", pattern " + str(pattern1) + " or " + str(pattern2))

    current_offset = offset
    while current_offset < offset + TEST_LIB.parse_size(size):
        verify_block(volumeId, current_offset, "4k", pattern1, pattern2)
        current_offset += 4096


def verify_block(arrayId, volumeId, offset, size, pattern1, pattern2):
    ret = run_fio("read", volumeId, offset, size, pattern1, TEST.traddr, TEST.port, "tcp")
    if ret != 0:
        ret = run_fio("read", volumeId, offset, size, pattern2, TEST.traddr, TEST.port, "tcp")
        if ret != 0:
            TEST_LOG.print_err("* Verify Failed (array{}, vol{})".format(arrayId, volumeId))
            sys.exit(1)
        else:
            TEST_LOG.print_info("* Verify Success with pattern {} at offset {} (vol {})".format(pattern2, offset, volumeId))
            return 0
    else:
        TEST_LOG.print_info("* Verify Success with pattern {} at offset {} (vol {})".format(pattern1, offset, volumeId))
        return 0


def get_fio_resultfile():
    if TEST.print_fio_result_to_log_file:
        out = open(TEST.output_log_path, "a")
        return out
    else:
        return 0


def run_fio(workload, arrayId, volumeId, io_offset, io_size_bytes, verify_pattern, ip, port, transport="tcp", runTime=0):
    print(workload)
    logger = get_fio_resultfile()

    subsystemId = TEST_LIB.get_subsystem_id(arrayId, volumeId)
    filename = "trtype=" + transport + " adrfam=IPv4" + " traddr=" + ip + " trsvcid=" + str(port) + " subnqn=nqn.2019-04.pos\\:subsystem" + str(subsystemId) + " ns=1"

    command = "fio --ioengine=" + TEST.ioengine
    command += " --runtime=" + str(runTime) + ""
    command += " --io_size=" + str(io_size_bytes) + ""
    command += " --bs=" + block_size + ""
    command += " --iodepth=" + str(q_depth) + ""
    command += " --readwrite=" + workload
    command += " --offset=" + str(io_offset)
    command += " --bs_unaligned=1"
    command += " --norandommap=1"

    if verify_pattern != 0:
        command += " --verify=pattern --verify_pattern=" + verify_pattern
    else:
        command += " --verify=0"

    if runTime != 0:
        command += " --time_based=" + str(1) + ""
    else:
        command += " --time_based=" + str(0) + ""

    command += " --ramp_time=0"
    command += " --numjobs=1"
    command += " --thread=1 --group_reporting=1 --direct=1 "
    command += " --name=test --filename='" + filename + "'"

    msg = "io={}, size={}, pattern={}".format(workload, io_size_bytes, verify_pattern)
    TEST_LOG.print_info("[FIO-array{},vol{}] Started. {}".format(arrayId, volumeId, msg), color="")
    TEST_LOG.print_info(command, color="")

    if TEST.skip_fio_run:
        return 0
    else:
        if logger != 0:
            ret = subprocess.call(command, shell=True, stdout=logger, stderr=logger)
        else:
            ret = subprocess.call(command, shell=True)

        if ret != 0:
            TEST_LOG.print_info("[FIO-array{},vol{}] Terminated".format(arrayId, volumeId))
        else:
            TEST_LOG.print_info("[FIO-array{},vol{}] Success.".format(arrayId, volumeId), color="")

        return ret
