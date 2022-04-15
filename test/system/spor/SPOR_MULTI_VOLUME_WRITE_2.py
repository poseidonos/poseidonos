#!/usr/bin/env python3

import sys
import os
import time
import subprocess

from itertools import product
from threading import Thread

import TEST
import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

arrayId = 0
volumes = [1, 2]
current_test = 0
run_time = 0

############################################################################
# Test Description
# write pattern to several volumes for (run_time) secs,
# simulate SPOR before all writes are completed (WRITE FAIL EXPECTED),
# and verify the pattern of first (size) bytes per each volume
############################################################################


def test(offset, size):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    thread_list = []
    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        th = Thread(target=TEST_FIO.write, args=(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId), run_time))
        thread_list.append(th)

    for th in thread_list:
        th.start()

    TEST_LOG.print_err("* Write Fail Expected")
    time.sleep(run_time / 2)
    TEST_SETUP_POS.trigger_spor()
    for th in thread_list:
        th.join()

    TEST_SETUP_POS.dirty_bringup()
    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        TEST_FIO.verify(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


def execute():
    offsets = [0, 4096]
    sizes = ['256k', '512k']

    if TEST.quick_mode == False:
        for (_offset, _size) in product(offsets, sizes):
            test(offset=_offset, size=_size)
    else:
        test(offset=offsets[-1], size=sizes[-1])


if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup()
    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId)

    run_time = TEST.run_time

    execute()

    TEST_LIB.tear_down(test_name=filename)
