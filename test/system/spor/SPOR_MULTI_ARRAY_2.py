#!/usr/bin/env python3

import sys
import os

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

arrays = [0, 1]
volumes = [1, 2, 3]
current_test = 0
offset = 4096
size = '128k'

############################################################################
# Test Description
# write pattern A to several array with multiple volumes,
# and simulate SPOR and verify each volume with latest pattern
############################################################################


def execute():
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[Test {} Started]".format(current_test))

    for arrayId in arrays:
        for volumeId in volumes:
            TEST_LIB.create_new_pattern(arrayId, volumeId)
            TEST_FIO.write(arrayId, volumeId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volumeId))

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup(arrays)

    for arrayId in arrays:
        for volumeId in volumes:
            TEST_SETUP_POS.create_subsystem(arrayId, volumeId)
            TEST_SETUP_POS.mount_volume(arrayId, volumeId)

    for arrayId in arrays:
        for volumeId in volumes:
            TEST_FIO.verify(arrayId, volumeId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volumeId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


if __name__ == "__main__":
    filename = sys.argv[0].split("/")[-1].split(".")[0]
    TEST_LIB.set_up(argv=sys.argv, test_name=filename)

    TEST_SETUP_POS.clean_bringup(arrays)
    for arrayId in arrays:
        for volumeId in volumes:
            TEST_SETUP_POS.create_subsystem(arrayId, volumeId)
            TEST_SETUP_POS.create_volume(arrayId, volumeId)

    execute()

    TEST_LIB.tear_down(filename, len(arrays))
