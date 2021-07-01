#!/usr/bin/env python3

import sys
import os

from threading import Thread

import TEST
import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

current_test = 0
current_offset = 0
arrayId = 0
volumes = [1, 2, 3, 4]
volSize = TEST.volSize

############################################################################
# Test Description
# Create 4 volumes with 1/4 size of array, precondition write to the whole volume,
# and then write data to each volume, simulate SPOR, and verify the pattern for 10 times
############################################################################


def precondition_write(volSize):
    TEST_LOG.print_notice("Pre-condition write started")
    thread_list = []

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        th = Thread(target=TEST_FIO.write, args=(arrayId, volId, TEST_LIB.get_latest_pattern(arrayId, volId), volSize, 0))
        thread_list.append(th)
        th.start()

    for th in thread_list:
        th.join()

    TEST_LOG.print_notice("Pre-condition write completed")


def test(size):
    global current_test
    global current_offset
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, current_offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        TEST_FIO.verify(arrayId, volId, current_offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

    current_offset += TEST_LIB.parse_size(size)


def get_max_vol_size():
    capacity_byte = TEST_LIB.get_array_capacity(arrayId)
    volSize = capacity_byte // len(volumes)

    megabytes = TEST_LIB.parse_size('1M')
    volSize = (volSize // megabytes) * megabytes

    return volSize


def execute():
    volSize = get_max_vol_size()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId, volSize)

    precondition_write(TEST_LIB.get_size(volSize))

    for _ in range(5):
        test('1M')


if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup()

    execute()

    TEST_LIB.tear_down(test_name=filename)
