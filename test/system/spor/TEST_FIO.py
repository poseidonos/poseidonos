#!/usr/bin/env python3

import subprocess
import sys

from threading import Thread

import TEST
import TEST_LOG
import TEST_LIB

# ######################################################################################
q_depth = 4
block_size = '4K'
verify_on = False
core_mask = '0x40'

extra_fio_options = " --numjobs=1 --ramp_time=0 --norandommap=1 --bs_unaligned=1 "
# ######################################################################################

def write(volumeId, offset, size, pattern, runTime=0):
    TEST_LOG.print_info("* Write data: vol " + str(volumeId) + ", offset " + str(offset) + \
                ", size " + str(size) + ", pattern " + str(pattern) + ", run_time " + str(runTime))
    test_out = open(TEST.output_log_path, "a")
    ret = run_fio("write", volumeId, size, offset, pattern, TEST.traddr, TEST.port, "tcp", runTime, logger=test_out)
    if ret != 0:
        TEST_LOG.print_err("* Write Failed (vol {})".format(volumeId))
        sys.exit(1)
    else:
        TEST_LOG.print_info("* Write Success (vol {})".format(volumeId))

def verify(volumeId, offset, size, pattern):
    TEST_LOG.print_info("* Verify data: vol " + str(volumeId) + ", offset " + str(offset) + \
                ", size " + str(size) + ", pattern " + str(pattern))
    test_out = open(TEST.output_log_path, "a")
    ret = run_fio("read", volumeId, size, offset, pattern, TEST.traddr, TEST.port, "tcp", logger=test_out)
    if ret != 0:
        TEST_LOG.print_err("* Verify Failed (vol {})".format(volumeId))
        sys.exit(1)
    else:
        TEST_LOG.print_info("* Verify Success (vol {})".format(volumeId))

# TODO (huijeong.kim) to update test cases with this verify
def verify_with_two_patterns(volumeId, offset, size, pattern1, pattern2):
    TEST_LOG.print_info("* Verify data: vol " + str(volumeId) + ", offset " + str(offset) + \
                ", size " + str(size) + ", pattern " + str(pattern1) + " or " + str(pattern2))
    test_out = open(TEST.output_log_path, "a")

    current_offset = offset
    while current_offset < offset+TEST_LIB.parse_size(size):
        verify_block(volumeId, current_offset, "4k", pattern1, pattern2, test_out)
        current_offset += 4096

def verify_block(volumeId, offset, size, pattern1, pattern2, logger=0):
    ret = run_fio("read", volumeId, size, offset, pattern1, TEST.traddr, TEST.port, "tcp", logger=logger)
    if ret != 0:
        ret = run_fio("read", volumeId, size, offset, pattern2, TEST.traddr, TEST.port, "tcp", logger=logger)
        if ret != 0:
            TEST_LOG.print_err("* Verify Failed (vol {})".format(volumeId))
            sys.exit(1)
        else:
            TEST_LOG.print_info("* Verify Success with pattern {} at offset {} (vol {})".format(pattern2, offset, volumeId))
            return 0
    else:
        TEST_LOG.print_info("* Verify Success with pattern {} at offset {} (vol {})".format(pattern1, offset, volumeId))
        return 0

def run_fio(workload, volumeId, io_size_bytes, io_offset, verify_pattern, ip, port, transport="tcp", runTime=0, logger=0):
    msg = "io={}, pattern={}".format(workload, verify_pattern)
    filename = "trtype=" + transport + " adrfam=IPv4" + " traddr=" + ip + " trsvcid=" + str(port) + " subnqn=nqn.2019-04.pos\\:subsystem" + str(volumeId) + " ns=1"
    command = "fio --thread=1 --group_reporting=1 --direct=1 " \
            + " --ioengine=" + TEST.ioengine + "" \
            + " --size=" + str(io_size_bytes) + "" \
            + " --bs=" + block_size + "" \
            + " --iodepth=" + str(q_depth) + "" \
            + " --readwrite=" + workload

    if verify_on == True:
        command += " --verify=md5 "
    else:
        command += " --verify=0 "

    command += " --offset=" + str(io_offset)
    command += " --verify=pattern --verify_pattern=" + verify_pattern
    command += extra_fio_options
    command += " --name=test --filename='" + filename + "'"

    if runTime != 0:
        command += " --runtime=" + str(runTime) + ""
        command += " --time_based=" + str(1) + ""
        msg += ", run_time={}".format(runTime)
    else:
        command += " --time_based=" + str(0) + ""
        msg += ", size={}".format(io_size_bytes)
    msg += ", block_size={}, qd={}, cpu mask={}".format(block_size, q_depth, core_mask)
    TEST_LOG.print_info("[FIO-vol{}] Started. {}".format(volumeId, msg), color="")
    TEST_LOG.print_debug(command)

    if logger != 0:
        ret = subprocess.call(command, shell=True, stdout=logger, stderr=logger)
    else:
        ret = subprocess.call(command, shell=True)

    if ret != 0:
        TEST_LOG.print_info("[FIO-vol{}] Terminated".format(volumeId))
    else:
        TEST_LOG.print_info("[FIO-vol{}] Success.".format(volumeId), color="")

    return ret