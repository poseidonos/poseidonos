#!/usr/bin/env python3

import sys
import os

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

log_buffer_size_mb = 50
current_test = 0
volId = 1

############################################################################
## Test Description
##  write pattern to the volume until the checkpoint is triggered, simulate SPOR, and verify the pattern
############################################################################
def test(offset, size):
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    index = 0
    while TEST_LIB.get_checkpoint_status() == "INIT":
        TEST_LIB.create_new_pattern(volId)
        TEST_FIO.write(volId, offset + (size * index), size, TEST_LIB.get_latest_pattern(volId))
        index += 1

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    TEST_SETUP_POS.create_subsystem(volId)
    TEST_SETUP_POS.mount_volume(volId)

    for index in range(len(TEST_LIB.patterns[volId]) // 2):
        TEST_FIO.verify(volId, offset + (size * index), size, TEST_LIB.get_pattern(volId, index))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))

def execute():
    size = TEST_LIB.get_log_buffer_size() // 4096 * 4096
    test(offset=0, size=int(size))

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)
    TEST_LIB.set_log_buffer_size(log_buffer_size_mb)

    TEST_SETUP_POS.clean_bringup()
    TEST_SETUP_POS.create_subsystem(volId)
    TEST_SETUP_POS.create_volume(volId)

    execute()

    TEST_LIB.tear_down(test_name=filename)
    TEST_LIB.set_log_buffer_size(0)
