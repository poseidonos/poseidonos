#!/usr/bin/env python3

import sys
import os

import TEST_LIB
import TEST_SETUP_POS
import TEST_FIO

log_buffer_size_mb = 5
arrayId = 0
volId = 1

if __name__ == "__main__":
    global filename
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)
    TEST_LIB.set_log_buffer_size(log_buffer_size_mb)

    TEST_SETUP_POS.clean_bringup()
    TEST_SETUP_POS.create_subsystem(arrayId, volId)
    TEST_SETUP_POS.create_volume(arrayId, volId)

    TEST_SETUP_POS.unmount_volume(arrayId, volId)
    TEST_SETUP_POS.unmount_array(arrayId)
    TEST_SETUP_POS.mount_array(arrayId)
    TEST_SETUP_POS.mount_volume(arrayId, volId)

    size = TEST_LIB.get_log_buffer_size() // 4096 * 4096
    while TEST_LIB.get_checkpoint_status() != "COMPLETED":
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, 0, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LIB.tear_down(test_name=filename)
