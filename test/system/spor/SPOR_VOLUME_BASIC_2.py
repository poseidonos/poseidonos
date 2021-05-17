#!/usr/bin/env python3

import sys
import os

import TEST_FIO
import TEST_LIB
import TEST_LOG
import TEST_SETUP_POS

current_test = 0
arrayId = 0
volumes = [1, 2]
num_volumes_to_create = 2
offset = 4096
size = '128k'

############################################################################
# Test Description
# write data to several volumes,
# create new volume, and write different pattern to every volumes,
# and simulate SPOR and verify each volume with latest pattern
############################################################################


def execute():
    global current_test
    current_test = current_test + 1
    TEST_LOG.print_notice("[{} - Test {} Started]".format(filename, current_test))

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    for volId in range(len(volumes) + 1, max(volumes) + num_volumes_to_create + 1):
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.create_volume(arrayId, volId)
        volumes.append(volId)

    for volId in volumes:
        TEST_LIB.create_new_pattern(arrayId, volId)
        TEST_FIO.write(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_SETUP_POS.trigger_spor()
    TEST_SETUP_POS.dirty_bringup()

    for volId in volumes:
        TEST_SETUP_POS.create_subsystem(arrayId, volId)
        TEST_SETUP_POS.mount_volume(arrayId, volId)

    for volId in volumes:
        TEST_FIO.verify(arrayId, volId, offset, size, TEST_LIB.get_latest_pattern(arrayId, volId))

    TEST_LOG.print_notice("[Test {} Completed]".format(current_test))


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
