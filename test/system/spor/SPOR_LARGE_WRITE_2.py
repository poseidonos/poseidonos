#!/usr/bin/env python3

import sys
import os
from threading import Thread

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

arrayId = 0
volumes = list(range(1, 11))
current_test = 0
num_write = 10

############################################################################
# Test Description
# write large data to several volumes, simulate SPOR, verify the pattern per each volume
############################################################################


def test(size):
    global current_test, num_write
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    write_size = TEST_LIB.parse_size(size)
    for volId in volumes:
        for index in range(num_write):
            TEST_LIB.create_new_pattern(arrayId, volId)
            TEST_FIO.write(arrayId, volId, write_size * index, size, TEST_LIB.get_pattern(arrayId, volId, index))

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        for index in range(num_write):
            TEST_FIO.verify(arrayId, volId, write_size * index, size, TEST_LIB.get_pattern(arrayId, volId, index))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


def execute():
    test(size='50M')


if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup()
    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)
